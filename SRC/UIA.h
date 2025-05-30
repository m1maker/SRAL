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

		bool StopSpeech()override;
		int GetNumber()override {
			return SRAL_ENGINE_UIA;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH;
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