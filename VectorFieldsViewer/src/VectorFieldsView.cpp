//=============================================================================
//                                                
//   Program for viewing vector fields
//                                                                            
//=============================================================================

#pragma warning( push )
#pragma warning( disable : 4101)
#include "VectorFieldsWindow.h"
#pragma warning( pop ) 




[System::STAThread]
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE |GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ALPHA);

	VectorFieldsWindow::initInstance("Vector Fields Viewer", 512, 512);
	glutMainLoop();
}
