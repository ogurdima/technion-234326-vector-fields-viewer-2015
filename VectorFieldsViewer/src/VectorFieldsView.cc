//=============================================================================
//                                                
//   Program for viewing vector fields
//                                                                            
//=============================================================================

#pragma warning( push )
#pragma warning( disable : 4101)
#include "VectorFieldsViewer.h"
#pragma warning( pop ) 





int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE |GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ALPHA);

	VectorFieldsViewer window("Vector Fields Viewer", 512, 512);
	if (argc>1) 
	{
		window.open_mesh(argv[1]);
	}
	glutMainLoop(); 
}
