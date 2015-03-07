#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include "VectorFieldsUtils.h"

typedef OpenMesh::FPropHandleT<vector<VectorFieldTimeVal>> FaceFieldPropT;
typedef OpenMesh::VPropHandleT<vector<VectorFieldTimeVal>> VertexFieldHandleT;

class FieldedMesh : public Mesh
{

public:
	FieldedMesh(void);

	bool									load(const char* path);
	bool									isLoaded() const;
	bool									hasField() const;
	bool									assignVectorField(const char* path, bool isConst);

	const vector<uint>&						getIndices() const;
	Triangle								getFacePoints(const OpenMesh::ArrayKernel::FaceHandle& faceHandle) const;

	Time									minTime();
	Time									maxTime();

	const vector<VectorFieldTimeVal>&		vertexField(const VertexHandle& vertex) const;
	void									setMeshColor(const Vec4f& color);
	const Vec4f*							getVertexColors() const;

protected:
	bool									isLoaded_;
	bool									isFieldAssigned;
	vector<unsigned int>					faceIndices;
	Vec4f									meshColor;
	vector<Vec4f>							vertexColors;

	VertexFieldHandleT						vertexFieldProperty;
	float									scaleFactor;

	Time									_minTime;
	Time									_maxTime;
private:
	void									normalizeMesh();
	void									updateFaceIndices();

	vector<Vec3f>							readConstFieldFile(const char* path);
	void									readFieldFile(const char* path, vector<vector<Vec3f>>& fieldPerFace, vector<Time>& times);
	bool									assignFieldToVertices(const vector<vector<Vec3f>>& fieldPerFace, const vector<Time>& times);
	bool									assignRotatingVectorFieldPerVertex(const Vec3f& rotationAxis = Vec3f(0,0,1));

};

