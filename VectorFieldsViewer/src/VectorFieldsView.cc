//=============================================================================
//                                                
//   Program for viewing vector fields
//                                                                            
//=============================================================================

#pragma warning( push )
#pragma warning( disable : 4101)
#include "VectorFieldsViewer.h"
#pragma warning( pop ) 

using namespace std;


[System::STAThread]
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	//std::cout << (glGetError()) << std::endl;
	//glutCreateWindow("GLEW Test");
	//std::cout << (glGetError()) << std::endl;
	//glutInitContextVersion(1,6);
	//std::cout << (glGetError()) << std::endl;
	//GLenum err = glewInit();
	//std::cout << (err) << std::endl;
	//cout << (char*) glGetString(GL_VERSION) << endl;
	//if (GLEW_OK != err)
	//{
	//  /* Problem: glewInit failed, something is seriously wrong. */
	//  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	//  return 1;
	//}
	//fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	
	VectorFieldsViewer window("Vector Fields Viewer", 512, 512);
	if (argc>1) 
	{
		window.open_mesh(argv[1]);
	}
	glutMainLoop(); 
}
