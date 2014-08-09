#pragma once

#include "VectorFieldsUtils.h"

class FieldedMesh : public Mesh
{

public:
	FieldedMesh(void);
	~FieldedMesh(void);

	bool load(const char* path);

	bool isLoaded();

	const Point& boundingBoxMin();
	const Point& boundingBoxMax();

	inline const Vec3f& faceVectorField(const Mesh::FaceHandle& face) const;

	const vector<unsigned int>& getIndices() const;

protected:
	bool									isLoaded_;
	Point									bbMax;
	Point									bbMin;
	vector<unsigned int>					faceIndices;
	OpenMesh::FPropHandleT<Vec3f>			vectorFieldFaceProperty;

private:
	void surroundBoundingBox();
	void updateFaceIndices();
	void assignVectorField();
};

