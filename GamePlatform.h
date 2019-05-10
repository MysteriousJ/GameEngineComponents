#ifndef PLATFORM_HEADER
#define PLATFORM_HEADER
#include "general/BasicTypes.h"
#include "goblin3D/Goblin3D.h"
#include <vector>
#include <string>
#include <fstream>

#ifdef SDL
	#include "libraries/SDL/SDL_main.h"
	#include "libraries/SDL/SDL.h"
	#include "libraries/SDL/SDL_mouse.h"
	#include "libraries/SDL/SDL_keyboard.h"
	#include "libraries/SDL/SDL_joystick.h"
	#include "libraries/SDL/SDL_timer.h"
#endif
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

namespace platform {
struct Window
{
	enum ContextType {none, gl, d3d};

	#ifdef SDL
		SDL_Window* window;
		SDL_GLContext sdlglContext;
	#else
		HWND hwnd;
	#endif

	#ifdef D3D
		IDXGISwapChain* swapChain;
	#endif

	ContextType contextType;
	unsigned int width;
	unsigned int height;
};

struct WindowMessages
{
	bool quit;
	bool resized;
};

struct File
{
	char* bytes;
	size_t byteCount;
};

struct FileSystem
{
	// Directories or asset archives
	std::vector<std::string> fileLocations;
};

struct Input
{
	struct Button {
		bool pressed; // True for one update when the button is first pressed.
		bool down;    // True while the button is held down.
	};
	struct Mouse {
		goblin::Vec2 position; // In window client space.
		goblin::Vec2 deltaPosition;
		Button leftButton, middleButton, rightButton;
	};
	struct Gamepad {
		uint buttonCount;
		Button *buttons;
		uint axisCount;
		float *axes;
		#ifdef SDL
			SDL_Joystick* joystick;
		#endif
	};

	Mouse mouse;
	static const uint8 supportedKeyCount = 0xFF;
	Button keyboard[supportedKeyCount];
	Gamepad gamepad;
	uint64 systemTime;
	double runTime;
	float deltaTime;
};

struct Thread
{
	#ifdef SDL
		SDL_Thread* pointer;
	#else
		HANDLE handle;
	#endif	
};

struct Mutex
{
	#ifdef SDL
		SDL_mutex* mutex;
	#else
		//#error
	#endif
};

struct CondVar
{
	#ifdef SDL
		SDL_cond* condVar;
	#else
		//#error
	#endif
};

struct ThreadPool
{
	struct Job {
		enum Status { enqueued, started, finished };
		Status status;
		int(*startRoutine)(void*);
		void* param;
		CondVar cond;
		Mutex mutex;
	};
	struct Worker {
		Thread thread;

	};
	struct WorkerParams {
		ThreadPool* pool;
		int workerIndex;
	};

