#include "SpeechDispatcher.h"
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
	return spd_say(Speech, SPD_IMPORTANT, text);
}
bool SpeechDispatcher::SetParameter(int param, void* value) {
	switch (param) {
	case SYMBOL_LEVEL:
		spd_set_punctuation(Speech, (SPDPunctuation)*value);
		break;
	default:
		return false;
	}
	return true;
}

void* SpeechDispatcher::GetParameter(int param) {
	return nullptr;
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


void SpeechDispatcher::SetVolume(uint64_t value) {
	if (!GetActive())return;
	spd_set_volume(Speech, value);
}
uint64_t SpeechDispatcher::GetVolume() {
	if (!GetActive())return 0;
	return spd_get_volume(Speech);
}
void SpeechDispatcher::SetRate(uint64_t value) {
	if (!GetActive())return;
	spd_set_voice_rate(Speech, value);
}
uint64_t SpeechDispatcher::GetRate() {
	if (!GetActive())return 0;
	return spd_get_voice_rate(Speech);
}
