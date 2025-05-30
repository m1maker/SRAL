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

		bool SetParameter(int param, const void* value)override;

		bool GetParameter(int param, void* value) override;

		bool StopSpeech()override;
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

	private:
		AVSpeechSynthesizerWrapper* obj = nullptr;

	};
}