	uint workerCount;
	Thread* workers;
	DynamicArray<Job> jobQueue;
	Mutex queueMutex;
	CondVar queueCond;
};

// Key codes for indexing the array of keys in Input
enum KeyIndex {
	#ifdef SDL
		KeyIndexA = SDL_SCANCODE_A,
		KeyIndexD = SDL_SCANCODE_D,
		KeyIndexE = SDL_SCANCODE_E,
		KeyIndexN = SDL_SCANCODE_N,
		KeyIndexP = SDL_SCANCODE_P,
		KeyIndexQ = SDL_SCANCODE_Q,
		KeyIndexR = SDL_SCANCODE_R,
		KeyIndexS = SDL_SCANCODE_S,
		KeyIndexW = SDL_SCANCODE_W,
		KeyIndexX = SDL_SCANCODE_X,
		KeyIndexY = SDL_SCANCODE_Y,
		KeyIndexZ = SDL_SCANCODE_Z,
		KeyIndex0 = SDL_SCANCODE_0,
		KeyIndex1 = SDL_SCANCODE_1,
		KeyIndex2 = SDL_SCANCODE_2,
		KeyIndex3 = SDL_SCANCODE_3,
		KeyIndex4 = SDL_SCANCODE_4,
		KeyIndex5 = SDL_SCANCODE_5,
		KeyIndex6 = SDL_SCANCODE_6,
		KeyIndex7 = SDL_SCANCODE_7,
		KeyIndex8 = SDL_SCANCODE_8,
		KeyIndex9 = SDL_SCANCODE_9,
		KeyIndexEnter = SDL_SCANCODE_RETURN,
		KeyIndexEscape = SDL_SCANCODE_ESCAPE,
		KeyIndexSpace = SDL_SCANCODE_SPACE,
		KeyIndexUp = SDL_SCANCODE_UP,
		KeyIndexDown = SDL_SCANCODE_DOWN,
		KeyIndexLeft = SDL_SCANCODE_LEFT,
		KeyIndexRight = SDL_SCANCODE_RIGHT,
		KeyIndexF1 = SDL_SCANCODE_F1,
		KeyIndexF2 = SDL_SCANCODE_F2,
		KeyIndexF3 = SDL_SCANCODE_F3,
		KeyIndexF4 = SDL_SCANCODE_F4,
		KeyIndexF5 = SDL_SCANCODE_F5,
		KeyIndexF6 = SDL_SCANCODE_F6,
		KeyIndexF7 = SDL_SCANCODE_F7,
		KeyIndexF8 = SDL_SCANCODE_F8,
		KeyIndexF9 = SDL_SCANCODE_F9,
		KeyIndexF10 = SDL_SCANCODE_F10,
		KeyIndexF11 = SDL_SCANCODE_F11,
		KeyIndexF12 = SDL_SCANCODE_F12,
	#else
		KeyIndexA = 'A',
		KeyIndexD = 'D',
		KeyIndexE = 'E',
		KeyIndexN = 'N',
		KeyIndexP = 'P',
		KeyIndexQ = 'Q',
		KeyIndexR = 'R',
		KeyIndexS = 'S',
		KeyIndexW = 'W',
		KeyIndexX = 'X',
		KeyIndexZ = 'Z',
		KeyIndex0 = '0',
		KeyIndex1 = '1',
		KeyIndex2 = '2',
		KeyIndex3 = '3',
		KeyIndex4 = '4',
		KeyIndex5 = '5',
		KeyIndex6 = '6',
		KeyIndex7 = '7',
		KeyIndex8 = '8',
		KeyIndex9 = '9',
		KeyIndexEnter = VK_RETURN,
		KeyIndexEscape = VK_ESCAPE,
		KeyIndexSpace = VK_SPACE,
		KeyIndexControl = VK_CONTROL,
		KeyIndexUp = VK_UP,
		KeyIndexDown = VK_DOWN,
		KeyIndexLeft = VK_LEFT,
		KeyIndexRight = VK_RIGHT,
		KeyIndexF1 = VK_F1,
		KeyIndexF2 = VK_F2,
		KeyIndexF3 = VK_F3,
		KeyIndexF4 = VK_F4,
		KeyIndexF5 = VK_F5,
		KeyIndexF6 = VK_F6,
		KeyIndexF7 = VK_F7,
		KeyIndexF8 = VK_F8,
		KeyIndexF9 = VK_F9,
		KeyIndexF10 = VK_F10,
		KeyIndexF11 = VK_F11,
		KeyIndexF12 = VK_F12
	#endif
};


#ifdef WIN32
LRESULT CALLBACK WindowProc(
	HWND   hwnd,
	UINT   msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

FUNCTION_DEF void createWindow(Window *window, uint width, uint height, Window::ContextType contextType)
{
	Window zero = {0};
	*window = zero;
	window->contextType = contextType;
#ifdef SDL
	SDL_Init(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	// TODO: on my laptop with IntelHD graphics, it appears the depth buffer
	// is being created with very few bits, leading to z fighting on everything.
	// This is only a problem with SDL! Win32 version works fine. Need something
	// to set the depth buffer size, but this causes an immediate crash:
	// SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

	window->window = SDL_CreateWindow(0, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	// Create OpenGL context
	assert(contextType == Window::ContextType::gl);
	window->sdlglContext = SDL_GL_CreateContext(window->window);
	// Load openGL extentions
	gladLoadGL();
#else
	HINSTANCE hInstance = GetModuleHandle(0);
	WNDCLASS wnd ={0};
	wnd.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = WindowProc;
	wnd.lpszClassName = L"GoblinWindowClass";
	wnd.hCursor = LoadCursor(0, IDC_ARROW);

	RegisterClass(&wnd);

	// CreateWindowEx takes the total size of the window, so we need to calculate how big the window should be to produce the desired client area
	RECT clientArea = {0, 0, (LONG)width, (LONG)height};
	AdjustWindowRect(&clientArea, WS_OVERLAPPEDWINDOW, FALSE);

	window->hwnd = CreateWindowEx(
		0,
		wnd.lpszClassName,
		NULL,
		WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		clientArea.right - clientArea.left,
		clientArea.bottom - clientArea.top,
		0,
		0,
		hInstance,
		0);

	// Create graphics context
	if (contextType == Window::ContextType::gl)
	{
		#ifdef OPENGL
			// Create pixel buffer for OpenGL
			PIXELFORMATDESCRIPTOR pfd ={0};
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 24;
			pfd.cDepthBits = 32;

			HDC deviceContext = GetDC(window->hwnd);
			int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
			SetPixelFormat(deviceContext, pixelFormat, &pfd);

			HGLRC renderingContext = wglCreateContext(deviceContext);
			if (renderingContext) {
				wglMakeCurrent(deviceContext, renderingContext);
			}
			// Load openGL extentions
			gladLoadWGL(deviceContext);
			gladLoadGL();
			ReleaseDC(window->hwnd, deviceContext);
		#endif
	}
	else if (contextType == Window::ContextType::d3d)
	{
		#ifdef D3D
			HRESULT hr = S_OK;
			ID3D11Device* device;
			ID3D11DeviceContext* deviceContext;
			IDXGISwapChain* swapChain;

			RECT rc;
			GetClientRect(mod_window->hwnd, &rc);
			UINT width = rc.right - rc.left;
			UINT height = rc.bottom - rc.top;

			UINT createDeviceFlags = 0;
			#ifdef _DEBUG
				createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			#endif

			D3D_DRIVER_TYPE driverTypes[] =
			{
				D3D_DRIVER_TYPE_HARDWARE,
				D3D_DRIVER_TYPE_WARP,
				D3D_DRIVER_TYPE_REFERENCE,
			};
			UINT numDriverTypes = ARRAYSIZE(driverTypes);

			D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
			};
			UINT numFeatureLevels = ARRAYSIZE(featureLevels);

			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 1;
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = mod_window->hwnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;

			for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
			{
				hr = D3D11CreateDeviceAndSwapChain(
					NULL,
					driverTypes[driverTypeIndex],
					NULL,
					createDeviceFlags,
					featureLevels,
					numFeatureLevels,
					D3D11_SDK_VERSION,
					&sd,
					&swapChain,
					&device,
					NULL,
					&deviceContext);

				if (SUCCEEDED(hr)) {
					break;
				}
			}

			createRenderStateD3D(out_rs, device, deviceContext, swapChain);
			mod_window->swapChain = swapChain;
			mod_window->contextType = GameWindow::ContextType::d3d;
		#endif
	}
#endif // WINDOWS
}

FUNCTION_DEF void destroyWindow(Window* window)
{
	#ifdef SDL
		SDL_DestroyWindow(window->window);
	#else
		// TODO: WINAPI version
	#endif
}

FUNCTION_DEF int getWindowWidth(Window window)
{
	#ifdef SDL
		int width;
		SDL_GetWindowSize(window.window, &width, 0);
		return width;
	#else
		RECT windowClientRect;
		GetClientRect(window.hwnd, &windowClientRect);
		return (int)windowClientRect.right;
	#endif
}

FUNCTION_DEF int getWindowHeight(Window window)
{
	#ifdef SDL
		int height;
		SDL_GetWindowSize(window.window, 0, &height);
		return height;
	#else
		RECT windowClientRect;
		GetClientRect(window.hwnd, &windowClientRect);
		return (int)windowClientRect.bottom;
	#endif
}

FUNCTION_DEF void setWindowTitle(Window window, const char* string)
{
	#ifdef SDL
		SDL_SetWindowTitle(window.window, string);
	#else
		SetWindowTextA(window.hwnd, string);
	#endif
}

FUNCTION_DEF void swapBuffers(Window* window, bool vSynch)
{
	#ifdef SDL
		SDL_GL_SetSwapInterval(vSynch ? 1 : 0);
		SDL_GL_SwapWindow(window->window);
	#else
	#ifdef GOBLIN_ENABLE_GL
		if (window->contextType == Window::ContextType::gl) {
			wglSwapIntervalEXT(vSynch ? 1 : 0);
			HDC deviceContext = GetDC(window->hwnd);
			SwapBuffers(deviceContext);
			ReleaseDC(window->hwnd, deviceContext);
		}
	#endif
	#ifdef GOBLIN_ENABLE_D3D
		if (window->contextType == Window::ContextType::d3d) {
			window->swapChain->Present(vSynch? 1 : 0, 0);
		}
	#endif
	#endif
}


FUNCTION_DEF WindowMessages processWindowMessages(Window* inout_win)
{
	WindowMessages result ={};
	#ifdef SDL
		SDL_Event message;
		while (SDL_PollEvent(&message)) {
			if (message.type == SDL_QUIT) {
				result.quit = true;
			}
		}

		// Check if the window has been resized
		int width;
		int height;
		SDL_GetWindowSize(inout_win->window, &width, &height);
		if (inout_win->width != width || inout_win->height != height) {
			inout_win->width = width;
			inout_win->height = height;
			result.resized = true;
		}
	#else
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				result.quit = true;
			}
		}

		// Check if the window has been resized
		RECT windowClientRect;
		GetClientRect(inout_win->hwnd, &windowClientRect);
		if (inout_win->width != windowClientRect.right || inout_win->height != windowClientRect.bottom) {
			inout_win->width = windowClientRect.right;
			inout_win->height = windowClientRect.bottom;
			result.resized = true;
		}
	#endif

	return result;
}

FUNCTION_DEF void addFileLocation(FileSystem* out_fs, std::string name)
{
	out_fs->fileLocations.push_back(name);
}


FUNCTION_DEF bool loadFile(File* out_file, std::string fileName)
{
	std::ifstream file(fileName.c_str(), std::ifstream::in|std::ifstream::binary|std::ifstream::ate);
	if (!file){
		*out_file ={0};
		return false;
	}
	// The file is opened with its read position set at the end, so tellg() returns the length
	std::streampos fileSize = file.tellg();
	// Move read position to the beginning of the file
	file.seekg(std::ifstream::beg);

	char* buffer = new char[(unsigned int)fileSize];
	file.read(buffer, fileSize);

	*out_file ={buffer, (size_t)fileSize};
	return true;

	/* C implementation
	fopen_s(&file, fullPath.c_str(), "rb");
	if (file) {
		// Get number of bytes in file
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		// Go back to beginning of file
		fseek(file, 0, SEEK_SET);

		out_file->byteCount = fileSize;
		out_file->bytes = new char[fileSize];
		fread(out_file->bytes, 1, fileSize, file);
		fclose(file);
		return true;
	}*/
}

FUNCTION_DEF File loadFile(std::string fileName)
{
	File newFile;
	loadFile(&newFile, fileName);
	return newFile;
}

FUNCTION_DEF bool loadFile(File *out_file, FileSystem& fs, std::string fileName)
{
	// Look in each location in the FileSystem for the file
	for (uint i=0; i<fs.fileLocations.size(); i++)
	{
		std::string fullPath = fs.fileLocations[i] + "/" + fileName;
		if (loadFile(out_file, fullPath)) {
			return true;
		}
	}
	*out_file = {};
	return false;
}

FUNCTION_DEF void releaseFile(File* file)
{
	if (file->bytes) {
		delete[] file->bytes;
	}
	file->bytes = 0;
	file->byteCount = 0;
}

FUNCTION_DEF std::string getFileExtensionString(std::string fullFileName)
{
	int lastDotPosition = fullFileName.find_last_of('.');
	return fullFileName.substr(lastDotPosition+1);
}

FUNCTION_DEF int64 getGlobalTime()
{
	#ifdef SDL
		return SDL_GetPerformanceCounter();
	#else
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		return time.QuadPart;
	#endif
}

FUNCTION_DEF int64 getTicksPerSecond()
{
	#ifdef SDL
		return SDL_GetPerformanceFrequency();
	#else
		LARGE_INTEGER ticksPerSecond;
		QueryPerformanceFrequency(&ticksPerSecond);
		return ticksPerSecond.QuadPart;
	#endif
}

FUNCTION_DEF void updateButton(Input::Button* inout_button, unsigned int isDown)
{
	if (isDown) {
		inout_button->pressed = !inout_button->down;
		inout_button->down = true;
	}
	else {
		inout_button->pressed = false;
		inout_button->down = false;
	}
}

FUNCTION_DEF void updateInput(Input* out_input, Window window, double maxDeltaTime)
{
	// Mouse
	#ifdef SDL
		int mouseX, mouseY;
		unsigned int buttonState = SDL_GetMouseState(&mouseX, &mouseY);
		out_input->mouse.deltaPosition.x = float(mouseX)-out_input->mouse.position.x;
		out_input->mouse.deltaPosition.y = float(mouseY)-out_input->mouse.position.y;
		out_input->mouse.position.x = (float)mouseX;
		out_input->mouse.position.y = (float)mouseY;
		updateButton(&out_input->mouse.leftButton, buttonState & SDL_BUTTON(SDL_BUTTON_LEFT));
		updateButton(&out_input->mouse.rightButton, buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT));
		updateButton(&out_input->mouse.middleButton, buttonState & SDL_BUTTON(SDL_BUTTON_MIDDLE));

