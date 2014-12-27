#include "FieldedMesh.h"

FieldedMesh::FieldedMesh(void) : 
	isLoaded_(false), 
	bbMin(.0f), 
	bbMax(.0f)
{
	request_face_normals();
	request_vertex_normals();
	request_vertex_colors();
	add_property(vectorFieldFaceProperty);
}

FieldedMesh::~FieldedMesh(void)
{

}

bool FieldedMesh::load(const char* path)
{
	isLoaded_ = false;
	// load mesh
	if (OpenMesh::IO::read_mesh(*this, path))
	{
		surroundBoundingBox();

		// compute face & vertex normals
		update_normals();

		// update face indices for faster rendering
		updateFaceIndices();

		// compute and assign random vector field
		assignVectorField();

		isLoaded_ = true;
	}
	return isLoaded_;
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
void FieldedMesh::assignVectorField()
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
		VectorFieldTimeVal vfield;

		vfield.f = (inStd - center);
		vfield.t = 0;
		faceVectorField.push_back(vfield);
		vfield.t = 1;
		faceVectorField.push_back(vfield);

		Vec3f nrm = normal(cfit.handle());
		double mustbezero = vfield.f | nrm;
		if (!VectorFieldsUtils::isCloseToZero(mustbezero)) {
			bool ok = false;
		}

		property(vectorFieldFaceProperty, cfit.handle()) = faceVectorField;

	}
}

Triangle FieldedMesh::getFacePoints(OpenMesh::ArrayKernel::FaceHandle faceHandle)
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

void FieldedMesh::surroundBoundingBox()
{
	// set center and radius
	ConstVertexIter  v_it(vertices_begin()), v_end(vertices_end());

	bbMin = bbMax = point(v_it);
	for (; v_it!=v_end; ++v_it)
	{
		bbMin.minimize(point(v_it));
		bbMax.maximize(point(v_it));
	}
}

Vec3f FieldedMesh::faceVectorField(const Mesh::FaceHandle& faceHandle, const Time& time) const
{
	vector<VectorFieldTimeVal> fieldSamples = property(vectorFieldFaceProperty, faceHandle);
	if (fieldSamples.size() == 0) {
		return Vec3f(0,0,0);
	}

	// assuming samples are listed in increasing time order
	Time	prevTime = fieldSamples[0].t;
	Time	nextTime = fieldSamples[0].t;
	Vec3f	prevField = fieldSamples[0].f;
	Vec3f	nextField = fieldSamples[0].f;

	for (uint i = 0; i < fieldSamples.size(); i++) {
		nextTime = fieldSamples[i].t;
		nextField = fieldSamples[i].f;
		if (nextTime >= time) {
			break;
		}
		prevTime = fieldSamples[i].t;
		prevField = fieldSamples[i].f;
	}

	if (abs(nextTime - prevTime) < NUMERICAL_ERROR_THRESH) {
		return (nextField + prevField) / 2.0;
	}
	return (prevField * (time - prevTime) + nextField * (nextTime - time)) / (nextTime - prevTime);

}

bool FieldedMesh::isLoaded()
{
	return isLoaded_;
}

const vector<unsigned int>& FieldedMesh::getIndices() const
{
	return faceIndices;
}