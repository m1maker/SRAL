#define SRAL_EXPORT
#include "../Include/SRAL.h"
#include "Engine.h"
#if defined(_WIN32)
#define UNICODE
#include "NVDA.h"
#include "SAPI.h"
#include "Jaws.h"
#include "UIA.h"
#include <Windows.h>
#include <tlhelp32.h>
#elif defined(__APPLE__)
#include "AVSpeech.h"
#else
#include "SpeechDispatcher.h"
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

	uint64_t elapsed() {
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
	}

	void restart() {
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
			if (g_delayedOutputs[i].speak)
				g_delayedOutputs[i].engine->Speak(g_delayedOutputs[i].text, g_delayedOutputs[i].interrupt);
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

extern "C" SRAL_API bool SRAL_Initialize(int engines_exclude) {
	if (g_initialized)return true;
#if defined(_WIN32)
	g_engines.push_back(new NVDA);
	g_engines.push_back(new SAPI);
	g_engines.push_back(new Jaws);
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
	g_currentEngine->Uninitialize();
	delete g_currentEngine;
	g_currentEngine = nullptr;
	g_engines.clear();
	g_excludes = 0;
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
	if (!g_currentEngine->GetActive() || g_currentEngine->GetNumber() == ENGINE_SAPI) {
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
