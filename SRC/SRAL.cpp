#define SRAL_EXPORT
#include "../Include/SRAL.h"
#include "Engine.h"
#if defined(_WIN32)
#define UNICODE
#include "NVDA.h"
#include "SAPI.h"
#include "Jaws.h"
#include "UIA.h"
#include <windows.h>
#include <tlhelp32.h>
#elif defined(__APPLE__)
#include "AVSpeech.h"
#else
#include "SpeechDispatcher.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif
#include <vector>
#include <string>
#include <chrono>
#include <thread>

class Timer {
public:
	Timer() {
		restart();
	}

	inline uint64_t elapsed() {
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
	}

	inline void restart() {
		start_time = std::chrono::high_resolution_clock::now();
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};



Engine* g_currentEngine = nullptr;
std::vector<Engine*> g_engines;
int g_excludes = 0;
bool g_initialized = false;

struct QueuedOutput {
	const char* text;
	bool interrupt;
	bool braille;
	bool speak;
	bool ssml;
	int time;
	Engine* engine;
};
std::vector<QueuedOutput> g_delayedOutputs;
bool g_delayOperation = false;
bool g_outputThreadRunning = false;


uint64_t g_lastDelayTime = 0;


static void output_thread() {
	g_outputThreadRunning = true;
	Timer t;
	while (g_delayOperation && g_delayedOutputs.size() != 0) {
		for (uint64_t i = 0; i < g_delayedOutputs.size(); ++i) {
			t.restart();
			while (t.elapsed() < g_delayedOutputs[i].time && g_delayOperation) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
			if (g_delayedOutputs[i].speak) {
				if (g_delayedOutputs[i].ssml)
					g_delayedOutputs[i].engine->SpeakSsml(g_delayedOutputs[i].text, g_delayedOutputs[i].interrupt);
				else
					g_delayedOutputs[i].engine->Speak(g_delayedOutputs[i].text, g_delayedOutputs[i].interrupt);

			}
			else if (g_delayedOutputs[i].braille)
				g_delayedOutputs[i].engine->Braille(g_delayedOutputs[i].text);
		}
		if (g_delayedOutputs.size() != 0) {
			g_delayedOutputs.clear();
			g_delayOperation = false;
			break;
		}
	}
	g_outputThreadRunning = false;
}



bool g_keyboardHookThread = false;
bool g_shiftPressed = false;

#if defined(_WIN32)
static HHOOK g_keyboardHook;
static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;
		for (uint64_t i = 0; i < g_engines.size(); ++i) {
			if (g_engines[i] == nullptr || !g_engines[i]->GetActive()) continue;

			if (wParam == WM_KEYDOWN) {
				if ((pKeyInfo->vkCode == VK_LCONTROL || pKeyInfo->vkCode == VK_RCONTROL) && g_engines[i]->GetKeyFlags() & HANDLE_INTERRUPT) {
					g_engines[i]->StopSpeech();
				}
				else if ((pKeyInfo->vkCode == VK_LSHIFT || pKeyInfo->vkCode == VK_RSHIFT) && g_engines[i]->GetKeyFlags() & HANDLE_PAUSE_RESUME && g_shiftPressed == false) {
					if (g_engines[i]->paused)
						g_engines[i]->ResumeSpeech();
					else
						g_engines[i]->PauseSpeech();
					g_shiftPressed = true;
				}
			}
			else if (wParam == WM_KEYUP) {
				if (pKeyInfo->vkCode == VK_LSHIFT || pKeyInfo->vkCode == VK_RSHIFT) {
					g_shiftPressed = false;
				}
			}
		}
	}
	return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}
static void hook_thread() {
	g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);
	if (g_keyboardHook == nullptr)return;
	MSG msg;

	while (g_keyboardHookThread) {
		if (GetMessageW(&msg, nullptr, 0, 0)) {
			DispatchMessageW(&msg);
			TranslateMessage(&msg);
		}
	}
	UnhookWindowsHookEx(g_keyboardHook);
}
extern "C" SRAL_API bool SRAL_RegisterKeyboardHooks(void) {
	if (g_keyboardHookThread)return g_keyboardHookThread;
	g_keyboardHookThread = true;
	std::thread t(hook_thread);
	t.detach();
	Timer timer;
	while (timer.elapsed() < 3000) {
		Sleep(5);
		if (g_keyboardHook != nullptr) {
			return true;
		}
	}
	return false; // Timeout: Hook is not set
}

