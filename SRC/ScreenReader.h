#ifndef SCREENREADER_H_
#define SCREENREADER_H_
#pragma once
#include <stdint.h>
class ScreenReader {
public:
	virtual bool Speak(const char* text, bool interrupt);
	virtual bool Braille(const char* text);
	virtual bool StopSpeech();
	virtual int GetNumber();
	virtual bool GetActive();
	virtual bool Initialize();
	virtual bool Uninitialize();
};
#endif