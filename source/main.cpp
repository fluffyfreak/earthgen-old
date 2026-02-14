
#include <cstdio>
#include <cassert>
#include <sstream>

// GLew
#include "glew.h"

// Include GLFW
#include "glfw3.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// IMGUI
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "utils.h"

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

#define USE_POLAR_COORDS 0

#define M_PI   3.14159265358979323846264338327950288f
#define DEGTORAD(angleInDegrees) ((angleInDegrees) * M_PI / 180.0f)
#define RADTODEG(angleInRadians) ((angleInRadians) * 180.0f / M_PI)

namespace NKeyboard {
	enum EKeyStates {
		eKeyUnset = 0,
		eKeyPressed,
		eKeyHeld,
		eKeyReleased
	};
#define NUM_KEYS 256
	EKeyStates g_keys[NUM_KEYS] = { eKeyUnset };
	EKeyStates g_keysPrev[NUM_KEYS] = { eKeyUnset };
	void UpdateKeyStates(GLFWwindow* window) {
		for (int k = 0; k < NUM_KEYS; k++)
		{
			g_keysPrev[k] = g_keys[k];
			g_keys[k] = (glfwGetKey(window, k) == GLFW_PRESS) ? eKeyPressed : eKeyReleased;
		}
	}
	EKeyStates GetKeyState(const uint8_t c)
	{
		switch (g_keys[c]) {
		case eKeyUnset:		return eKeyUnset;
		case eKeyPressed:	return g_keysPrev[c] == eKeyPressed ? eKeyHeld : eKeyPressed;
		case eKeyReleased:	return g_keysPrev[c] == eKeyReleased ? eKeyUnset : eKeyReleased;
		default: assert(false && "this shouldn't happen!"); break;
		}
		return eKeyUnset;
	}
};

namespace {
	GLFWwindow* PrimaryWindow = nullptr;
}

static bool s_bShouldClose = false;
void WindowCloseFunc(GLFWwindow*)
{
	s_bShouldClose = true;
}

static int mouseW = 0;
void ScrollFunc(GLFWwindow* pWnd, double x, double y)
{
	mouseW = int(y) * 10;
}

bool InitWindow()
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWmonitor* PrimaryMonitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(PrimaryMonitor);
	const int screen_height = mode->height - (mode->height >> 2);
	const int screen_width = Clamp((double)(mode->width - (mode->width >> 2)), (double)screen_height, screen_height * 1.7777778);
	const int screen_x_offset = (mode->width - screen_width) >> 1;
	const int screen_y_offset = (mode->height - screen_height) >> 1;

	PrimaryWindow = glfwCreateWindow(screen_width, screen_height, "GLSLPlanet", NULL, NULL);

	// Open a window and create its OpenGL context
	if (!PrimaryWindow)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(PrimaryWindow);
	glfwSwapInterval(1);
	glfwSetWindowCloseCallback(PrimaryWindow, WindowCloseFunc);

	int major, minor, rev;
	glfwGetVersion(&major, &minor, &rev);

	const char* pVerStr = (char*)glGetString(GL_VERSION);
	const char* pVenStr = (char*)glGetString(GL_VENDOR);
	const char* pRenStr = (char*)glGetString(GL_RENDERER);
	const char* pSLVStr = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	std::stringstream log;
	log << "--------------------------\n" <<
		"OpenGL details: \n -- VERSION: \"" << pVerStr << "\"\n -- VENDOR: \"" << pVenStr << "\"\n" <<
		" -- RENDERER: " << pRenStr << "\"\n -- SHADING LANGUAGE VERSION: " << pSLVStr << std::endl;


	//const GLboolean bInitOk = GLeeInit();
	//assert(bInitOk == GL_TRUE);

	glewExperimental = true;
	GLenum glew_err;
	if ((glew_err = glewInit()) != GLEW_OK) {
		log << "GLEW initialisation failed: " << glewGetErrorString(glew_err) << std::endl;
		assert(false);
	}

	// TODO
	//_mkdir("./logs");
	//textFileWrite("./logs/opengl.log", log.str().c_str());

	int MaxVertexTextureImageUnits;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &MaxVertexTextureImageUnits);
	int MaxCombinedTextureImageUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MaxCombinedTextureImageUnits);

	glfwSetWindowTitle(PrimaryWindow, "EarthGen");
	//GLFWimage icons[1];
	//glfwSetWindowIcon(PrimaryWindow, 1, &icons[0]);

	glfwSetWindowPos(PrimaryWindow, screen_x_offset, screen_y_offset);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(PrimaryWindow, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(PrimaryWindow, screen_width / 2, screen_height / 2);


	// Set color and depth clear value
	glClearDepth(1.0f);
	// Dark blue background
	glClearColor(0.0f, 0.05f, 0.1f, 0.0f);
	//glClearColor(100/255.f, 149/255.f, 237/255.f, 0.0f);
	checkGLError();

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	checkGLError();

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	checkGLError();
	//glDisable(GL_CULL_FACE);

	glfwSetScrollCallback(PrimaryWindow, ScrollFunc);

	return true;
}

