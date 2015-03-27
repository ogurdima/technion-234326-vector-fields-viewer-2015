#include "FieldedMesh.h"
#include <iostream>
#include <fstream>
#include <string>

FieldedMesh::FieldedMesh(void) : 
	isLoaded_(false), 
	isFieldAssigned(false),
	scaleFactor(1),
	_minTime(0),
	_maxTime(1)
{
	request_face_normals();
	request_vertex_normals();
	request_vertex_colors();
	//add_property(vectorFieldFaceProperty);
	add_property(vertexFieldProperty);
}

bool FieldedMesh::load(const char* path)
{
	isLoaded_ = false;
	isFieldAssigned = false;
	if (OpenMesh::IO::read_mesh(*this, path))
	{
		normalizeMesh();
		update_normals();
		updateFaceIndices();
		isLoaded_ = true;
	}
	if(isLoaded_)
	{
		setMeshColor(meshColor);
		assignRotatingVectorFieldPerVertex();
	}
	return isLoaded_;
}

bool FieldedMesh::assignVectorField(const char* path, bool isConst)
{
	vector<vector<Vec3f>> fields;
	vector<Time> times;
	
	if(!isConst)
	{
		try {
			readFieldFile(path, fields, times);
		}
		catch (std::exception e)
		{
			std::cerr << "Exception caught in FieldedMesh::assignVectorField: " << e.what() << std::endl;
			return false;
		}
	}
	else
	{
		vector<Vec3f> constField;
		try {
			constField = readConstFieldFile(path);
		}
		catch (std::exception e)
		{
			std::cerr << "Exception caught in FieldedMesh::assignVectorField: " << e.what() << std::endl;
			return false;
		}
		fields.resize(constField.size());
		for(unsigned int i = 0; i < constField.size(); i++)
		{
			fields[i].push_back(constField[i]);
			fields[i].push_back(constField[i]);
		}
		times.push_back(Time(0));
		times.push_back(Time(1));
	}
	return assignFieldToVertices(fields, times);
}

void FieldedMesh::updateFaceIndices()
{
	faceIndices.clear();
	faceIndices.reserve(n_faces()*3);

	Mesh::ConstFaceIter			f_it(faces_sbegin()), f_end(faces_end());
	for (; f_it!=f_end; ++f_it)
	{
		Mesh::ConstFaceVertexIter fv_it = fv_iter(f_it);
		faceIndices.push_back( fv_it.handle().idx() );
		faceIndices.push_back((++fv_it).handle().idx());
		faceIndices.push_back((++fv_it).handle().idx());
	}
}

bool FieldedMesh::assignRotatingVectorFieldPerVertex(const Vec3f& axis)
{
	isFieldAssigned = false;
	for(VertexIter vit(vertices_begin()), vend(vertices_end()); vit != vend; ++vit)
	{
		vector<VectorFieldTimeVal>& field = property(vertexFieldProperty, vit);
		Vec3f first(point(vit) % axis);
		field.push_back(VectorFieldTimeVal(first, Time(0)));
		field.push_back(VectorFieldTimeVal(normal(vit) % first, Time(1)));
	}
	_minTime = Time(0);
	_maxTime = Time(1);
	isFieldAssigned = true;
	return true;
}

Time FieldedMesh::maxTime()
{
	return _maxTime;
}

Time FieldedMesh::minTime()
{
	return _minTime;
}

void FieldedMesh::normalizeMesh()
{
	ConstVertexIter  v_it(vertices_begin()), v_end(vertices_end());
	Point bbMin,bbMax;
	bbMin = bbMax = point(v_it);
	for (++v_it; v_it!=v_end; ++v_it)
	{
		bbMin.minimize(point(v_it));
		bbMax.maximize(point(v_it));
	}

	Point translation = (bbMin + bbMax) / 2;
	bbMax -= bbMin;
	scaleFactor = std::max(bbMax[0], std::max(bbMax[1], bbMax[2])) / 2;

	v_it = vertices_begin();
	v_end = vertices_end();
	for (; v_it!=v_end; ++v_it)
	{
		set_point(v_it, (point(v_it) + translation) / scaleFactor);
	}
}

const vector<VectorFieldTimeVal>& FieldedMesh::vertexField(const Mesh::VertexHandle& vertexHandle) const
{
	return property(vertexFieldProperty, vertexHandle);
}

