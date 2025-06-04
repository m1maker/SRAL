#include "Encoding.h"
#include "ZDSR.h"
#include <windows.h>

namespace Sral {
	bool Zdsr::Initialize() {
		lib = LoadLibraryW(L"ZDSRAPI.dll");
		if (lib == nullptr) {
			return false;
		}

		fInitTTS = (InitTTS_t)GetProcAddress(lib, "InitTTS");
		fSpeak = (Speak_t)GetProcAddress(lib, "Speak");
		fStopSpeak = (StopSpeak_t)GetProcAddress(lib, "StopSpeak");
		fGetSpeakState = (GetSpeakState_t)GetProcAddress(lib, "GetSpeakState");
		if (!fInitTTS || !fSpeak || !fStopSpeak || !fGetSpeakState) {
			this->Uninitialize();
			return false;
		}

		return fInitTTS(0, nullptr) == 0 ? true : false;
	}

	bool Zdsr::Uninitialize() {
		if (lib == nullptr)return true;
		FreeLibrary(lib);
		fInitTTS = nullptr;
		fSpeak = nullptr;
		fStopSpeak = nullptr;
		fGetSpeakState = nullptr;
		return true;
	}

	bool Zdsr::GetActive() {
		if (lib == nullptr || !fGetSpeakState) return false;
		int r = fGetSpeakState();
		return r == 3 || r == 4;
	}

	bool Zdsr::Speak(const char* text, bool interrupt) {
		if (!GetActive())return false;
		std::wstring out;
		UnicodeConvert(text, out);
		return fSpeak(out.c_str(), interrupt) == 0;
	}

	bool Zdsr::StopSpeech() {
		if (!GetActive())return false;
		fStopSpeak();
		return true;
	}

	bool Zdsr::IsSpeaking() {
		if (!GetActive())return false;
		return fGetSpeakState() == 3;
	}
}
