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
extern "C" SRAL_API bool SRAL_Initialize(const char* library_path) {
#ifdef _WIN32
	//	g_screenReaders.push_back(new NVDA);
	g_screenReaders.push_back(new SAPI);
#endif
	bool found = false;
	for (uint64_t i = 0; i < g_screenReaders.size(); ++i) {
		g_screenReaders[i]->Initialize();
		if (g_screenReaders[i]->GetActive()) {
			g_currentScreenReader = g_screenReaders[i];
			found = true;
			break;
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

extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt) {
	if (g_currentScreenReader == nullptr)
		return false;
	return g_currentScreenReader->Speak(text, interrupt);
}
extern "C" SRAL_API bool SRAL_Braille(const char* text) {
	if (g_currentScreenReader == nullptr)return false;
	return g_currentScreenReader->Braille(text);

}
extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt) {
	const bool speech = SRAL_Speak(text, interrupt);
	const bool braille = SRAL_Braille(text);
	return speech || braille;
}
extern "C" SRAL_API bool SRAL_StopSpeech(void) {
	if (g_currentScreenReader == nullptr)return false;
	return g_currentScreenReader->StopSpeech();
}
extern "C" SRAL_API int SRAL_GetCurrentScreenReader(void) {
	if (g_currentScreenReader == nullptr)return SCREEN_READER_NONE;
	return g_currentScreenReader->GetNumber();
}