extern "C" SRAL_API void SRAL_UnregisterKeyboardHooks(void) {
	PostMessage(0, WM_KEYUP, 0, 0);
	g_keyboardHookThread = false;
}
#elif defined(__APPLE__)
extern "C" SRAL_API bool SRAL_RegisterKeyboardHooks(void) {
	return false;
}
extern "C" SRAL_API void SRAL_UnregisterKeyboardHooks(void) {
	return;
}
#else
Display* g_display = nullptr;
static void hook_thread() {

	g_display = XOpenDisplay(nullptr);

	if (g_display == nullptr)return;
	XGrabKeyboard(g_display, DefaultRootWindow(g_display), True, GrabModeAsync, GrabModeAsync, CurrentTime);
	XEvent event;

	while (g_keyboardHookThread) {
		if (XPending(g_display)) {
			XNextEvent(g_display, &event);
		}
		for (uint64_t i = 0; i < g_engines.size(); ++i) {
			if (g_engines[i] == nullptr || !g_engines[i]->GetActive()) continue;

			if (event.type == KeyPress) {
				KeySym key = XLookupKeysym(&event.xkey, 0);

				if ((key == XK_Control_L || key == XK_Control_R) && g_engines[i]->GetKeyFlags() & HANDLE_INTERRUPT) {
					g_engines[i]->StopSpeech();
				}
				else if ((key == XK_Shift_L || key == XK_Shift_R) && g_engines[i]->GetKeyFlags() & HANDLE_PAUSE_RESUME && g_shiftPressed == false) {
					if (g_engines[i]->paused)
						g_engines[i]->ResumeSpeech();
					else
						g_engines[i]->PauseSpeech();
					g_shiftPressed = true;
				}
			}
			else if (event.type == KeyRelease) {
				KeySym key = XLookupKeysym(&event.xkey, 0);

				if ((key == XK_Shift_L || key == XK_Shift_R) && g_shiftPressed) {
					g_shiftPressed = false;
				}
			}
		}
	}
	XCloseDisplay(g_display);
}
extern "C" SRAL_API bool SRAL_RegisterKeyboardHooks(void) {
	if (g_keyboardHookThread)return g_keyboardHookThread;
	g_keyboardHookThread = true;
	std::thread t(hook_thread);
	t.detach();
	Timer timer;
	while (timer.elapsed() < 3000) {
		usleep(5000);
		if (g_display != nullptr) {
			return true;
		}
	}
	return false; // Timeout: Hook is not set
}

extern "C" SRAL_API void SRAL_UnregisterKeyboardHooks(void) {
	g_keyboardHookThread = false;
}

#endif



extern "C" SRAL_API bool SRAL_Initialize(int engines_exclude) {
	if (g_initialized)return true;
#if defined(_WIN32)
	g_engines.push_back(new NVDA);
	g_engines.push_back(new Jaws);
	g_engines.push_back(new SAPI);
	g_engines.push_back(new UIA);
#elif defined(__APPLE__)
	g_engines.push_back(new AVSpeech);
#else
	g_engines.push_back(new SpeechDispatcher);
#endif
	bool found = false;
	for (uint64_t i = 0; i < g_engines.size(); ++i) {
		g_engines[i]->Initialize();
		if (g_engines[i]->GetActive() && !found && !(engines_exclude & g_engines[i]->GetNumber())) {
			g_currentEngine = g_engines[i];
			found = true;
		}
	}
	if (g_currentEngine == nullptr)return false;
	g_excludes = engines_exclude;
	g_initialized = found;
	return found;
}
extern "C" SRAL_API void SRAL_Uninitialize(void) {
	if (!g_initialized)return;
	for (uint64_t i = 0; i < g_engines.size(); ++i) {
		g_engines[i]->Uninitialize();
		delete g_engines[i];
	}
	g_currentEngine = nullptr;
	g_engines.clear();
	g_excludes = 0;
	if (g_keyboardHookThread) {
		SRAL_UnregisterKeyboardHooks();
		// Just for sure
		g_keyboardHookThread = false;
	}
	g_initialized = false;
}
#ifdef _WIN32
// This is used for find the Windows Narrator process
BOOL FindProcess(const wchar_t* name) {
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		return FALSE; // Snapshot failed
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process.
	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap); // Clean up the snapshot object
		return FALSE; // Unable to retrieve process information
	}

	// Now walk the snapshot of processes
	do {
		// Compare the process name with the input name
		if (_wcsicmp(pe32.szExeFile, name) == 0) {
			CloseHandle(hProcessSnap); // Clean up the snapshot object
			return TRUE; // Process found
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap); // Clean up the snapshot object
	return FALSE; // Process not found
}


