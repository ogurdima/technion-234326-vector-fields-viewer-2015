#pragma once

#include "VectorFieldsUtils.h"

typedef OpenMesh::FPropHandleT<vector<VectorFieldTimeVal>> FaceFieldPropT;

//typedef OpenMesh::VectorT<Vec3f,3> TriVerteces;
//
//typedef struct 
//{
//	OpenMesh::ArrayItems::Halfedge		halfEdge;
//	Point								from;
//	Point								to;
//
//	bool								isWindward(Vec3f field);
//} TriHalfEdge;
//
//typedef OpenMesh::VectorT<TriHalfEdge,3> TriHalfEdges;
//
//class FaceWrapper
//{
//public:
//	TriHalfEdges halfEdges;
//};


class FieldedMesh : public Mesh
{

public:
	FieldedMesh(void);

	bool									load(const char* path);
	bool									isLoaded();
	bool									assignVectorField(const char* path);

	const Point&							boundingBoxMin();
	const Point&							boundingBoxMax();

	Vec3f									faceVectorField(const FaceHandle& face, const Time& time) const;
				
	const vector<uint>&						getIndices() const;
	Triangle								getFacePoints(const OpenMesh::ArrayKernel::FaceHandle& faceHandle);
protected:
	bool									isLoaded_;
	Point									bbMax;
	Point									bbMin;
	vector<unsigned int>					faceIndices;
	FaceFieldPropT							vectorFieldFaceProperty;

private:
	void									normalizeMesh();
	void									updateFaceIndices();
	void									assignRandVectorField();
	void									assignRotatingVectorField(const Vec3f& rotationAxis = Vec3f(0,0,1));
	std::vector<Vec3f>						readFieldFile(const char* path);
	bool									assignFieldToFaces(std::vector<Vec3f> fieldPerFace);
};

