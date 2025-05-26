#include "../Dep/utf-8.h"
#include "SpeechDispatcher.h"
namespace Sral {
	bool SpeechDispatcher::Initialize() {
		const auto* address = spd_get_default_address(nullptr);
		if (address == nullptr) {
			return false;
		}
		Speech = spd_open2("SRAL", nullptr, nullptr, SPD_MODE_THREADED, address, true, nullptr);
		if (Speech == nullptr) {
			return false;
		}

		return true;
	}
	bool SpeechDispatcher::GetActive() {
		return Speech != nullptr;
	}
	bool SpeechDispatcher::Uninitialize() {
		if (Speech == nullptr)return false;
		spd_close(Speech);
		Speech = nullptr;
		return true;
	}
	bool SpeechDispatcher::Speak(const char* text, bool interrupt) {
		if (Speech == nullptr)return false;
		if (interrupt) {
			spd_stop(Speech);
			spd_cancel(Speech);
		}
		if (this->paused) {
			this->ResumeSpeech();
			this->paused = false;

		}
		if (!enableSpelling) {
			return spd_say(Speech, SPD_IMPORTANT, text) != -1;
		}
		else {
			utf8_iter iter;
			bool result = true;
			utf8_init(&iter, text);
			while (utf8_next(&iter) && result) {
				result = (spd_char(Speech, SPD_IMPORTANT, utf8_getchar(&iter)) != -1);
			}
			return result;
		}
		return false;
	}


	bool SpeechDispatcher::SetParameter(int param, const void* value) {
		if (Speech == nullptr)return false;
		switch (param) {
		case SRAL_PARAM_SYMBOL_LEVEL:
			spd_set_punctuation(Speech, static_cast<SPDPunctuation>(*reinterpret_cast<const int*>(value)));
			break;
		case SRAL_PARAM_SPEECH_RATE:
			spd_set_voice_rate(Speech, *reinterpret_cast<const int*>(value));
			break;
		case SRAL_PARAM_SPEECH_VOLUME:
			spd_set_volume(Speech, *reinterpret_cast<const int*>(value));
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
		if (Speech == nullptr)return false;
		switch (param) {
		case SRAL_PARAM_SPEECH_RATE:
			*(int*)value = spd_get_voice_rate(Speech);
			return true;
		case SRAL_PARAM_SPEECH_VOLUME:
			*(int*)value = spd_get_volume(Speech);
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
		if (Speech == nullptr)return false;
		spd_stop(Speech);
		spd_cancel(Speech);
		return true;
	}
	bool SpeechDispatcher::PauseSpeech() {
		if (!GetActive())return false;
		this->paused = true;
		return spd_pause_all(Speech) == 0;
	}
	bool SpeechDispatcher::ResumeSpeech() {
		if (!GetActive())return false;
		this->paused = false;
		return spd_resume_all(Speech) == 0;
	}


}

