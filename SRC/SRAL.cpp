#define SRAL_EXPORT
#include "../Include/SRAL.h"
#include "Engine.h"
#ifdef _WIN32
#include "NVDA.h"
#include "SAPI.h"
#include "Jaws.h"
#include "UIA.h"
#else
#include "SpeechDispatcher.h"
#endif
#include <vector>
#include <string>


Engine* g_currentEngine = nullptr;
std::vector<Engine*> g_engines;
int g_excludes = 0;
bool g_initialized = false;


extern "C" SRAL_API bool SRAL_Initialize(int engines_exclude) {
	if (g_initialized)return true;
#ifdef _WIN32
	g_engines.push_back(new NVDA);
	g_engines.push_back(new SAPI);
	g_engines.push_back(new Jaws);
	g_engines.push_back(new UIA);
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
static void speech_engine_update() {
	if (!g_currentEngine->GetActive() || g_currentEngine->GetNumber() == ENGINE_SAPI) {
		for (uint64_t i = 0; i < g_engines.size(); ++i) {
			if (g_engines[i]->GetActive() && !(g_excludes & g_engines[i]->GetNumber())) {
				g_currentEngine = g_engines[i];
				break;
			}
		}
	}
}
static Engine* get_engine(int engine) {
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
	return g_currentEngine->Speak(text, interrupt);
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
	return g_currentEngine->StopSpeech();
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
	return e->Speak(text, interrupt);
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
	return e->StopSpeech();
}



extern "C" SRAL_API bool SRAL_IsInitialized(void) {
	return g_initialized;
}
