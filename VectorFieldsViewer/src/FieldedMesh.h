#pragma once

#include "VectorFieldsUtils.h"

typedef OpenMesh::FPropHandleT<vector<VectorFieldTimeVal>> FaceFieldPropT;
typedef OpenMesh::VPropHandleT<vector<VectorFieldTimeVal>> VertexFieldHandleT;

class FieldedMesh : public Mesh
{

public:
	/*Default Constructor*/					FieldedMesh(void);
	bool									load(const char* path);
	bool									isLoaded();
	bool									assignVectorField(const char* path, bool isConst);
	const Point&							boundingBoxMin();
	const Point&							boundingBoxMax();
	const vector<uint>&						getIndices() const;
	Triangle								getFacePoints(const OpenMesh::ArrayKernel::FaceHandle& faceHandle);
	Time									minTime();
	Time									maxTime();
	const vector<VectorFieldTimeVal>&		vertexField(const VertexHandle& vertex) const;

protected:
	bool									isLoaded_;
	Point									bbMax;
	Point									bbMin;
	vector<unsigned int>					faceIndices;
	VertexFieldHandleT						vertexFieldProperty;
	float									scaleFactor;
	Time									_minTime;
	Time									_maxTime;
private:
	void									normalizeMesh();
	void									updateFaceIndices();
	void									assignRandVectorField();
	void									assignRotatingVectorField(const Vec3f& rotationAxis = Vec3f(0,0,1));
	vector<Vec3f>							readConstFieldFile(const char* path);
	void									readFieldFile(const char* path, vector<vector<Vec3f>>& fieldPerFace, vector<Time>& times);
	bool									assignFieldToFaces(const vector<vector<Vec3f>>& fieldPerFace, const vector<Time>& times);
	bool									assignFieldToVertices(const vector<vector<Vec3f>>& fieldPerFace, const vector<Time>& times);
	bool									assignRotatingVectorFieldPerVertex(const Vec3f& rotationAxis = Vec3f(0,0,1));

};

