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
	~FieldedMesh(void);

	bool									load(const char* path);
	bool									isLoaded();

	const Point&							boundingBoxMin();
	const Point&							boundingBoxMax();

	Vec3f									faceVectorField(const FaceHandle& face, float time) const;
				
	const vector<uint>&						getIndices() const;
	Triangle								getFacePoints(OpenMesh::ArrayKernel::FaceHandle faceHandle);
protected:
	bool									isLoaded_;
	Point									bbMax;
	Point									bbMin;
	vector<unsigned int>					faceIndices;
	FaceFieldPropT							vectorFieldFaceProperty;

private:
	void									surroundBoundingBox();
	void									updateFaceIndices();
	void									assignVectorField();
};

