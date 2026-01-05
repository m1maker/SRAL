#define SRAL_EXPORT
#include "../Include/SRAL.h"
#include "Engine.h"
#if defined(_WIN32)
#define UNICODE
#include "NVDA.h"
#include "ZDSR.h"
#include "SAPI.h"
#include "Jaws.h"
#ifndef SRAL_NO_UIA
#include "UIA.h"
#endif
#include <windows.h>
#include <tlhelp32.h>
#elif defined(__APPLE__)
#include "AVSpeech.h"
#include "VoiceOver.h"
#else
#include "SpeechDispatcher.h"
#endif
#include <map>
#include <mutex>
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




static Sral::Engine* g_currentEngine{nullptr};
static std::map<SRAL_Engines, std::unique_ptr<Sral::Engine>> g_engines;
static int g_excludes{SRAL_ENGINE_NONE};
static int g_enginesFailedToInitialize{SRAL_ENGINE_NONE};
static bool g_initialized{false};

struct QueuedOutput {
	std::string text;
	bool interrupt;
	bool braille;
	bool speak;
	bool ssml;
	int time;
	Sral::Engine* engine;
};

static std::vector<QueuedOutput> g_delayedOutputs;
static std::mutex g_delayedOutputsMutex;
static std::atomic<bool> g_delayOperation{false};
static std::atomic<bool> g_outputThreadRunning{false};

static std::thread g_outputThread;

static std::atomic<uint64_t> g_lastDelayTime{0};


