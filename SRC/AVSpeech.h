/*
Thanks Gruia for implementing AVSpeech.
*/
#pragma once
#include "../Include/SRAL.h"
#include "Engine.h"
#include <string>
class AVSpeechSynthesizerWrapper;

namespace Sral {

	class AvSpeech final : public Engine {
	public:
		bool Speak(const char* text, bool interrupt)override;
		bool SpeakSsml(const char* ssml, bool interrupt)override {
			return false;
		}
		void* SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample)override {
			return nullptr;
		}

		bool SetParameter(int param, const void* value)override;

		bool GetParameter(int param, void* value) override;

		bool Braille(const char* text)override { return false; }
		bool StopSpeech()override;
		bool PauseSpeech()override { return false; }
		bool ResumeSpeech()override { return false; }
		bool IsSpeaking() override;
		int GetNumber()override {
			return SRAL_ENGINE_AV_SPEECH;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH | SRAL_SUPPORTS_SPEECH_RATE | SRAL_SUPPORTS_SPEECH_VOLUME | SRAL_SUPPORTS_SELECT_VOICE;
		}
		int GetKeyFlags()override {
			return HANDLE_NONE;
		}

	private:
		AVSpeechSynthesizerWrapper* obj = nullptr;

	};
}
