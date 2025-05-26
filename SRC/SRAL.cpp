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
#endif
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <memory>

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



static const std::map<SRAL_Engines, std::string> g_engineNames = {
	{ SRAL_ENGINE_NONE, "None" },
	{ SRAL_ENGINE_NVDA, "NVDA" },
	{ SRAL_ENGINE_SAPI, "SAPI" },
	{ SRAL_ENGINE_JAWS, "JAWS" },
	{ SRAL_ENGINE_SPEECH_DISPATCHER, "Speech Dispatcher" },
	{ SRAL_ENGINE_UIA, "UIA" },
	{ SRAL_ENGINE_AV_SPEECH, "AV Speech" },
	{ SRAL_ENGINE_NARRATOR, "Narrator" }
};


static Sral::Engine* g_currentEngine = nullptr;
static std::map<SRAL_Engines, std::unique_ptr<Sral::Engine>> g_engines;
static int g_excludes = 0;
static bool g_initialized = false;

struct QueuedOutput {
	const char* text;
	bool interrupt;
	bool braille;
	bool speak;
	bool ssml;
	int time;
	Sral::Engine* engine;
};

static std::vector<QueuedOutput> g_delayedOutputs;
static bool g_delayOperation = false;
static bool g_outputThreadRunning = false;

static std::thread g_outputThread;

static uint64_t g_lastDelayTime = 0;


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



static bool g_keyboardHookThread = false;
static bool g_shiftPressed = false;

#if defined(_WIN32)
static HHOOK g_keyboardHook;
static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;
		for (const auto& [value, ptr] : g_engines) {
			if (ptr == nullptr || !ptr->GetActive()) continue;

			if (wParam == WM_KEYDOWN) {
				if ((pKeyInfo->vkCode == VK_LCONTROL || pKeyInfo->vkCode == VK_RCONTROL) && ptr->GetKeyFlags() & Sral::HANDLE_INTERRUPT) {
					ptr->StopSpeech();
				}
				else if ((pKeyInfo->vkCode == VK_LSHIFT || pKeyInfo->vkCode == VK_RSHIFT) && ptr->GetKeyFlags() & Sral::HANDLE_PAUSE_RESUME && g_shiftPressed == false) {
					if (ptr->paused)
						ptr->ResumeSpeech();
					else
						ptr->PauseSpeech();
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

static std::thread g_hookThread;

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
	g_hookThread = std::thread(hook_thread);
	g_hookThread.detach();
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
	if (g_hookThread.joinable()) {
		g_hookThread.join();
	}
}
#else
extern "C" SRAL_API bool SRAL_RegisterKeyboardHooks(void) {
	return false;
}
extern "C" SRAL_API void SRAL_UnregisterKeyboardHooks(void) {
	return;
}
#endif

extern "C" SRAL_API bool SRAL_Initialize(int engines_exclude) {
	if (g_initialized)return true;
#if defined(_WIN32)
	g_engines[SRAL_ENGINE_NVDA] = std::make_unique<Sral::Nvda>();
	g_engines[SRAL_ENGINE_JAWS] = std::make_unique<Sral::Jaws>();
	g_engines[SRAL_ENGINE_SAPI] = std::make_unique<Sral::Sapi>();
	g_engines[SRAL_ENGINE_UIA] = std::make_unique<Sral::Uia>();
#elif defined(__APPLE__)
	g_engines[SRAL_ENGINE_AV_SPEECH] = std::make_unique<Sral::AvSpeech>();
#else
	g_engines[SRAL_ENGINE_SPEECH_DISPATCHER] = std::make_unique<Sral::SpeechDispatcher>();
#endif
	bool found = false;
	for (const auto& [value, ptr] : g_engines) {
		ptr->Initialize();
		if (ptr->GetActive() && !found && !(engines_exclude & value)) {
			g_currentEngine = ptr.get();
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
	for (const auto& [value, ptr] : g_engines) {
		ptr->Uninitialize();
	}
	g_currentEngine = nullptr;
	g_engines.clear();
	g_excludes = 0;
	if (g_outputThread.joinable()) {
		g_outputThread.join();
	}
	if (g_keyboardHookThread) {
		SRAL_UnregisterKeyboardHooks();
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
static void speech_engine_update() {
	if (!g_currentEngine->GetActive() || g_currentEngine->GetNumber() == SRAL_ENGINE_SAPI || g_currentEngine->GetNumber() == SRAL_ENGINE_UIA) {
#ifdef _WIN32
		if (FindProcess(L"narrator.exe") == TRUE) {
			g_currentEngine = g_engines[SRAL_ENGINE_UIA].get();
			return;
		}
		else {
#endif
			for (const auto& [value, ptr] : g_engines) {
				if (ptr->GetActive() && !(g_excludes & value)) {
					g_currentEngine = ptr.get();
					break;
				}
			}
		}
#ifdef _WIN32
	}
#endif
}

extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt) {
	if (g_currentEngine == nullptr)		return false;
	speech_engine_update();
	return SRAL_SpeakEx(g_currentEngine->GetNumber(), text, interrupt);
}

extern "C" SRAL_API void* SRAL_SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
	if (g_currentEngine == nullptr)		return nullptr;
	speech_engine_update();
	return SRAL_SpeakToMemoryEx(g_currentEngine->GetNumber(), text, buffer_size, channels, sample_rate, bits_per_sample);
}
extern "C" SRAL_API bool SRAL_SpeakSsml(const char* ssml, bool interrupt) {
	if (g_currentEngine == nullptr)		return false;
	speech_engine_update();
	return SRAL_SpeakSsmlEx(g_currentEngine->GetNumber(), ssml, interrupt);
}

extern "C" SRAL_API bool SRAL_Braille(const char* text) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return SRAL_BrailleEx(g_currentEngine->GetNumber(), text);
}
extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return SRAL_OutputEx(g_currentEngine->GetNumber(), text, interrupt);
}

extern "C" SRAL_API bool SRAL_StopSpeech(void) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return SRAL_StopSpeechEx(g_currentEngine->GetNumber());
}
extern "C" SRAL_API bool SRAL_PauseSpeech(void) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return SRAL_PauseSpeechEx(g_currentEngine->GetNumber());
}
extern "C" SRAL_API bool SRAL_ResumeSpeech(void) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return SRAL_ResumeSpeechEx(g_currentEngine->GetNumber());
}