static void output_thread() {
	g_outputThreadRunning.store(true);
	static Timer s_timer;
	s_timer.restart();
	while (g_delayOperation.load()) {
		QueuedOutput current_output;
		{
			std::unique_lock<std::mutex> lock(g_delayedOutputsMutex);

			if (!g_delayOperation.load() || g_delayedOutputs.empty()) {
				break;
			}

			current_output = g_delayedOutputs.front();
			g_delayedOutputs.erase(g_delayedOutputs.begin());
		}

		s_timer.restart();
		while (s_timer.elapsed() < current_output.time && g_delayOperation.load()) {
			if (current_output.engine->IsSpeaking()) {
				s_timer.restart();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		if (!g_delayOperation.load()) break;

		if (current_output.speak) {
			if (current_output.ssml)
				current_output.engine->SpeakSsml(current_output.text.c_str(), current_output.interrupt);
			else
				current_output.engine->Speak(current_output.text.c_str(), current_output.interrupt);

		}
		else if (current_output.braille)
			current_output.engine->Braille(current_output.text.c_str());

	}
	g_delayOperation = false;
	g_lastDelayTime = 0;
	g_outputThreadRunning.store(false);
}



static std::atomic<bool> g_keyboardHookThread{false};
static std::atomic<bool> g_shiftPressed{false};

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
				else if ((pKeyInfo->vkCode == VK_LSHIFT || pKeyInfo->vkCode == VK_RSHIFT) && ptr->GetKeyFlags() & Sral::HANDLE_PAUSE_RESUME && !g_shiftPressed.load()) {
					if (ptr->paused)
						ptr->ResumeSpeech();
					else
						ptr->PauseSpeech();
					g_shiftPressed.store(true);
				}
			}
			else if (wParam == WM_KEYUP) {
				if (pKeyInfo->vkCode == VK_LSHIFT || pKeyInfo->vkCode == VK_RSHIFT) {
					g_shiftPressed.store(false);
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

	while (g_keyboardHookThread.load()) {
		if (GetMessageW(&msg, nullptr, 0, 0)) {
			DispatchMessageW(&msg);
			TranslateMessage(&msg);
		}
	}
	UnhookWindowsHookEx(g_keyboardHook);
}

extern "C" SRAL_API bool SRAL_RegisterKeyboardHooks(void) {
	if (!SRAL_IsInitialized()) return false;
	if (g_keyboardHookThread.load()) return true;
	g_keyboardHookThread.store(true);
	g_hookThread = std::thread(hook_thread);
	g_hookThread.detach();
	static Timer s_timer;
	while (s_timer.elapsed() < 3000) {
		Sleep(5);
		if (g_keyboardHook != nullptr) {
			return true;
		}
	}
	return false; // Timeout: Hook is not set
}

extern "C" SRAL_API void SRAL_UnregisterKeyboardHooks(void) {
	if (!SRAL_IsInitialized()) return;
	PostMessage(0, WM_KEYUP, 0, 0);
	g_keyboardHookThread.store(false);
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



extern "C" SRAL_API void* SRAL_malloc(size_t size) {
	return malloc(size);
}

extern "C" void SRAL_free(void* memory) {
	free(memory);
}




extern "C" SRAL_API bool SRAL_Initialize(int engines_exclude) {
	if (g_initialized)return true;
#if defined(_WIN32)
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	g_engines[SRAL_ENGINE_NVDA] = std::make_unique<Sral::Nvda>();
	g_engines[SRAL_ENGINE_JAWS] = std::make_unique<Sral::Jaws>();
	g_engines[SRAL_ENGINE_ZDSR] = std::make_unique<Sral::Zdsr>();
#ifndef SRAL_NO_UIA
	g_engines[SRAL_ENGINE_UIA] = std::make_unique<Sral::Uia>();
#endif
	g_engines[SRAL_ENGINE_SAPI] = std::make_unique<Sral::Sapi>();
#elif defined(__APPLE__)
	g_engines[SRAL_ENGINE_VOICE_OVER] = std::make_unique<Sral::VoiceOver>();
	g_engines[SRAL_ENGINE_AV_SPEECH] = std::make_unique<Sral::AvSpeech>();
#else
	g_engines[SRAL_ENGINE_SPEECH_DISPATCHER] = std::make_unique<Sral::SpeechDispatcher>();
#endif
	// Here we need to check that at least one engine has been initialized.
	// Otherwise, if none of them are running, there is no point in returning true.
	bool success = false;
	for (const auto& [value, ptr] : g_engines) {
		if (!ptr->Initialize()) {
			g_enginesFailedToInitialize |= ptr->GetNumber();
		}
		else {
			success = true;
		}
	}

	g_initialized = success;
	if (!g_initialized) return false;
	SRAL_SetEnginesExclude(engines_exclude);
	return g_initialized;
}

extern "C" SRAL_API void SRAL_Uninitialize(void) {
	if (!SRAL_IsInitialized())return;
	for (const auto& [value, ptr] : g_engines) {
		ptr->Uninitialize();
	}
#ifdef _WIN32
	CoUninitialize();
#endif
	g_currentEngine = nullptr;
	g_engines.clear();
	g_excludes = SRAL_ENGINE_NONE;
	g_enginesFailedToInitialize = SRAL_ENGINE_NONE;
	if (g_outputThread.joinable()) {
		g_outputThread.join();
	}
	if (g_keyboardHookThread.load()) {
		SRAL_UnregisterKeyboardHooks();
	}
	g_initialized = false;
}

static Sral::Engine* get_engine(int engine) {
	auto it = g_engines.find(static_cast<SRAL_Engines>(engine));
	if (it != g_engines.end()) {
		return it->second.get();
	}
	else {
		return nullptr;
	}
	return nullptr;
}




#ifdef _WIN32
// This is used for find the Windows Narrator process
static BOOL FindProcess(const wchar_t* name) {
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
	if (!g_currentEngine || !g_currentEngine->GetActive() || g_currentEngine->GetNumber() == SRAL_ENGINE_SAPI || g_currentEngine->GetNumber() == SRAL_ENGINE_UIA || g_currentEngine->GetNumber() == SRAL_ENGINE_AV_SPEECH) {
#if defined(_WIN32) && !defined(SRAL_NO_UIA)
		if (FindProcess(L"narrator.exe") == TRUE) {
			g_currentEngine = get_engine(SRAL_ENGINE_UIA);
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
#if defined(_WIN32) && !defined(SRAL_NO_UIA)
		}
#endif
	}
}

extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt) {
	speech_engine_update();
	if (g_currentEngine == nullptr)		return false;
	return SRAL_SpeakEx(g_currentEngine->GetNumber(), text, interrupt);
}

extern "C" SRAL_API void* SRAL_SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
	speech_engine_update();
	if (g_currentEngine == nullptr)		return nullptr;
	return SRAL_SpeakToMemoryEx(g_currentEngine->GetNumber(), text, buffer_size, channels, sample_rate, bits_per_sample);
}

extern "C" SRAL_API bool SRAL_SpeakSsml(const char* ssml, bool interrupt) {
	speech_engine_update();
	if (g_currentEngine == nullptr)		return false;
	return SRAL_SpeakSsmlEx(g_currentEngine->GetNumber(), ssml, interrupt);
}

extern "C" SRAL_API bool SRAL_Braille(const char* text) {
	speech_engine_update();
	if (g_currentEngine == nullptr)return false;
	return SRAL_BrailleEx(g_currentEngine->GetNumber(), text);
}

extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt) {
	speech_engine_update();
	if (g_currentEngine == nullptr)return false;
	return SRAL_OutputEx(g_currentEngine->GetNumber(), text, interrupt);
}

