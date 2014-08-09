#pragma once

#include "VectorFieldsUtils.h"

typedef OpenMesh::FPropHandleT<vector<VectiorFieldTimeVal>> FaceFieldPropT;

class FieldedMesh : public Mesh
{

public:
	FieldedMesh(void);
	~FieldedMesh(void);

	bool load(const char* path);

	bool isLoaded();

	const Point& boundingBoxMin();
	const Point& boundingBoxMax();

	Vec3f faceVectorField(const Mesh::FaceHandle& face, double time) const;

	const vector<unsigned int>& getIndices() const;

protected:
	bool									isLoaded_;
	Point									bbMax;
	Point									bbMin;
	vector<unsigned int>					faceIndices;
	FaceFieldPropT							vectorFieldFaceProperty;

private:
	void surroundBoundingBox();
	void updateFaceIndices();
	void assignVectorField();
};

