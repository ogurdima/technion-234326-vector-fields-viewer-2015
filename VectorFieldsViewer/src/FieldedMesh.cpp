#include "FieldedMesh.h"

FieldedMesh::FieldedMesh(void) : 
	isLoaded_(false), 
	bbMin(.0f), 
	bbMax(.0f)
{
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
	for(ConstFaceIter cfit(faces_begin()), cfitEnd(faces_end()); cfit != cfitEnd; ++cfit) {
		float x = (float)rand();
		float y = (float)rand();
		float z = (float)rand();
		Vec3f field = Vec3f(x,y,z).normalized();

		property(vectorFieldFaceProperty, cfit.handle()) = field;


		/*for(Mesh::FaceVertexIter fvit = mesh_.fv_begin(fit.handle()); fvit != mesh_.fv_end(fit.handle()); ++fvit) {
		unsigned char r = floor((255 * field[0]));
		unsigned char g = floor((255 * field[1]));
		unsigned char b = floor((255 * field[2]));
		mesh_.set_color(fvit, Mesh::Color(r,g,b));
		}*/
	}
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
	Mesh::ConstVertexIter  v_it(vertices_begin()), v_end(vertices_end());

	bbMin = bbMax = point(v_it);
	for (; v_it!=v_end; ++v_it)
	{
		bbMin.minimize(point(v_it));
		bbMax.maximize(point(v_it));
	}
}

inline const Point& FieldedMesh::faceVectorField(const Mesh::FaceHandle& faceHandle) const
{
	return property(vectorFieldFaceProperty,faceHandle);
}

bool FieldedMesh::isLoaded()
{
	return isLoaded_;
}

const vector<unsigned int>& FieldedMesh::getIndices() const
{
	return faceIndices;
}