#ifndef SPEECHDISPATCHER_H_
#define SPEECHDISPATCHER_H_
#include "../Include/SRAL.h"
#include "Engine.h"
#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <speech-dispatcher/libspeechd.h>


class SpeechDispatcher : public Engine {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text) { return false; }
	bool StopSpeech();
	int GetNumber() {
		return ENGINE_SPEECH_DISPATCHER;
	}
	bool GetActive();
	bool Initialize();
	bool Uninitialize();
	int GetFeatures() {
		return SUPPORTS_SPEECH;
	}
	void SetVolume(uint64_t) { return; }
	uint64_t GetVolume() { return 0; }
	void SetRate(uint64_t) { return; }
	uint64_t GetRate() { return 0; }
private:
	SPDConnection* Speech = nullptr;
	void* Lib = nullptr;
};

#endif
#endif