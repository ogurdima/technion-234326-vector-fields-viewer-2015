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

	void assignVectorField();

	inline const Point& faceVectorField(const Mesh::FaceHandle& face) const;

	const vector<unsigned int>& getIndices() const;

protected:
	bool isLoaded_;
	vector<unsigned int>  faceIndices;
	Point            bbMin, bbMax;

	OpenMesh::FPropHandleT<Vec3f> vectorFieldFaceProperty;

private:
	void surroundBoundingBox();
	void updateFaceIndices();

};