extern "C" SRAL_API bool SRAL_StopSpeech(void) {
	speech_engine_update();
	if (g_currentEngine == nullptr)return false;
	return SRAL_StopSpeechEx(g_currentEngine->GetNumber());
}

extern "C" SRAL_API bool SRAL_PauseSpeech(void) {
	speech_engine_update();
	if (g_currentEngine == nullptr)return false;
	return SRAL_PauseSpeechEx(g_currentEngine->GetNumber());
}

extern "C" SRAL_API bool SRAL_ResumeSpeech(void) {
	speech_engine_update();
	if (g_currentEngine == nullptr)return false;
	return SRAL_ResumeSpeechEx(g_currentEngine->GetNumber());
}

extern "C" SRAL_API bool SRAL_IsSpeaking(void) {
	speech_engine_update();
	if (g_currentEngine == nullptr)		return false;
	return SRAL_IsSpeakingEx(g_currentEngine->GetNumber());
}

extern "C" SRAL_API int SRAL_GetCurrentEngine(void) {
	speech_engine_update();
	if (g_currentEngine == nullptr)return SRAL_ENGINE_NONE;
	return g_currentEngine->GetNumber();
}

extern "C" SRAL_API int SRAL_GetEngineFeatures(int engine) {
	if (engine == 0) {
		if (g_currentEngine == nullptr)return -1;
		return g_currentEngine->GetFeatures();
	}
	else {
		Sral::Engine* e = get_engine(engine);
		if (e == nullptr)return -1;
		return e->GetFeatures();
	}
	return -1;
}



extern "C" SRAL_API bool SRAL_SetEngineParameter(int engine, int param, const void* value) {
	if (engine == 0 && g_currentEngine != nullptr) {
		return g_currentEngine->SetParameter(param, value);
	}
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->SetParameter(param, value);
}


extern "C" SRAL_API bool SRAL_GetEngineParameter(int engine, int param, void* value) {
	if (engine == 0 && g_currentEngine != nullptr) {
		return g_currentEngine->GetParameter(param, value);
	}
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->GetParameter(param, value);
}



extern "C" SRAL_API bool SRAL_SpeakEx(int engine, const char* text, bool interrupt) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (!g_delayOperation.load())
		return e->Speak(text, interrupt);
	else {
		QueuedOutput qout;
		qout.text = std::string(text);
		qout.interrupt = interrupt;
		qout.braille = false;
		qout.speak = true;
		qout.ssml = false;
		qout.engine = e;
		qout.time = g_lastDelayTime;
		{
			std::unique_lock<std::mutex> lock(g_delayedOutputsMutex);
			g_delayedOutputs.push_back(qout);
		}
		if (!g_outputThreadRunning) {
			g_outputThread = std::thread(output_thread);
			g_outputThread.detach();
		}
		return true;
	}
	return false;
}

extern "C" SRAL_API void* SRAL_SpeakToMemoryEx(int engine, const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return nullptr;
	return e->SpeakToMemory(text, buffer_size, channels, sample_rate, bits_per_sample);
}

