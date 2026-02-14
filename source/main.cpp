
#include <cstdio>
#include <cassert>
#include <sstream>

#include "application.h"

// Making this run in nVidia fixes everything, it's fucked on Intel UHD... suspect there's something wrong with the shaders in that case
#if 1
typedef unsigned long       DWORD;
extern "C" {
	// This is the quickest and easiest way to enable using the nVidia GPU on a Windows laptop with a dedicated nVidia GPU and Optimus tech.
	// enable optimus!
	// https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus.htm
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// AMD have one too!!!
	// https://gpuopen.com/amdpowerxpressrequesthighperformance/
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#endif



int main (int argv, char **args) {
	Application app(argv, args);
	app.Init();
	return app.Run();

	//earthgen::MainWindow window;
	//window.setWindowIcon(QIcon("icon.png"));
	//window.show();

	// app.exec();
}
