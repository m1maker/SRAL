#ifndef ZDSR_H_
#define ZDSR_H_
#pragma once
#include "../Include/SRAL.h"
#include "Engine.h"
#include <windows.h>

namespace Sral {
	class Zdsr final : public Engine {
	public:
		bool Speak(const char* text, bool interrupt)override;
		bool StopSpeech()override;
		bool IsSpeaking() override;
		int GetNumber()override {
			return SRAL_ENGINE_ZDSR;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH;
		}


	private:
		HINSTANCE lib = nullptr;
		using InitTTS_t = int(__stdcall*) (int, wchar_t*);
		using Speak_t = int(__stdcall*) (const wchar_t*, BOOL);
		using GetSpeakState_t = int(__stdcall*) ();
		using StopSpeak_t = int(__stdcall*) ();

		InitTTS_t fInitTTS = nullptr;
		Speak_t fSpeak = nullptr;
		GetSpeakState_t fGetSpeakState = nullptr;
		StopSpeak_t fStopSpeak = nullptr;
	};
}
#endif