//=============================================================================
//                                                
//   Program for viewing vector fields
//                                                                            
//=============================================================================

#include "VectorFieldsViewer.h"



int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	VectorFieldsViewer window("Vector Fields Viewer", 512, 512);
	if (argc>1) {
		window.open_mesh(argv[1]);
	}
	glutMainLoop(); 
}
