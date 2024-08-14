#ifndef SRAL_H_
#define SRAL_H_
#pragma once
#ifdef _WIN32
#if defined SRAL_EXPORT
#define SRAL_API __declspec(dllexport)
#else
#define SRAL_API __declspec(dllimport)
#endif
#else
#define SRAL_API
#endif
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
enum SRAL_ScreenReaders {
	SCREEN_READER_NONE = 0,
	SCREEN_READER_NVDA,
	SCREEN_READER_SAPI,
	SCREEN_READER_JAWS
};
enum SRAL_SupportedFeatures {
	SUPPORTS_SPEECH = 0x41e2,
	SUPPORTS_BRAILLE = 0x41e3,
	SUPPORTS_SPEECH_RATE = 0x1e4c2,
	SUPPORTS_SPEECH_VOLUME = 0x1e4c3
};
extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt);
extern "C" SRAL_API bool SRAL_Braille(const char* text);
extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt);
extern "C" SRAL_API bool SRAL_StopSpeech(void);
extern "C" SRAL_API int SRAL_GetCurrentScreenReader(void);
extern "C" SRAL_API int SRAL_GetEngineFeatures(void);
extern "C" SRAL_API bool SRAL_Initialize(const char* library_path, int engines_exclude = 0);
extern "C" SRAL_API void SRAL_Uninitialize(void);
extern "C" SRAL_API bool SRAL_SetVolume(uint64_t value);
extern "C" SRAL_API uint64_t SRAL_GetVolume(void);
extern "C" SRAL_API bool SRAL_SetRate(uint64_t value);
extern "C" SRAL_API uint64_t SRAL_GetRate(void);
#endif // SRAL_H_