#ifndef JAWS_H_
#define JAWS_H_
#pragma once
#include "../Dep/fsapi.h"
#include "../Include/SRAL.h"
#include "Engine.h"
#include <windows.h>

namespace Sral {
	class Jaws final : public Engine {
	public:
		bool Speak(const char* text, bool interrupt)override;

		bool Braille(const char* text)override;
		bool StopSpeech()override;
		int GetNumber()override {
			return SRAL_ENGINE_JAWS;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH | SRAL_SUPPORTS_BRAILLE;
		}


	private:
		IJawsApi* pJawsApi = nullptr;
	};
}
#endif