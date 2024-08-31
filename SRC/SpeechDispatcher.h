#ifndef SPEECHDISPATCHER_H_
#define SPEECHDISPATCHER_H_
#include "../Include/SRAL.h"
#include "Engine.h"
#include <speech-dispatcher/libspeechd.h>


class SpeechDispatcher : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool Braille(const char* text)override { return false; }
	bool StopSpeech()override;
	bool PauseSpeech()override;
	bool ResumeSpeech()override;

	int GetNumber()override {
		return ENGINE_SPEECH_DISPATCHER;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH | SUPPORTS_SPEECH_RATE | SUPPORTS_SPEECH_VOLUME | SUPPORTS_PAUSE_SPEECH;
	}
	void SetVolume(uint64_t)override;
	uint64_t GetVolume()override;
	void SetRate(uint64_t)override;
	uint64_t GetRate()override;
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
		return HANDLE_INTERRUPT | HANDLE_PAUSE_RESUME;
	}

private:
	SPDConnection* Speech = nullptr;
	void* Lib = nullptr;
};

#endif