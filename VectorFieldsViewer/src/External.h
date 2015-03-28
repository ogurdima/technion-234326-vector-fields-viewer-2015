_declspec(dllexport) void OpenWindow(void (*changedDrawStateCallback)(int),
									 void (*openMeshCallback)(char*),
									 void (*changedMeshColorCallback)(float,float,float,float),
									 void (*changedFieldColorCallback)(float,float,float,float),
									 void (*changedVisualizationCallback)(int, double, double),
									 void (*recomputePathsCallback)(char* path, bool isConst, double step, double min, double max));

_declspec(dllexport) void UpdateDrawStateGui(int drawState);