	#else
		POINT mousePosition;
		GetCursorPos(&mousePosition);
		ScreenToClient(window.hwnd, &mousePosition);
		out_input->mouse.deltaPosition.x = float(mousePosition.x)-out_input->mouse.position.x;
		out_input->mouse.deltaPosition.y = float(mousePosition.y)-out_input->mouse.position.y;
		out_input->mouse.position.x = float(mousePosition.x);
		out_input->mouse.position.y = float(mousePosition.y);

		updateButton(&out_input->mouse.leftButton, (1<<16)&GetKeyState(VK_LBUTTON));
		updateButton(&out_input->mouse.rightButton, (1<<16)&GetKeyState(VK_RBUTTON));
		updateButton(&out_input->mouse.middleButton, (1<<16)&GetKeyState(VK_MBUTTON));
	#endif

	// Keyboard
	#ifdef SDL
		int sdlKeyCount;
		const Uint8 *keystates = SDL_GetKeyboardState(&sdlKeyCount);
		forloop(i, platform::Input::supportedKeyCount)
		{
			int isKeyDown = 0;
			// Make sure we don't read past the end of SDL's key array
			if (i < (uint)sdlKeyCount) {
				isKeyDown = keystates[i];
			}
			updateButton(&out_input->keyboard[i], isKeyDown);
		}
	#else
		// Only update if our window is active
		if (window.hwnd == GetActiveWindow())
		{
			forloop(i, Input::supportedKeyCount)
			{
				int isKeyDown = (1<<16)&GetKeyState(i);
				updateButton(&out_input->keyboard[i], isKeyDown);
			}
		}
	#endif