extern "C" SRAL_API bool SRAL_SpeakSsmlEx(int engine, const char* ssml, bool interrupt) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (!g_delayOperation.load())
		return e->SpeakSsml(ssml, interrupt);
	else {
		QueuedOutput qout;
		qout.text = std::string(ssml);
		qout.interrupt = interrupt;
		qout.braille = false;
		qout.speak = true;
		qout.ssml = true;
		qout.engine = e;
		qout.time = g_lastDelayTime;
		{
			std::unique_lock<std::mutex> lock(g_delayedOutputsMutex);
			g_delayedOutputs.push_back(qout);
		}
		if (!g_outputThreadRunning) {
			g_outputThread = std::thread(output_thread);
			g_outputThread.detach();
		}
		return true;
	}
	return false;
}

extern "C" SRAL_API bool SRAL_BrailleEx(int engine, const char* text) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->Braille(text);
}

extern "C" SRAL_API bool SRAL_OutputEx(int engine, const char* text, bool interrupt) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	const bool speech = e->Speak(text, interrupt);
	const bool braille = e->Braille(text);
	return speech || braille;
}

extern "C" SRAL_API bool SRAL_StopSpeechEx(int engine) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (g_delayOperation.load()) {
		{
			std::unique_lock<std::mutex> lock(g_delayedOutputsMutex);
			g_delayedOutputs.clear();
		}
		g_delayOperation.store(false);
		if (g_outputThread.joinable()) {
			g_outputThread.join();
		}
	}
	return e->StopSpeech();
}


extern "C" SRAL_API bool SRAL_PauseSpeechEx(int engine) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	if (g_delayOperation.load()) {
		g_delayOperation.store(false);
		if (g_outputThread.joinable()) {
			g_outputThread.join();
		}
	}
	return e->PauseSpeech();
}



extern "C" SRAL_API bool SRAL_ResumeSpeechEx(int engine) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	{
		std::unique_lock<std::mutex> lock(g_delayedOutputsMutex);
		if (!g_delayedOutputs.empty()) {
			g_delayOperation.store(true);
			if (!g_outputThreadRunning) {
				g_outputThread = std::thread(output_thread);
				g_outputThread.detach();
			}
		}
	}
	return e->ResumeSpeech();
}


extern "C" SRAL_API bool SRAL_IsSpeakingEx(int engine) {
	Sral::Engine* e = get_engine(engine);
	if (e == nullptr)return false;
	return e->IsSpeaking();
}

extern "C" SRAL_API bool SRAL_IsInitialized(void) {
	return g_initialized && !g_engines.empty();
}



extern "C" SRAL_API void SRAL_Delay(int time) {
	if (!SRAL_IsInitialized()) return;
	g_lastDelayTime = time;
	g_delayOperation.store(true);
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
	switch (static_cast<SRAL_Engines>(engine)) {
		case SRAL_ENGINE_NONE: return "None";
		case SRAL_ENGINE_NVDA: return "NVDA";
		case SRAL_ENGINE_SAPI: return "SAPI";
		case SRAL_ENGINE_JAWS: return "JAWS";
		case SRAL_ENGINE_SPEECH_DISPATCHER: return "Speech Dispatcher";
		case SRAL_ENGINE_UIA: return "UIA";
		case SRAL_ENGINE_AV_SPEECH: return "AV Speech";
		case SRAL_ENGINE_NARRATOR: return "Narrator";
		case SRAL_ENGINE_VOICE_OVER: return "Voice Over";
		case SRAL_ENGINE_ZDSR: return "ZDSR";
		default: return "Unknown";
	}
}

extern "C" SRAL_API bool SRAL_SetEnginesExclude(int engines_exclude) {
	if (!SRAL_IsInitialized()) return false;
	g_excludes = engines_exclude;
	speech_engine_update();
	return true;
}

extern "C" SRAL_API int SRAL_GetEnginesExclude(void) {
	return SRAL_IsInitialized() ? g_excludes : -1;
}
