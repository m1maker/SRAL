#ifndef SRAL_H_
#define SRAL_H_
#pragma once
#ifdef _WIN32
#if defined SRAL_EXPORT
#define SRAL_API __declspec(dllexport)
#elif defined (SRAL_STATIC)
#define SRAL_API
#else
#define SRAL_API __declspec(dllimport)
#endif
#else
#define SRAL_API
#endif
#ifdef __cplusplus
extern "C" {
#include <stdbool.h>
#endif
#include <stdint.h>
#include <stdlib.h>
	enum SRAL_Engines {
		ENGINE_NONE = 0,
		ENGINE_NVDA = 2,
		ENGINE_SAPI = 4,
		ENGINE_JAWS = 8,
		ENGINE_SPEECH_DISPATCHER = 16,
		ENGINE_UIA = 32
	};
	enum SRAL_SupportedFeatures {
		SUPPORTS_SPEECH = 128,
		SUPPORTS_BRAILLE = 256,
		SUPPORTS_SPEECH_RATE = 512,
		SUPPORTS_SPEECH_VOLUME = 1024,
		SUPPORTS_SELECT_VOICE = 2048

	};


	SRAL_API bool SRAL_Speak(const char* text, bool interrupt);
	SRAL_API bool SRAL_Braille(const char* text);
	SRAL_API bool SRAL_Output(const char* text, bool interrupt);
	SRAL_API bool SRAL_StopSpeech(void);
	SRAL_API int SRAL_GetCurrentEngine(void);
	SRAL_API int SRAL_GetEngineFeatures(int engine = 0);
	SRAL_API bool SRAL_Initialize(int engines_exclude = 0);
	SRAL_API void SRAL_Uninitialize(void);
	SRAL_API bool SRAL_SetVolume(uint64_t value);
	SRAL_API uint64_t SRAL_GetVolume(void);
	SRAL_API bool SRAL_SetRate(uint64_t value);
	SRAL_API uint64_t SRAL_GetRate(void);

	SRAL_API uint64_t SRAL_GetVoiceCount(void);
	SRAL_API const char* SRAL_GetVoiceName(uint64_t index);
	SRAL_API bool SRAL_SetVoice(uint64_t index);

	SRAL_API bool SRAL_SpeakEx(int engine, const char* text, bool interrupt);
	SRAL_API bool SRAL_BrailleEx(int engine, const char* text);
	SRAL_API bool SRAL_OutputEx(int engine, const char* text, bool interrupt);
	SRAL_API bool SRAL_StopSpeechEx(int engine);

	SRAL_API bool SRAL_SetVolumeEx(int engine, uint64_t value);
	SRAL_API uint64_t SRAL_GetVolumeEx(int engine);
	SRAL_API bool SRAL_SetRateEx(int engine, uint64_t value);
	SRAL_API uint64_t SRAL_GetRateEx(int engine);

	SRAL_API uint64_t SRAL_GetVoiceCountEx(int engine);
	SRAL_API const char* SRAL_GetVoiceNameEx(int engine, uint64_t index);
	SRAL_API bool SRAL_SetVoiceEx(int engine, uint64_t index);



	SRAL_API bool SRAL_IsInitialized(void);


#ifdef __cplusplus
}// extern "C"
#endif


#endif // SRAL_H_