	// Gamepad
	#ifdef SDL
		// Check if a joystick has been plugged in
		if (!out_input->gamepad.joystick && SDL_NumJoysticks() > 0) {
			out_input->gamepad.joystick = SDL_JoystickOpen(0);
			if (out_input->gamepad.joystick) {
				out_input->gamepad.buttonCount = SDL_JoystickNumButtons(out_input->gamepad.joystick);
				out_input->gamepad.buttons = new platform::Input::Button[out_input->gamepad.buttonCount];
				out_input->gamepad.axisCount = SDL_JoystickNumAxes(out_input->gamepad.joystick);
				out_input->gamepad.axes = new float[out_input->gamepad.axisCount];
			}
		}
		// Check if the joystick has been disconnected
		if (out_input->gamepad.joystick && !SDL_JoystickGetAttached(out_input->gamepad.joystick)) {
			SDL_JoystickClose(out_input->gamepad.joystick);
			out_input->gamepad.joystick = 0;
			delete[] out_input->gamepad.buttons;
			delete[] out_input->gamepad.axes;
			platform::Input::Gamepad zero ={0};
			out_input->gamepad = zero;
		}

		// Update Gamepad
		SDL_JoystickUpdate();
		// Buttons
		forloop(i, out_input->gamepad.buttonCount) {
			updateButton(&out_input->gamepad.buttons[i], SDL_JoystickGetButton(out_input->gamepad.joystick, i));
		}
		// Axes
		forloop(i, out_input->gamepad.axisCount) {
			// Map axis value to [-1,1]
			const float maxSigned16bitInt = 32768;
			out_input->gamepad.axes[i] = float(SDL_JoystickGetAxis(out_input->gamepad.joystick, i)) / maxSigned16bitInt;
		}
	#endif // SDL

