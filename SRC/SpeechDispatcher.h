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

		bool IsSpeaking()override;

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
			return SRAL_SUPPORTS_SPEECH | SRAL_SUPPORTS_BRAILLE | SRAL_SUPPORTS_SPEECH_RATE | SRAL_SUPPORTS_SPEECH_VOLUME | SRAL_SUPPORTS_PAUSE_SPEECH | SRAL_SUPPORTS_SPELLING | SRAL_SUPPORTS_SSML | SRAL_SUPPORTS_SELECT_VOICE;
		}

		int GetKeyFlags()override {
			return HANDLE_NONE;
		}

	private:
		SPDConnection* speech = nullptr;
		bool enableSpelling = false;
		bool brailleInitialized = false;

		SPDVoice** m_voiceList{nullptr};
		int m_voiceCount{0};
		int m_voiceIndex{0};
		int SetVoiceIndex();
		inline void ClearVoiceList() {
			if (m_voiceList) {
				free_spd_voices(m_voiceList);
				m_voiceList = nullptr;
			}
		m_voiceCount = 0;
		}

		inline void RefreshVoiceList() {
			ClearVoiceList();
			if (speech == nullptr) return;
			m_voiceList = spd_list_synthesis_voices(speech);
			if (!m_voiceList) return;
			for (; m_voiceList[m_voiceCount] != nullptr; ++m_voiceCount);
		}

		static void SpeechNotificationCallback(size_t msg_id, size_t client_id, SPDNotificationType type);
	};
}
#endif
