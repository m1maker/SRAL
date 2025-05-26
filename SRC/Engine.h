#ifndef SCREENREADER_H_
#define SCREENREADER_H_
#pragma once
#include <stdint.h>

namespace Sral {

	enum KeyboardFlags {
		HANDLE_NONE = 0,
		HANDLE_INTERRUPT = 2,
		HANDLE_PAUSE_RESUME = 4
	};

	class Engine {
	public:
		virtual bool Speak(const char* text, bool interrupt) = 0;
		virtual bool SpeakSsml(const char* ssml, bool interrupt) = 0;
		virtual void* SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample) = 0;
		virtual bool Braille(const char* text) = 0;
		virtual bool StopSpeech() = 0;
		virtual bool PauseSpeech() = 0;
		virtual bool ResumeSpeech() = 0;
		virtual int GetNumber() = 0;
		virtual bool GetActive() = 0;
		virtual int GetFeatures() = 0;
		virtual bool Initialize() = 0;
		virtual bool Uninitialize() = 0;
		virtual int GetKeyFlags() = 0;
		virtual bool SetParameter(int param, const void* value) = 0;
		virtual bool GetParameter(int param, void* value) = 0;

		bool paused;
	};
}
#endif