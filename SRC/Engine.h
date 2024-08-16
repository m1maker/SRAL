#ifndef SCREENREADER_H_
#define SCREENREADER_H_
#pragma once
#include <stdint.h>
class Engine {
public:
	virtual bool Speak(const char* text, bool interrupt) = 0;

	virtual bool Braille(const char* text) = 0;
	virtual bool StopSpeech() = 0;
	virtual int GetNumber() = 0;
	virtual bool GetActive() = 0;
	virtual int GetFeatures() = 0;
	virtual bool Initialize() = 0;
	virtual bool Uninitialize() = 0;
	virtual void SetVolume(uint64_t value) = 0;
	virtual uint64_t GetVolume() = 0;
	virtual void SetRate(uint64_t value) = 0;
	virtual uint64_t GetRate() = 0;
};
#endif