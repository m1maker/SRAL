#ifndef SPEECHDISPATCHER_H_
#define SPEECHDISPATCHER_H_
#include "../Include/SRAL.h"
#include "ScreenReader.h"
#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#define spd_get_default_address (*spd_get_default_address)
#define spd_open2 (*spd_open2)
#define spd_close (*spd_close)
#define spd_say (*spd_say)
#define spd_stop (*spd_stop)
#define spd_cancel (*spd_cancel)
#include <speech-dispatcher/libspeechd.h>
#undef spd_get_default_address
#undef spd_open2
#undef spd_close
#undef spd_say
#undef spd_stop
#undef spd_cancel


class SpeechDispatcher : public ScreenReader {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text) { return false; }
	bool StopSpeech();
	int GetNumber() {
		return SCREEN_READER_SPEECH_DISPATCHER;
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