	// Time
	uint64 newTime = getGlobalTime();
	uint64 previousTime = out_input->systemTime;
	uint64 ticksPerSecond = getTicksPerSecond();
	if (newTime > previousTime) {
		double deltaTime = double(newTime - previousTime) / double(ticksPerSecond);
		if (deltaTime > maxDeltaTime) deltaTime = maxDeltaTime;
		out_input->runTime += deltaTime;
		out_input->deltaTime = (float)deltaTime;
	}
	else {
		// If time is uninitialized or wrapped around, don't change delta time
		out_input->runTime += out_input->deltaTime;
	}
	out_input->systemTime = newTime;
}

FUNCTION_DEF void createThread(Thread* thread, int(*startRoutine)(void*), void* parameter)
{
	#ifdef SDL
		thread->pointer = SDL_CreateThread(startRoutine, 0, parameter);
	#else
		thread->handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)startRoutine, (LPVOID)parameter, 0, 0);
	#endif
}

FUNCTION_DEF void detachThread(Thread* thread)
{
	#ifdef SDL
		SDL_DetachThread(thread->pointer);
	#else
		CloseHandle(thread->handle);
	#endif
	*thread = {0};
}

FUNCTION_DEF void
joinThread(Thread* thread)
{
	#ifdef SDL
		SDL_WaitThread(thread->pointer, 0);
	#else
		WaitForSingleObject(thread->handle, INFINITE);
	#endif
	*thread = {0};
}

