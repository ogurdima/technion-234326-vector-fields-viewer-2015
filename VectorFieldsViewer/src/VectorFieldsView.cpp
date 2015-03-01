//=============================================================================
//                                                
//   Program for viewing vector fields
//                                                                            
//=============================================================================

#include "VectorFieldsWindow.h"




[System::STAThread]
int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	VectorFieldsWindow::initInstance("Vector Fields Viewer", 512, 512);
	glutMainLoop();
}
