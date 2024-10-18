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
	bool SpeakSsml(const char* ssml, bool interrupt)override;
	bool SetParameter(int param, int value)override;
	bool Braille(const char* text)override;
	bool StopSpeech()override;
	bool PauseSpeech()override;
	bool ResumeSpeech()override;
	int GetNumber()override {
		return ENGINE_NVDA;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH | SUPPORTS_BRAILLE | SUPPORTS_PAUSE_SPEECH;
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

	int GetKeyFlags()override {
		return HANDLE_NONE;
	}

private:
	HINSTANCE lib = nullptr;
	typedef error_status_t(__stdcall* NVDAController_speakText)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_brailleMessage)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_cancelSpeech)();
	typedef error_status_t(__stdcall* NVDAController_testIfRunning)();
	typedef error_status_t(__stdcall* NVDAController_speakSsml)(const wchar_t*, int, int, int);

	NVDAController_speakText nvdaController_speakText = nullptr;
	NVDAController_brailleMessage nvdaController_brailleMessage = nullptr;
	NVDAController_cancelSpeech nvdaController_cancelSpeech = nullptr;
	NVDAController_testIfRunning nvdaController_testIfRunning = nullptr;
	NVDAController_speakSsml nvdaController_speakSsml = nullptr;
	int symbolLevel = -1;


};
#endif
#endif