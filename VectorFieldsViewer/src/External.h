_declspec(dllexport) void OpenWindow(void (*changedDrawStateCallback)(int),
									 void (*openMeshCallback)(char*),
									 void (*changedMeshColorCallback)(float,float,float,float),
									 void (*changedFieldColorCallback)(float[4], float[4]),
									 void (*changedVisualizationCallback)(int, double, double),
									 void (*recomputePathsCallback)(char* path, bool isConst, bool normalize, double step, double min, double max),
									 void (*printScreenCallback)(char* path, int howMany));

_declspec(dllexport) void UpdateDrawStateGui(int drawState);

_declspec(dllexport) void UpdateCurrentTimeGui(double currentTime);
