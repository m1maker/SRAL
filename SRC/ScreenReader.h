#ifndef SCREENREADER_H_
#define SCREENREADER_H_
#pragma once
#include <stdint.h>
class ScreenReader {
public:
	virtual bool Speak(const char* text, bool interrupt) const = 0;
	virtual bool Braille(const char* text) const = 0;
	virtual int GetNumber() const = 0;
	virtual void SetVolume(uint64_t value) const = 0;
	virtual uint64_t GetVolume() const = 0;
	virtual void SetRate(uint64_t value) const = 0;
	virtual uint64_t GetRate() const = 0;
private:
	virtual bool Initialize() const = 0;
	virtual bool Uninitialize()const = 0;
};
#endif