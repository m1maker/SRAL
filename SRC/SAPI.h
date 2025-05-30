#ifndef SAPI_H_
#define SAPI_H_
#pragma once
#define BLASTSPEAK_IMPLEMENTATION
#include "../Dep/blastspeak.h"
#include "../Dep/wasapi.h"
#include "../Include/SRAL.h"
#include "Engine.h"
#include <memory>
#include <thread>

namespace Sral {
	class Sapi final : public Engine {
	public:
		bool Speak(const char* text, bool interrupt)override;
		void* SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample)override;
		bool SetParameter(int param, const void* value)override;
		bool GetParameter(int param, void* value) override;

		bool StopSpeech()override;
		bool PauseSpeech()override;
		bool ResumeSpeech()override;
		bool IsSpeaking() override;
		int GetNumber()override {
			return SRAL_ENGINE_SAPI;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH | SRAL_SUPPORTS_SPEECH_RATE | SRAL_SUPPORTS_SPEECH_VOLUME | SRAL_SUPPORTS_SELECT_VOICE | SRAL_SUPPORTS_PAUSE_SPEECH | SRAL_SUPPORTS_SPEAK_TO_MEMORY;
		}
		int GetKeyFlags()override {
			return HANDLE_INTERRUPT | HANDLE_PAUSE_RESUME;
		}

	private:
		std::unique_ptr<blastspeak> instance;
		WAVEFORMATEX wfx;
		int trimThreshold = 20;
		int voiceIndex = 0;
		std::thread speechThread;
	};
}
#endif