#endif
static Engine* get_engine(int);
static void speech_engine_update() {
	if (!g_currentEngine->GetActive() || g_currentEngine->GetNumber() == ENGINE_SAPI || g_currentEngine->GetNumber() == ENGINE_UIA) {
#ifdef _WIN32
		if (FindProcess(L"narrator.exe") == TRUE) {
			g_currentEngine = get_engine(ENGINE_UIA);
			return;
		}
		else {
#endif
			for (uint64_t i = 0; i < g_engines.size(); ++i) {
				if (g_engines[i]->GetActive() && !(g_excludes & g_engines[i]->GetNumber())) {
					g_currentEngine = g_engines[i];
					break;
				}
			}
		}
#ifdef _WIN32
	}
#endif
}
static Engine* get_engine(int engine) {
#ifdef _WIN32
	if (engine == ENGINE_NARRATOR)return get_engine(ENGINE_UIA);
#endif
	bool found = false;
	uint64_t i;
	for (i = 0; i < g_engines.size(); ++i) {
		if (g_engines[i]->GetNumber() == engine) {
			found = true;
			break;
		}
	}
	if (!found)return nullptr;
	return g_engines[i];
}
extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt) {
	if (g_currentEngine == nullptr)		return false;
	speech_engine_update();
	if (!g_delayOperation)
		return g_currentEngine->Speak(text, interrupt);
	else {
		QueuedOutput qout;
		qout.text = text;
		qout.interrupt = interrupt;
		qout.braille = false;
		qout.speak = true;
		qout.ssml = false;
		qout.engine = g_currentEngine;
		qout.time = g_lastDelayTime;
		g_delayedOutputs.push_back(qout);
		if (!g_outputThreadRunning) {
			std::thread t(output_thread);
			t.detach();
		}
		return true;
	}
	return false;
}
extern "C" SRAL_API void* SRAL_SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
	if (g_currentEngine == nullptr)		return nullptr;
	speech_engine_update();
	return g_currentEngine->SpeakToMemory(text, buffer_size, channels, sample_rate, bits_per_sample);
}
extern "C" SRAL_API bool SRAL_SpeakSsml(const char* ssml, bool interrupt) {
	if (g_currentEngine == nullptr)		return false;
	speech_engine_update();
	if (!g_delayOperation)
		return g_currentEngine->SpeakSsml(ssml, interrupt);
	else {
		QueuedOutput qout;
		qout.text = ssml;
		qout.interrupt = interrupt;
		qout.braille = false;
		qout.speak = true;
		qout.ssml = true;
		qout.engine = g_currentEngine;
		qout.time = g_lastDelayTime;
		g_delayedOutputs.push_back(qout);
		if (!g_outputThreadRunning) {
			std::thread t(output_thread);
			t.detach();
		}
		return true;
	}
	return false;
}

extern "C" SRAL_API bool SRAL_Braille(const char* text) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return g_currentEngine->Braille(text);
}
extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	const bool speech = SRAL_Speak(text, interrupt);
	const bool braille = SRAL_Braille(text);
	return speech || braille;
}
extern "C" SRAL_API bool SRAL_StopSpeech(void) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	if (g_delayOperation) {
		g_delayedOutputs.clear();
		g_delayOperation = false;
	}
	return g_currentEngine->StopSpeech();
}
extern "C" SRAL_API bool SRAL_PauseSpeech(void) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	if (g_delayOperation) {
		// Just stop the thread. Don't clear the queue
		g_delayOperation = false;
	}
	return g_currentEngine->PauseSpeech();
}
extern "C" SRAL_API bool SRAL_ResumeSpeech(void) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	if (g_delayedOutputs.size() != 0) {
		g_delayOperation = true;
		if (!g_outputThreadRunning) {
			std::thread t(output_thread);
			t.detach();
		}
	}
	return g_currentEngine->ResumeSpeech();
}

