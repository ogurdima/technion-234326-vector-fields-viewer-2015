#include "FieldedMesh.h"
#include <iostream>
#include <fstream>
#include <string>


FieldedMesh::FieldedMesh(void) : 
	isLoaded_(false), 
	bbMin(.0f), 
	bbMax(.0f),
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
	// load mesh
	
	if (OpenMesh::IO::read_mesh(*this, path))
	{
		isLoaded_ = true;
		normalizeMesh();

		// compute face & vertex normals
		update_normals();

		// update face indices for faster rendering
		updateFaceIndices();

		// compute and assign vector field
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
		readFieldFile(path, fields, times);
	}
	else
	{
		vector<Vec3f> constField = readConstFieldFile(path);
		fields.resize(constField.size());
		for(int i = 0; i < constField.size(); i++)
		{
			fields[i].push_back(constField[i]);
			fields[i].push_back(constField[i]);
		}
		times.push_back(Time(0));
		times.push_back(Time(1));
	}
	return assignFieldToVertices(fields, times);
	//return assignFieldToFaces(fields, times);
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

void FieldedMesh::assignRotatingVectorField(const Vec3f& rotationAxis)
{
	vector<Vec3f> constField;
	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit) 
	{
		constField.push_back(normal(cfit.handle()) % rotationAxis);
	}

	vector<vector<Vec3f>> fields;
	vector<Time> times;

	fields.resize(constField.size());
	for(int i = 0; i < constField.size(); i++)
	{
		fields[i].push_back(constField[i]);
		fields[i].push_back(constField[i]);
	}
	times.push_back(Time(0));
	times.push_back(Time(1));

	assignFieldToVertices(fields, times);
	//assignFieldToFaces(fields, times);
}

//void FieldedMesh::assignRandVectorField()
//{
//	srand((uint)time(0));
//	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit) 
//	{
//		float x = VectorFieldsUtils::fRand(0,1);
//		float y = VectorFieldsUtils::fRand(0,(1-x));
//		float z = 1 - x - y;
//		Point inBarycentric = Vec3f(x,y,z);
//		// For now we make this vector field constant in time
//		Triangle triangle = getFacePoints(cfit);
//		
//		Point inStd = VectorFieldsUtils::barycentricToStd(inBarycentric, triangle);
//		Point center = VectorFieldsUtils::barycentricToStd(Vec3f(float(1/3.)), triangle);
//
//		vector<VectorFieldTimeVal> faceVectorField;
//		Vec3f field = inStd - center;
//		field.normalize();
//		faceVectorField.push_back(VectorFieldTimeVal(field,0));
//		faceVectorField.push_back(VectorFieldTimeVal(field,1));
//
//		property(vectorFieldFaceProperty, cfit.handle()) = faceVectorField;
//
//	}
//}

//bool FieldedMesh::assignRandVectorFieldPerVertex()
//{
//	srand((uint)time(0));
//	for(VertexIter vit(vertices_begin()), vend(vertices_end()); vit != vend; ++vit)
//	{
//		const Normal& vn = normal(vit);
//		vector<Normal> faceNormals;
//		for(VertexFaceIter vfit(vf_begin(vit)), vfend(vf_end(vit)); vfit != vfend; ++vfit)
//		{
//			faceNormals.push_back(normal(vfit));
//		}
//		int size = faceNormals.size();
//		property(vertexFieldProperty, vit) = size == 0 ? Vec3f(0.) :  vn % faceNormals[ rand() % size];
//	}
//	return true;
//}

bool FieldedMesh::assignRotatingVectorFieldPerVertex(const Vec3f& axis)
{
	for(VertexIter vit(vertices_begin()), vend(vertices_end()); vit != vend; ++vit)
	{
		vector<VectorFieldTimeVal>& field = property(vertexFieldProperty, vit);
		Vec3f first(point(vit) % axis);
		field.push_back(VectorFieldTimeVal(first, Time(0)));
		field.push_back(VectorFieldTimeVal(normal(vit) % first, Time(1)));
	}
	_minTime = Time(0);
	_maxTime = Time(1);
	return true;
}

Triangle FieldedMesh::getFacePoints(const OpenMesh::ArrayKernel::FaceHandle& faceHandle)
{
	ConstFaceVertexIter cvit(cfv_iter(faceHandle));
	return Triangle(point(cvit), point(++cvit), point(++cvit));
}

