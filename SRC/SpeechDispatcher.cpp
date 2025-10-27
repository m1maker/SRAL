// Actually, it should only be SpeechDispatcher, but since we currently don't support anything else on Linux, we'll integrate BRLTTY here.
#include "../Dep/utf-8.h"
#include "SpeechDispatcher.h"
#include <brlapi.h>
#include "Encoding.h"
#include <atomic>
#include <stdio.h>

std::atomic<bool> g_isSpeaking{false};

namespace Sral {
	bool SpeechDispatcher::Initialize() {
		const auto* address = spd_get_default_address(nullptr);
		if (address == nullptr) {
			return false;
		}
		speech = spd_open2("SRAL", nullptr, nullptr, SPD_MODE_THREADED, address, true, nullptr);
		if (speech == nullptr) {
			return false;
		}

		spd_set_data_mode(speech, SPD_DATA_SSML);

		speech->callback_begin = &SpeechDispatcher::SpeechNotificationCallback;
		speech->callback_end = &SpeechDispatcher::SpeechNotificationCallback;
		speech->callback_cancel = &SpeechDispatcher::SpeechNotificationCallback;
		spd_set_notification_on(speech, SPD_BEGIN);
		spd_set_notification_on(speech, SPD_END);
		spd_set_notification_on(speech, SPD_CANCEL);

		brailleInitialized = brlapi_openConnection(nullptr, nullptr) < 0 ? false : true;
		brlapi_enterTtyMode(BRLAPI_TTY_DEFAULT, nullptr);
		return true;
	}

	bool SpeechDispatcher::GetActive() {
		return speech != nullptr;
	}

	bool SpeechDispatcher::Uninitialize() {
		if (speech == nullptr)return false;
		g_isSpeaking.store(false);
		spd_close(speech);
		speech = nullptr;

		if (brailleInitialized) {
			brlapi_leaveTtyMode();
			brlapi_closeConnection();
			brailleInitialized = false;
		}
		return true;
	}

	bool SpeechDispatcher::Speak(const char* text, bool interrupt) {
		if (!enableSpelling) {
			std::string text_str(text);
			XmlEncode(text_str);
			std::string final = "<speak>" + text_str + "</speak>";
			return this->SpeakSsml(text_str.c_str(), interrupt);
		}
		else {
			if (interrupt) {
				spd_stop(speech);
				spd_cancel(speech);
			}

			utf8_iter iter;
			bool result = true;
			utf8_init(&iter, text);
			while (utf8_next(&iter) && result) {
				result = (spd_char(speech, SPD_IMPORTANT, utf8_getchar(&iter)) != -1);
			}
			return result;
		}
		return false;
	}

	bool SpeechDispatcher::SpeakSsml(const char* ssml, bool interrupt) {
		if (speech == nullptr)return false;
		if (interrupt) {
			spd_stop(speech);
			spd_cancel(speech);
		}
		if (this->paused) {
			this->ResumeSpeech();
			this->paused = false;

		}

		return spd_say(speech, SPD_IMPORTANT, ssml) != -1;
	}

	bool SpeechDispatcher::Braille(const char* text) {
		if (!brailleInitialized) return false;
		return brlapi_writeText(0, text);
	}

	bool SpeechDispatcher::IsSpeaking() {
		return g_isSpeaking.load();
	}

	bool SpeechDispatcher::SetParameter(int param, const void* value) {
		if (speech == nullptr)return false;
		switch (param) {
		case SRAL_PARAM_SYMBOL_LEVEL:
			spd_set_punctuation(speech, static_cast<SPDPunctuation>(*reinterpret_cast<const int*>(value)));
			break;
		case SRAL_PARAM_SPEECH_RATE:
			spd_set_voice_rate(speech, *reinterpret_cast<const int*>(value));
			break;
		case SRAL_PARAM_SPEECH_VOLUME:
			spd_set_volume(speech, *reinterpret_cast<const int*>(value));
			break;
		case SRAL_PARAM_ENABLE_SPELLING:
			this->enableSpelling = *reinterpret_cast<const bool*>(value);
			break;
		default:
			return false;
		}
		return true;
	}

	bool SpeechDispatcher::GetParameter(int param, void* value) {
		if (speech == nullptr)return false;
		switch (param) {
		case SRAL_PARAM_SPEECH_RATE:
			*(int*)value = spd_get_voice_rate(speech);
			return true;
		case SRAL_PARAM_SPEECH_VOLUME:
			*(int*)value = spd_get_volume(speech);
			return true;
		case SRAL_PARAM_ENABLE_SPELLING:
			*(bool*)value = this->enableSpelling;
			return true;
		default:
			return false;
		}
		return false;
	}

	bool SpeechDispatcher::StopSpeech() {
		if (speech == nullptr)return false;
		spd_stop(speech);
		spd_cancel(speech);
		return true;
	}

	bool SpeechDispatcher::PauseSpeech() {
		if (!GetActive())return false;
		this->paused = true;
		return spd_pause(speech) == 0;
	}

	bool SpeechDispatcher::ResumeSpeech() {
		if (!GetActive())return false;
		this->paused = false;
		return spd_resume(speech) == 0;
	}

	void SpeechDispatcher::SpeechNotificationCallback(size_t msg_id, size_t client_id, SPDNotificationType type) {
		switch (type) {
			case SPD_EVENT_BEGIN:
				g_isSpeaking.store(true);
				break;
			case SPD_EVENT_END:
			case SPD_EVENT_CANCEL:
				g_isSpeaking.store(false);
				break;
			default:
				return;
		}
	}
}