extern "C" SRAL_API int SRAL_GetCurrentEngine(void) {
	if (g_currentEngine == nullptr)return ENGINE_NONE;
	return g_currentEngine->GetNumber();
}
extern "C" SRAL_API int SRAL_GetEngineFeatures(int engine) {
	if (engine == 0) {
		if (g_currentEngine == nullptr)return -1;
		return g_currentEngine->GetFeatures();
	}
	else {
		Engine* e = get_engine(engine);
		if (e == nullptr)return -1;
		return e->GetFeatures();
	}
	return -1;
}
extern "C" SRAL_API bool SRAL_SetVolume(uint64_t value) {
	if (g_currentEngine == nullptr)return false;
	g_currentEngine->SetVolume(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetVolume(void) {
	if (g_currentEngine == nullptr)return false;
	return g_currentEngine->GetVolume();
}
extern "C" SRAL_API bool SRAL_SetRate(uint64_t value) {
	if (g_currentEngine == nullptr)return false;
	g_currentEngine->SetRate(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetRate(void) {
	if (g_currentEngine == nullptr)return false;
	return g_currentEngine->GetRate();
}

extern "C" SRAL_API uint64_t SRAL_GetVoiceCount(void) {
	if (g_currentEngine == nullptr)return false;
	return g_currentEngine->GetVoiceCount();
}
extern "C" SRAL_API const char* SRAL_GetVoiceName(uint64_t index) {
	if (g_currentEngine == nullptr)return nullptr;
	return g_currentEngine->GetVoiceName(index);
}
extern "C" SRAL_API bool SRAL_SetVoice(uint64_t index) {
	if (g_currentEngine == nullptr)return false;
	return g_currentEngine->SetVoice(index);
}

extern "C" SRAL_API bool SRAL_SetVolumeEx(int engine, uint64_t value) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	e->SetVolume(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetVolumeEx(int engine) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->GetVolume();
}
extern "C" SRAL_API bool SRAL_SetRateEx(int engine, uint64_t value) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	e->SetRate(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetRateEx(int engine) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->GetRate();
}

extern "C" SRAL_API uint64_t SRAL_GetVoiceCountEx(int engine) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->GetVoiceCount();
}
extern "C" SRAL_API const char* SRAL_GetVoiceNameEx(int engine, uint64_t index) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return nullptr;
	return e->GetVoiceName(index);
}
extern "C" SRAL_API bool SRAL_SetVoiceEx(int engine, uint64_t index) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->SetVoice(index);
}



extern "C" SRAL_API bool SRAL_SetEngineParameter(int engine, int param, int value) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->SetParameter(param, value);
}



extern "C" SRAL_API bool SRAL_SpeakEx(int engine, const char* text, bool interrupt) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (!g_delayOperation)
		return e->Speak(text, interrupt);
	else {
		QueuedOutput qout;
		qout.text = text;
		qout.interrupt = interrupt;
		qout.braille = false;
		qout.speak = true;
		qout.ssml = false;
		qout.engine = e;
		qout.time = g_lastDelayTime;
		g_delayedOutputs.push_back(qout);
		if (!g_outputThreadRunning) {
			std::thread t(output_thread);
			t.detach();
		}
		return true;
	}
	return false;
}
extern "C" SRAL_API void* SRAL_SpeakToMemoryEx(int engine, const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return nullptr;
	return e->SpeakToMemory(text, buffer_size, channels, sample_rate, bits_per_sample);
}

extern "C" SRAL_API bool SRAL_SpeakSsmlEx(int engine, const char* ssml, bool interrupt) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (!g_delayOperation)
		return e->SpeakSsml(ssml, interrupt);
	else {
		QueuedOutput qout;
		qout.text = ssml;
		qout.interrupt = interrupt;
		qout.braille = false;
		qout.speak = true;
		qout.ssml = true;
		qout.engine = e;
		qout.time = g_lastDelayTime;
		g_delayedOutputs.push_back(qout);
		if (!g_outputThreadRunning) {
			std::thread t(output_thread);
			t.detach();
		}
		return true;
	}
	return false;
}

extern "C" SRAL_API bool SRAL_BrailleEx(int engine, const char* text) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->Braille(text);
}
extern "C" SRAL_API bool SRAL_OutputEx(int engine, const char* text, bool interrupt) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	const bool speech = e->Speak(text, interrupt);
	const bool braille = e->Braille(text);
	return speech || braille;
}
extern "C" SRAL_API bool SRAL_StopSpeechEx(int engine) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (g_delayOperation) {
		g_delayedOutputs.clear();
		g_delayOperation = false;
	}
	return e->StopSpeech();
}


extern "C" SRAL_API bool SRAL_PauseSpeechEx(int engine) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (g_delayOperation) {
		g_delayOperation = false;
	}
	return e->PauseSpeech();
}



extern "C" SRAL_API bool SRAL_ResumeSpeechEx(int engine) {
	Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (g_delayedOutputs.size() != 0) {
		g_delayOperation = true;
		if (!g_outputThreadRunning) {
			std::thread t(output_thread);
			t.detach();
		}
	}
	return e->ResumeSpeech();
}


extern "C" SRAL_API bool SRAL_IsInitialized(void) {
	return g_initialized;
}



extern "C" SRAL_API void SRAL_Delay(int time) {
	g_lastDelayTime = time;
	g_delayOperation = true;
}

extern "C" SRAL_API int SRAL_GetAvailableEngines(void) {
	if (g_engines.empty())return 0;
	int mask = 0;
	for (Engine* e : g_engines) {
		if (e)
			mask |= e->GetNumber();
	}
	return mask;
}

extern "C" SRAL_API int SRAL_GetActiveEngines(void) {
	if (g_engines.empty())return 0;
	int mask = 0;
	for (Engine* e : g_engines) {
		if (e)
			mask |= e->GetActive();
	}
	return mask;
}