Time FieldedMesh::maxTime()
{
	return _maxTime;
}

Time FieldedMesh::minTime()
{
	return _minTime;
}

const Point& FieldedMesh::boundingBoxMin()
{
	return bbMin;
}

const Point& FieldedMesh::boundingBoxMax()
{
	return bbMax;
}

void FieldedMesh::normalizeMesh()
{
	// set center and radius
	ConstVertexIter  v_it(vertices_begin()), v_end(vertices_end());

	bbMin = bbMax = point(++v_it);
	for (; v_it!=v_end; ++v_it)
	{
		bbMin.minimize(point(v_it));
		bbMax.maximize(point(v_it));
	}

	Point translate = (bbMin + bbMax) / 2;

	bbMax -= bbMin;

	scaleFactor = std::max(bbMax[0], std::max(bbMax[1], bbMax[2])) / 2;

	v_it = vertices_begin();
	v_end = vertices_end();
	for (; v_it!=v_end; ++v_it)
	{
		set_point(v_it, (point(v_it) + translate) / scaleFactor);
	}

	v_it = vertices_begin();
	v_end = vertices_end();
	bbMin = bbMax = point(++v_it);
	for (; v_it!=v_end; ++v_it)
	{
		bbMin.minimize(point(v_it));
		bbMax.maximize(point(v_it));
	}
}

//const vector<VectorFieldTimeVal>& FieldedMesh::getVectorField(const FaceHandle& handle) const
//{
//	return property(vectorFieldFaceProperty, handle);
//}

const vector<VectorFieldTimeVal>& FieldedMesh::vertexField(const Mesh::VertexHandle& vertexHandle) const
{
	return property(vertexFieldProperty, vertexHandle);
}

//Vec3f FieldedMesh::faceVectorField(const Mesh::FaceHandle& faceHandle, const Time& time) const
//{
//	const vector<VectorFieldTimeVal>& fieldSamples = property(vectorFieldFaceProperty, faceHandle);
//	if (fieldSamples.size() == 0) 
//	{
//		return Vec3f(0,0,0);
//	}
//
//	// assuming samples are listed in increasing time order
//	Time	prevTime = fieldSamples[0].time;
//	Time	nextTime = fieldSamples[0].time;
//	Vec3f	prevField = fieldSamples[0].field;
//	Vec3f	nextField = fieldSamples[0].field;
//
//	for (uint i = 0; i < fieldSamples.size(); i++) {
//		nextTime = fieldSamples[i].time;
//		nextField = fieldSamples[i].field;
//		if (nextTime >= time) {
//			break;
//		}
//		prevTime = fieldSamples[i].time;
//		prevField = fieldSamples[i].field;
//	}
//	Vec3f result;
//	if (abs(nextTime - prevTime) < NUMERICAL_ERROR_THRESH) 
//	{
//		result = (nextField + prevField) / 2.0;
//	}
//	else 
//	{
//		result = (prevField * (time - prevTime) + nextField * (nextTime - time)) / (nextTime - prevTime);
//	}
//	if (!_finite(result[0]))
//	{
//		bool debug = true;
//	}
//	return result;
//}

bool FieldedMesh::isLoaded()
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
	
	float t;
	std::string timePath;
	std::ifstream file = std::ifstream(path);
	while (file >> t >> timePath)
	{
		vector<Vec3f> constField = readConstFieldFile(timePath.c_str());
		assert(constField.size() == n_faces());
		for (int i = 0; i < constField.size(); i++)
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

bool FieldedMesh::assignFieldToFaces(const vector<vector<Vec3f>>& fieldPerFace, const vector<Time>& times)
{
	if (!isLoaded())
	{
		std::cerr << "Failed to assign vector field: mesh not loaded" << std::endl;
		return false;
	}
	int timeSize = times.size();
	int i = 0;
	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit, ++i) 
	{
		vector<VectorFieldTimeVal> faceVectorField;
		for(int t = 0; t < timeSize; ++t)
		{
			faceVectorField.push_back(VectorFieldTimeVal(fieldPerFace[i][t], times[t]));
		}
		//property(vectorFieldFaceProperty, cfit) = faceVectorField;
	}
	_minTime = times[0];
	_maxTime = times[timeSize - 1];
	return true;
}

bool FieldedMesh::assignFieldToVertices(const vector<vector<Vec3f>>& fieldPerFace, const vector<Time>& times)
{
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
	return true;
}

