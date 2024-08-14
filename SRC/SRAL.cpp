#define SRAL_EXPORT
#include "../Include/SRAL.h"
#include "ScreenReader.h"
#ifdef _WIN32
#include "NVDA.h"
#include "SAPI.h"
#endif
#include <vector>
ScreenReader* g_currentScreenReader = nullptr;
std::vector<ScreenReader*> g_screenReaders;
extern "C" SRAL_API bool SRAL_Initialize(const char* library_path, int engines_exclude) {
#ifdef _WIN32
	g_screenReaders.push_back(new NVDA);
	g_screenReaders.push_back(new SAPI);
#endif
	bool found = false;
	for (uint64_t i = 0; i < g_screenReaders.size(); ++i) {
		g_screenReaders[i]->Initialize();
		if (g_screenReaders[i]->GetActive() && !found && !g_screenReaders[i]->GetNumber() & engines_exclude) {
			g_currentScreenReader = g_screenReaders[i];
			found = true;
		}
	}
	if (g_currentScreenReader == nullptr)return false;
	return found;
}
extern "C" SRAL_API void SRAL_Uninitialize(void) {
	g_currentScreenReader->Uninitialize();
	delete g_currentScreenReader;
	g_currentScreenReader = nullptr;
	g_screenReaders.clear();
}
static void speech_engine_update() {
	if (!g_currentScreenReader->GetActive() || g_currentScreenReader->GetNumber() == SCREEN_READER_SAPI) {
		for (uint64_t i = 0; i < g_screenReaders.size(); ++i) {
			if (g_screenReaders[i]->GetActive()) {
				g_currentScreenReader = g_screenReaders[i];
				break;
			}
		}
	}
}
extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt) {
	if (g_currentScreenReader == nullptr)		return false;
	speech_engine_update();
	return g_currentScreenReader->Speak(text, interrupt);
}
extern "C" SRAL_API bool SRAL_Braille(const char* text) {
	if (g_currentScreenReader == nullptr)return false;
	speech_engine_update();
	return g_currentScreenReader->Braille(text);

}
extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt) {
	if (g_currentScreenReader == nullptr)return false;
	speech_engine_update();
	const bool speech = SRAL_Speak(text, interrupt);
	const bool braille = SRAL_Braille(text);
	return speech || braille;
}
extern "C" SRAL_API bool SRAL_StopSpeech(void) {
	if (g_currentScreenReader == nullptr)return false;
	speech_engine_update();
	return g_currentScreenReader->StopSpeech();
}
extern "C" SRAL_API int SRAL_GetCurrentScreenReader(void) {
	if (g_currentScreenReader == nullptr)return SCREEN_READER_NONE;
	return g_currentScreenReader->GetNumber();
}
extern "C" SRAL_API int SRAL_GetEngineFeatures(void) {
	if (g_currentScreenReader == nullptr)return 0;
	return g_currentScreenReader->GetFeatures();
}
extern "C" SRAL_API bool SRAL_SetVolume(uint64_t value) {
	if (g_currentScreenReader == nullptr)return false;
	g_currentScreenReader->SetVolume(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetVolume(void) {
	if (g_currentScreenReader == nullptr)return false;
	return g_currentScreenReader->GetVolume();
}
extern "C" SRAL_API bool SRAL_SetRate(uint64_t value) {
	if (g_currentScreenReader == nullptr)return false;
	g_currentScreenReader->SetRate(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetRate(void) {
	if (g_currentScreenReader == nullptr)return false;
	return g_currentScreenReader->GetRate();
}
