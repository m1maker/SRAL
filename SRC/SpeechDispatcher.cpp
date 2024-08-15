#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include "SpeechDispatcher.h"
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
	auto* address = spd_get_default_address(nullptr);
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
}
bool SpeechDispatcher::Speak(const char* text, bool interrupt) {
	if (Speech == nullptr)return false;
	if (interrupt) {
		spd_stop(Speech);
		spd_cancel(Speech);
	}
	return spd_say(Speech, interrupt ? SPD_IMPORTANT : SPD_TEXT, text);
}
bool SpeechDispatcher::StopSpeech() {
	if (Speech == nullptr)return false;
	spd_stop(Speech);
	spd_cancel(Speech);
	return true;
}
#endif