/*FUNCTION_DEF void
createMutex()
{

}

FUNCTION_DEF void
destroyMutex()
{

}

FUNCTION_DEF void
lockMutex()
{

}

FUNCTION_DEF void
unlock_mutex()
{

}

FUNCTION_DEF int
threadJobIdleLoop(void* param)
{
	ThreadPool::WorkerParams* params = (ThreadPool::WorkerParams*) param;
	ThreadPool* pool = params->pool;

	while (1)
	{
		lockMutex(pool->queueMutex);
		// Find the first job in the list that hasn't started
		ThreadPool::Job* job = 0;
		for (uint i=pool->jobQueue.size(); i>=0; --i) {
			if (pool->jobQueue[i].status == ThreadPool::Job::enqueued) {
				job = &pool->jobQueue[i];
			}
		}
		
		if (job) {
			lockMutex(job->mutex);
			job->status = ThreadPool::Job::started;
			unlockMutex(job->mutex);
			job->startRoutine(job->param);
			lockMutex(job->mutex);
			job->status = ThreadPool::Job::finished;
			condVarBroadcast(job->finishCond);
			unlockMutex(job->mutex);
		}
		else {
			condVarWait(pool->queueCond, pool->queueMutex);
		}
		unlockMutex(pool->queueMutex);
	}
	return 0;
}

FUNCTION_DEF void
createThreadPool(ThreadPool* pool)
{
	*pool = {0};
	pool->threadCount = SDL_GetCPUCount();
	pool->threads = ALLOC_ARRAY(Thread, pool->threadCount);
	forloop(i, pool->threadCount) {
		createThread(&pool->threads[i], threadJobIdleLoop, pool);
	}
	pool->queueCond = SDL_CreateCond();
	pool->queueMutex = SDL_CreateMutex();
}

FUNCTION_DEF ThreadJobHandle
addThreadJob(ThreadPool* pool, int(*startRoutine)(void*), void* parameter)
{
	ThreadJobHandle jobHandle = {0};
	ThreadPool::Job job ={0};
	job.param = parameter;
	job.startRoutine = startRoutine;
	pool->jobQueue.push_front(job);
	return jobHandle;
}

FUNCTION_DEF void
joinThreadJob(ThreadJobHandle job)
{
	while (!job->worker->done) {
		condWait(job->worker->finishCond);
		condBroadcast(job->worker->restartCond);
	}
}*/


} // namespace
#endif // include guard