extern "C" SRAL_API int SRAL_GetCurrentEngine(void) {
	if (g_currentEngine == nullptr)return SRAL_ENGINE_NONE;
	return g_currentEngine->GetNumber();
}
extern "C" SRAL_API int SRAL_GetEngineFeatures(int engine) {
	if (engine == 0) {
		if (g_currentEngine == nullptr)return -1;
		return g_currentEngine->GetFeatures();
	}
	else {
		Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
		if (e == nullptr)return -1;
		return e->GetFeatures();
	}
	return -1;
}



extern "C" SRAL_API bool SRAL_SetEngineParameter(int engine, int param, const void* value) {
	if (engine == 0 && g_currentEngine != nullptr) {
		return g_currentEngine->SetParameter(param, value);
	}
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return false;
	return e->SetParameter(param, value);
}


extern "C" SRAL_API bool SRAL_GetEngineParameter(int engine, int param, void* value) {
	if (engine == 0 && g_currentEngine != nullptr) {
		return g_currentEngine->GetParameter(param, value);
	}
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return false;
	return e->GetParameter(param, value);
}



extern "C" SRAL_API bool SRAL_SpeakEx(int engine, const char* text, bool interrupt) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
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
			g_outputThread = std::thread(output_thread);
			g_outputThread.detach();
		}
		return true;
	}
	return false;
}
extern "C" SRAL_API void* SRAL_SpeakToMemoryEx(int engine, const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return nullptr;
	return e->SpeakToMemory(text, buffer_size, channels, sample_rate, bits_per_sample);
}

extern "C" SRAL_API bool SRAL_SpeakSsmlEx(int engine, const char* ssml, bool interrupt) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
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
			g_outputThread = std::thread(output_thread);
			g_outputThread.detach();
		}
		return true;
	}
	return false;
}

extern "C" SRAL_API bool SRAL_BrailleEx(int engine, const char* text) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return false;
	return e->Braille(text);
}
extern "C" SRAL_API bool SRAL_OutputEx(int engine, const char* text, bool interrupt) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return false;
	const bool speech = e->Speak(text, interrupt);
	const bool braille = e->Braille(text);
	return speech || braille;
}
extern "C" SRAL_API bool SRAL_StopSpeechEx(int engine) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return false;
	if (g_delayOperation) {
		g_delayedOutputs.clear();
		g_delayOperation = false;
		if (g_outputThread.joinable()) {
			g_outputThread.join();
		}
	}
	return e->StopSpeech();
}


extern "C" SRAL_API bool SRAL_PauseSpeechEx(int engine) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return false;
	if (g_delayOperation) {
		g_delayOperation = false;
		if (g_outputThread.joinable()) {
			g_outputThread.join();
		}
	}
	return e->PauseSpeech();
}



extern "C" SRAL_API bool SRAL_ResumeSpeechEx(int engine) {
	Sral::Engine* e = g_engines[static_cast<SRAL_Engines>(engine)].get();
	if (e == nullptr)return false;
	if (g_delayedOutputs.size() != 0) {
		g_delayOperation = true;
		if (!g_outputThreadRunning) {
			g_outputThread = std::thread(output_thread);
			g_outputThread.detach();
		}
	}
	return e->ResumeSpeech();
}


extern "C" SRAL_API bool SRAL_IsInitialized(void) {
	return g_initialized && g_currentEngine && !g_engines.empty();
}



extern "C" SRAL_API void SRAL_Delay(int time) {
	g_lastDelayTime = time;
	g_delayOperation = true;
}

extern "C" SRAL_API int SRAL_GetAvailableEngines(void) {
	if (g_engines.empty())return 0;
	int mask = 0;
	for (const auto& [value, ptr] : g_engines) {
		if (ptr)
			mask |= value;
	}
	return mask;
}

extern "C" SRAL_API int SRAL_GetActiveEngines(void) {
	if (g_engines.empty())return 0;
	int mask = 0;
	for (const auto& [value, ptr] : g_engines) {
		if (ptr && ptr->GetActive())
			mask |= value;
	}
	return mask;
}


extern "C" SRAL_API const char* SRAL_GetEngineName(int engine) {
	auto it = g_engineNames.find(static_cast<SRAL_Engines>(engine));
	if (it != g_engineNames.end()) {
		return it->second.c_str();
	}
	else {
		return "";
	}
	return "";
}