/*void HackyImguiWindow(TerrainMesh* _pSphere)
{
	// Exceptionally add an extra assert here for people confused about initial Dear ImGui setup
	// Most functions would normally just assert/crash if the context is missing.
	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing Dear ImGui context. Refer to examples app!");

	bool* p_open = nullptr;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoNav;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	window_flags |= ImGuiWindowFlags_NoDecoration;
	window_flags |= ImGuiWindowFlags_NoInputs;
	window_flags |= ImGuiWindowFlags_NoSavedSettings;
	//if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;
	//if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("Menu", p_open, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	// Basic info
	ImGuiContext& g = *ImGui::GetCurrentContext();
	ImGuiIO& io = g.IO;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	//SameLine(); if (SmallButton("GC")) { g.GcCompactAll = true; }

	ImGui::Text("fpsCamera pos (%.2f | %.2f | %.2f)", fpsCamera.pos.x, fpsCamera.pos.y, fpsCamera.pos.z);
	ImGui::Text("samplingPos   (%.2f | %.2f | %.2f)", samplingPos.x, samplingPos.y, samplingPos.z);

	ImGui::Text("Terrain Split Requests - %ld, Results - %ld", _pSphere->GetNumSplitRequestsPending(), _pSphere->GetNumSplitResultsPending());

	ImGui::Separator();

	// Always end
	ImGui::End();
}*/



int main (int argv, char **args) {
	//QApplication app(argv, args);
	//earthgen::MainWindow window;
	//window.setWindowIcon(QIcon("icon.png"));
	//window.show();
	if (!InitWindow())
		return 1;

	//char windowTitle[256] = { 0 };
	//snprintf(windowTitle, 256, "EarthGen");
	//glfwSetWindowTitle(PrimaryWindow, windowTitle);

	do
	{
		////////////////////////////////////////////////////////////////
		// handle resizing the screen/window
		int	screenWide = 0;
		int	screenHigh = 0;
		glfwGetWindowSize(PrimaryWindow, &screenWide, &screenHigh);
		//screenWidef = float(screenWide);
		//screenHighf = float(screenHigh);
		//aspect = screenWidef / screenHighf;

		////////////////////////////////////////////////////////////////
		// Render the main scene
		glViewport(0, 0, screenWide, screenHigh);
		checkGLError();

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		checkGLError();

		////////////////////////////////////////////////////////////////
		// render the bloody sphere here
		//if (bUseWireframe) {
		//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//	//pSphere->Render(ViewMatrix, ModelMatrix, MVP);
		//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//}
		//else {
		//	//pSphere->Render(ViewMatrix, ModelMatrix, MVP);
		//}

		glUseProgram(0);
		checkGLError();

		// Swap buffers
		glfwSwapBuffers(PrimaryWindow);
		checkGLError();
		
		glfwPollEvents();
		NKeyboard::UpdateKeyStates(PrimaryWindow);
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(PrimaryWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS && (false == s_bShouldClose));

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;// app.exec();
}
