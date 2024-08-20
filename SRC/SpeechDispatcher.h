#ifndef SPEECHDISPATCHER_H_
#define SPEECHDISPATCHER_H_
#include "../Include/SRAL.h"
#include "Engine.h"
#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <speech-dispatcher/libspeechd.h>


class SpeechDispatcher : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool Braille(const char* text)override { return false; }
	bool StopSpeech()override;
	int GetNumber()override {
		return ENGINE_SPEECH_DISPATCHER;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH;
	}
	void SetVolume(uint64_t)override { return; }
	uint64_t GetVolume()override { return 0; }
	void SetRate(uint64_t)override { return; }
	uint64_t GetRate()override { return 0; }
private:
	SPDConnection* Speech = nullptr;
	void* Lib = nullptr;
};

#endif
#endif