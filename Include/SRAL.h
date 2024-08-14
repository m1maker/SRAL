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
#include <stdlib.h>
extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt);
extern "C" SRAL_API bool SRAL_Braille(const char* text);
extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt);
extern "C" SRAL_API bool SRAL_StopSpeech(void);
extern "C" SRAL_API int SRAL_GetCurrentScreenReader(void);

#endif // SRAL_H_