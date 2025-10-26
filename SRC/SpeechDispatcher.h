#ifndef SPEECHDISPATCHER_H_
#define SPEECHDISPATCHER_H_
#include "../Include/SRAL.h"
#include "Engine.h"
#include <speech-dispatcher/libspeechd.h>

namespace Sral {
	class SpeechDispatcher final : public Engine {
	public:
		bool Speak(const char* text, bool interrupt)override;
		bool SpeakSsml(const char* ssml, bool interrupt)override;

		bool Braille(const char* text)override;

		bool SetParameter(int param, const void* value)override;
		bool GetParameter(int param, void* value) override;


		bool StopSpeech()override;
		bool PauseSpeech()override;
		bool ResumeSpeech()override;
		int GetNumber()override {
			return SRAL_ENGINE_SPEECH_DISPATCHER;
		}
		bool GetActive()override;
		bool Initialize()override;
		bool Uninitialize()override;
		int GetFeatures()override {
			return SRAL_SUPPORTS_SPEECH | SRAL_SUPPORTS_BRAILLE | SRAL_SUPPORTS_SPEECH_RATE | SRAL_SUPPORTS_SPEECH_VOLUME | SRAL_SUPPORTS_PAUSE_SPEECH | SRAL_SUPPORTS_SPELLING | SRAL_SUPPORTS_SSML;
		}
		int GetKeyFlags()override {
			return HANDLE_INTERRUPT | HANDLE_PAUSE_RESUME;
		}

	private:
		SPDConnection* speech = nullptr;
		bool enableSpelling = false;
		bool brailleInitialized = false;
	};
}
#endif
