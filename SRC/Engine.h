#ifndef ENGINE_H_
#define ENGINE_H_
#pragma once
#include <stdint.h>
#include <vector>
#include <string.h>

namespace Sral {

	enum KeyboardFlags {
		HANDLE_NONE = 0,
		HANDLE_INTERRUPT = 2,
		HANDLE_PAUSE_RESUME = 4
	};

	class Engine {
	public:
		Engine();
		virtual ~Engine();
		virtual bool Speak(const char* text, bool interrupt);
		virtual bool SpeakSsml(const char* ssml, bool interrupt);
		virtual void* SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample);
		virtual bool Braille(const char* text);
		virtual bool StopSpeech();
		virtual bool PauseSpeech();
		virtual bool ResumeSpeech();
		virtual bool IsSpeaking();
		virtual int GetNumber();
		virtual bool GetActive();
		virtual int GetFeatures();
		virtual bool Initialize();
		virtual bool Uninitialize();
		virtual int GetKeyFlags();
		virtual bool SetParameter(int param, const void* value);
		virtual bool GetParameter(int param, void* value);

		bool paused;
	protected:
		std::vector<char*> m_strings;

		inline const char* AddString(const char* str) {
			if (!str) return nullptr;

			size_t len = strlen(str) + 1;
			char* cString = new char[len];
			strcpy(cString, str);
			m_strings.push_back(cString);
			return cString;
		}

		inline void ReleaseAllStrings() {
			for (auto str : m_strings) {
				delete[] str;
			}
			m_strings.clear();
		}
	};
}
#endif
