#include "FieldedMesh.h"
#include <iostream>
#include <fstream>
#include <string>

FieldedMesh::FieldedMesh(void) : 
	isLoaded_(false), 
	bbMin(.0f), 
	bbMax(.0f),
	scaleFactor(1)
{
	request_face_normals();
	request_vertex_normals();
	request_vertex_colors();
	add_property(vectorFieldFaceProperty);
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
		assignRotatingVectorField();
		//assignRandVectorField();
	}
	return isLoaded_;
}

bool FieldedMesh::assignVectorField(const char* path)
{
	const std::vector<Vec3f>& field = readFieldFile(path);	
	return assignFieldToFaces(field);
}

void FieldedMesh::updateFaceIndices()
{
	Mesh::ConstFaceIter			f_it(faces_sbegin()), f_end(faces_end());
	Mesh::ConstFaceVertexIter	fv_it;

	faceIndices.clear();
	faceIndices.reserve(n_faces()*3);

	for (; f_it!=f_end; ++f_it)
	{
		fv_it = fv_iter(f_it);
		faceIndices.push_back( fv_it.handle().idx() );
		faceIndices.push_back((++fv_it).handle().idx());
		faceIndices.push_back((++fv_it).handle().idx());
	}
}

// Add random normalized Vec3f as vfield_fprop property for each face
// Color all certices appropriately
void FieldedMesh::assignRotatingVectorField(const Vec3f& rotationAxis)
{
	vector<Vec3f> fieldPerFace;
	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit) 
	{
		fieldPerFace.push_back(normal(cfit.handle()) % rotationAxis);
	}
	assignFieldToFaces(fieldPerFace);
}

void FieldedMesh::assignRandVectorField()
{
	srand((uint)time(0));
	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit) 
	{
		float x = VectorFieldsUtils::fRand(0,1);
		float y = VectorFieldsUtils::fRand(0,(1-x));
		float z = 1 - x - y;
		Point inBarycentric = Vec3f(x,y,z);
		// For now we make this vector field constant in time
		Triangle triangle = getFacePoints(cfit);
		
		Point inStd = VectorFieldsUtils::barycentricToStd(inBarycentric, triangle);
		Point center = VectorFieldsUtils::barycentricToStd(Vec3f(float(1/3.)), triangle);

		vector<VectorFieldTimeVal> faceVectorField;
		Vec3f field = inStd - center;
		field.normalize();
		faceVectorField.push_back(VectorFieldTimeVal(field,0));
		faceVectorField.push_back(VectorFieldTimeVal(field,1));

		property(vectorFieldFaceProperty, cfit.handle()) = faceVectorField;

	}
}

Triangle FieldedMesh::getFacePoints(const OpenMesh::ArrayKernel::FaceHandle& faceHandle)
{
	ConstFaceVertexIter cvit(cfv_iter(faceHandle));
	return Triangle(point(cvit), point(++cvit), point(++cvit));
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

const vector<VectorFieldTimeVal> FieldedMesh::getVectorField(const FaceHandle& handle) const
{
	return property(vectorFieldFaceProperty, handle);
}

Vec3f FieldedMesh::faceVectorField(const Mesh::FaceHandle& faceHandle, const Time& time) const
{
	const vector<VectorFieldTimeVal>& fieldSamples = property(vectorFieldFaceProperty, faceHandle);
	if (fieldSamples.size() == 0) 
	{
		return Vec3f(0,0,0);
	}

	// assuming samples are listed in increasing time order
	Time	prevTime = fieldSamples[0].time;
	Time	nextTime = fieldSamples[0].time;
	Vec3f	prevField = fieldSamples[0].field;
	Vec3f	nextField = fieldSamples[0].field;

	for (uint i = 0; i < fieldSamples.size(); i++) {
		nextTime = fieldSamples[i].time;
		nextField = fieldSamples[i].field;
		if (nextTime >= time) {
			break;
		}
		prevTime = fieldSamples[i].time;
		prevField = fieldSamples[i].field;
	}
	Vec3f result;
	if (abs(nextTime - prevTime) < NUMERICAL_ERROR_THRESH) 
	{
		result = (nextField + prevField) / 2.0;
	}
	else 
	{
		result = (prevField * (time - prevTime) + nextField * (nextTime - time)) / (nextTime - prevTime);
	}
	if (!_finite(result[0]))
	{
		bool debug = true;
	}
	return result;
}

bool FieldedMesh::isLoaded()
{
	return isLoaded_;
}

const vector<unsigned int>& FieldedMesh::getIndices() const
{
	return faceIndices;
}

std::vector<Vec3f> FieldedMesh::readFieldFile(const char* path)
{
	std::ifstream file = std::ifstream(path);
	std::vector<Vec3f> fieldPerFace;
	float x,y,z;
	while (file >> x >> y >> z)
	{
		fieldPerFace.push_back(Vec3f(x,y,z));
	}
	std::cout << "Read " << fieldPerFace.size() << " vectors" << std::endl;
	return fieldPerFace;
}

bool FieldedMesh::assignFieldToFaces(const std::vector<Vec3f>& fieldPerFace)
{
	if (!isLoaded())
	{
		std::cerr << "Failed to assign vector field: mesh not loaded" << std::endl;
		return false;
	}
	if (fieldPerFace.size() < n_faces())
	{
		std::cerr << "Failed to assign vector field: fieldPerFace.size() < n_faces()" << std::endl;
		return false;
	}
	int i = 0;
	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit, ++i) 
	{
		Vec3f fixed = VectorFieldsUtils::projectVectorOntoTriangle(fieldPerFace[i], normal(cfit)) / scaleFactor;
		
		vector<VectorFieldTimeVal> faceVectorField;
		faceVectorField.push_back(VectorFieldTimeVal(fixed, 0));
		faceVectorField.push_back(VectorFieldTimeVal(fixed, 1));
		property(vectorFieldFaceProperty, cfit) = faceVectorField;
	}
	return true;
}
	