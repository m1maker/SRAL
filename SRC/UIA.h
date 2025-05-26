#ifndef UIA_H_
#define UIA_H_
#pragma once
#include "../Dep/UIAProvider.h"
#include "../Include/SRAL.h"
#include "Engine.h"

namespace Sral {
	class Uia final : public Engine {
	public:
		bool Speak(const char* text, bool interrupt)override;
		bool SpeakSsml(const char* ssml, bool interrupt)override {
			return false;
		}
		void* SpeakToMemory(const char* text, uint64_t* buffer_size, int* channels, int* sample_rate, int* bits_per_sample)override {
			return nullptr;
		}

		bool SetParameter(int param, const void* value)override {
			return false;
		}

		bool GetParameter(int param, void* value) override {
			return false;
		}

		bool Braille(const char* text)override { return false; }
		bool StopSpeech()override;
		bool PauseSpeech()override { return false; }
		bool ResumeSpeech()override { return false; }

		int GetNumber()override {
			return SRAL_ENGINE_UIA;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH;
		}

		int GetKeyFlags()override {
			return HANDLE_NONE;
		}

	private:
		IUIAutomation* pAutomation = nullptr;
		IUIAutomationCondition* pCondition = nullptr;
		VARIANT varName;
		Provider* pProvider = nullptr;
		IUIAutomationElement* pElement = nullptr;
	};
}
#endif