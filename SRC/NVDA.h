#ifndef NVDA_H_
#define NVDA_H_
#ifdef _WIN32
#pragma once
#include "../Include/SRAL.h"
#include "Engine.h"
#include <Windows.h>
class NVDA : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool Braille(const char* text)override;
	bool StopSpeech()override;
	int GetNumber()override {
		return ENGINE_NVDA;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH | SUPPORTS_BRAILLE;
	}
	void SetVolume(uint64_t)override { return; }
	uint64_t GetVolume()override { return 0; }
	void SetRate(uint64_t)override { return; }
	uint64_t GetRate()override { return 0; }
	uint64_t GetVoiceCount()override {
		return 0;
	}
	const char* GetVoiceName(uint64_t index)override {
		return nullptr;
	}
	bool SetVoice(uint64_t index)override {
		return false;
	}

private:
	HINSTANCE lib = nullptr;
	typedef error_status_t(__stdcall* NVDAController_speakText)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_brailleMessage)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_cancelSpeech)();
	typedef error_status_t(__stdcall* NVDAController_testIfRunning)();

	NVDAController_speakText nvdaController_speakText = nullptr;
	NVDAController_brailleMessage nvdaController_brailleMessage = nullptr;
	NVDAController_cancelSpeech nvdaController_cancelSpeech = nullptr;
	NVDAController_testIfRunning nvdaController_testIfRunning = nullptr;
};
#endif
#endif