bool FieldedMesh::isLoaded() const
{
	return isLoaded_;
}

const vector<unsigned int>& FieldedMesh::getIndices() const
{
	return faceIndices;
}

void FieldedMesh::readFieldFile(const char* path, vector<vector<Vec3f>>& fieldPerFace, vector<Time>& times)
{
	times.clear();
	fieldPerFace.clear();
	fieldPerFace.resize(n_faces());
	
	
	std::string pathString(path);
	int idx = pathString.find_last_of('\\');
	std::string start = std::string(path).substr(0, idx + 1);
	
	float t;
	std::string timePath;
	std::ifstream file = std::ifstream(path);
	while (file >> t >> timePath)
	{
		std::string fullPath = start + timePath;
		vector<Vec3f> constField = readConstFieldFile(fullPath.c_str());
		assert(constField.size() == n_faces());
		for (unsigned int i = 0; i < constField.size(); i++)
		{
			fieldPerFace[i].push_back(constField[i]);
		}
		times.push_back(t);
		assert(times.size() < 2 || times[times.size() - 2] < times[times.size() - 1]);
	}
	std::cout << "Read " << times.size() << " snapshots" << std::endl;
}

vector<Vec3f> FieldedMesh::readConstFieldFile(const char* path)
{
	vector<Vec3f> fieldPerFace;
	fieldPerFace.reserve(n_faces());
	
	float x,y,z;
	std::ifstream file = std::ifstream(path);
	while (file >> x >> y >> z)
	{
		fieldPerFace.push_back(Vec3f(x,y,z) / scaleFactor);
	}
	std::cout << "Read " << fieldPerFace.size() << " vectors" << std::endl;
	return fieldPerFace;
}

bool FieldedMesh::assignFieldToVertices(const vector<vector<Vec3f>>& fieldPerFace, const vector<Time>& times)
{
	isFieldAssigned = false;
	if (!isLoaded())
	{
		std::cerr << "Failed to assign vector field: mesh not loaded" << std::endl;
		return false;
	}
	if (fieldPerFace.size() != n_faces())
	{
		std::cerr<< "Failed to assign vector field: fieldPerFace.size() != n_faces()" << std::endl;
		return false;
	}
	std::cout << "Assigning vector field to vertices..." << std::endl;
	int timeSize = times.size();
	_minTime = times[0];
	_maxTime = times[timeSize - 1];

	FaceFieldPropT tmpFaceFieldProp;
	add_property(tmpFaceFieldProp);
	int i = 0;
	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit, ++i) 
	{
		vector<VectorFieldTimeVal> faceVectorField;
		for(int t = 0; t < timeSize; ++t)
		{
			faceVectorField.push_back(VectorFieldTimeVal(fieldPerFace[i][t], times[t]));
		}
		property(tmpFaceFieldProp, cfit) = faceVectorField;
	}
	for(VertexIter vit(vertices_begin()), vitEnd(vertices_end()); vit != vitEnd; ++vit)
	{
		vector<VectorFieldTimeVal> vertexField;
		for (int timeIdx = 0; timeIdx < timeSize; timeIdx++) 
		{
			vector<Vec3f> facesSnapshot;
			for(VertexFaceIter vfit(vf_begin(vit.handle())); vfit != vf_end(vit.handle()); vfit++)
			{
				vector<VectorFieldTimeVal> faceTime = property(tmpFaceFieldProp, vfit.handle());
				facesSnapshot.push_back(faceTime[timeIdx].field);
			}
			Vec3f vertexSnapshot = VectorFieldsUtils::average(facesSnapshot);
			vertexField.push_back(VectorFieldTimeVal(vertexSnapshot, times[timeIdx]));
		}
		property(vertexFieldProperty, vit.handle()) = vertexField;
	}
	remove_property(tmpFaceFieldProp);
	std::cout << "Finished assigning vector field to vertices" << std::endl;
	isFieldAssigned = true;
	return true;
}

bool FieldedMesh::hasField() const
{
	return isFieldAssigned;
}

void FieldedMesh::setMeshColor(const Vec4f& color)
{
	meshColor = color;
	uint size = n_vertices();
	if(vertexColors.size() < size) 
	{	
		vertexColors.resize(size);
	}
	for(uint i = 0; i < size; ++i)
	{
		vertexColors[i] = color;
	}
}

const Vec4f* FieldedMesh::getVertexColors() const
{
	return &vertexColors[0];
}