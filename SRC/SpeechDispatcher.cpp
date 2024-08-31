#define spd_get_default_address (*spd_get_default_address)
#define spd_open2 (*spd_open2)
#define spd_close (*spd_close)
#define spd_say (*spd_say)
#define spd_stop (*spd_stop)
#define spd_cancel (*spd_cancel)
#define spd_set_voice_rate (*spd_set_voice_rate)
#define spd_get_voice_rate (*spd_get_voice_rate)
#define spd_set_volume (*spd_set_volume)
#define spd_get_volume (*spd_get_volume)
#define spd_pause_all (*spd_pause_all)
#define spd_resume_all (*spd_resume_all)

#include "SpeechDispatcher.h"
#undef spd_get_default_address
#undef spd_open2
#undef spd_close
#undef spd_say
#undef spd_stop
#undef spd_cancel
#undef spd_set_voice_rate
#undef spd_get_voice_rate
#undef spd_set_volume
#undef spd_get_volume
#undef spd_pause_all
#undef spd_resume_all


#include <dlfcn.h> // For dlopen, dlsym, dlclose
bool SpeechDispatcher::Initialize() {
	Lib = dlopen("libspeechd.so", RTLD_LAZY);
	if (Lib == nullptr) {
		return false;
	}

	*(void**)&spd_get_default_address = dlsym(Lib, "spd_get_default_address");
	*(void**)&spd_open2 = dlsym(Lib, "spd_open2");
	*(void**)&spd_close = dlsym(Lib, "spd_close");
	*(void**)&spd_say = dlsym(Lib, "spd_say");
	*(void**)&spd_stop = dlsym(Lib, "spd_stop");
	*(void**)&spd_cancel = dlsym(Lib, "spd_cancel");
	*(void**)&spd_set_voice_rate = dlsym(Lib, "spd_set_voice_rate");
	*(void**)&spd_get_voice_rate = dlsym(Lib, "spd_get_voice_rate");
	*(void**)&spd_set_volume = dlsym(Lib, "spd_set_volume");
	*(void**)&spd_get_volume = dlsym(Lib, "spd_get_volume");
	*(void**)&spd_pause_all = dlsym(Lib, "spd_pause_all");
	*(void**)&spd_resume_all = dlsym(Lib, "spd_resume_all");
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
	if (Lib == nullptr || Speech == nullptr)return false;
	spd_close(Speech);
	Speech = nullptr;
	dlclose(Lib);
	Lib = nullptr;
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
	return spd_say(Speech, interrupt ? SPD_IMPORTANT : SPD_TEXT, text);
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
