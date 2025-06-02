#include "../Dep/nvda_control.h"
#include "Encoding.h"
#include "NVDA.h"
#include <windows.h>

namespace Sral {
	bool Nvda::Initialize() {
		int r = nvda_connect();
		if (r == 0) {
			return true;
		}
		lib = LoadLibraryW(L"nvdaControllerClient.dll");
		if (lib == nullptr) {
			return false;
		}
		nvdaController_speakText = (NVDAController_speakText)GetProcAddress(lib, "nvdaController_speakText");
		nvdaController_brailleMessage = (NVDAController_brailleMessage)GetProcAddress(lib, "nvdaController_brailleMessage");
		nvdaController_cancelSpeech = (NVDAController_cancelSpeech)GetProcAddress(lib, "nvdaController_cancelSpeech");
		nvdaController_testIfRunning = (NVDAController_testIfRunning)GetProcAddress(lib, "nvdaController_testIfRunning");
		nvdaController_speakSsml = (NVDAController_speakSsml)GetProcAddress(lib, "nvdaController_speakSsml");
		return true;
	}

	bool Nvda::Uninitialize() {
		nvda_disconnect();
		if (lib == nullptr)return true;
		FreeLibrary(lib);
		nvdaController_speakText = nullptr;
		nvdaController_brailleMessage = nullptr;
		nvdaController_cancelSpeech = nullptr;
		nvdaController_testIfRunning = nullptr;
		nvdaController_speakSsml = nullptr;

		return true;
	}

	bool Nvda::GetActive() {
		if (nvda_active() == 0) {
			this->extended = true;
			return true;
		}
		else {
			// Try to use the library
			this->extended = false;
			this->Uninitialize();
			this->Initialize();
		}
		if (lib == nullptr) return false;
		if (nvdaController_testIfRunning) return  (!!FindWindowW(L"wxWindowClassNR", L"NVDA") && nvdaController_testIfRunning() == 0);
		return false;
	}

	bool Nvda::Speak(const char* text, bool interrupt) {
		if (!GetActive())return false;
		if (interrupt) {
			this->extended ? nvda_cancel_speech() : nvdaController_cancelSpeech();
		}
		if (this->extended)
			return !enable_spelling ? nvda_speak(text, this->symbolLevel) == 0 : nvda_speak_spelling(text, "", this->use_character_descriptions) == 0;
		std::wstring out;
		if (this->symbolLevel == -1) {
			UnicodeConvert(text, out);
			return nvdaController_speakText(out.c_str()) == 0;
		}
		std::string text_str(text);
		XmlEncode(text_str);
		std::string final = "<speak>" + text_str + "</speak>";
		UnicodeConvert(final, out);
		error_status_t result = nvdaController_speakSsml(out.c_str(), this->symbolLevel, 0, true);
		if (result == 1717) {
			UnicodeConvert(text, out);
			return nvdaController_speakText(out.c_str()) == 0;
		}
		else {
			return result == 0;
		}
		return false;
	}

	bool Nvda::SpeakSsml(const char* ssml, bool interrupt) {
		if (!GetActive())return false;
		if (interrupt)
			this->extended ? nvda_cancel_speech() : nvdaController_cancelSpeech();
		if (this->extended)
			return nvda_speak_ssml(ssml, this->symbolLevel) == 0;
		std::string text_str(ssml);
		std::wstring out;
		UnicodeConvert(ssml, out);
		return nvdaController_speakSsml(out.c_str(), this->symbolLevel, 0, true) == 0;
	}

	bool Nvda::SetParameter(int param, const void* value) {
		switch (param) {
		case SRAL_PARAM_SYMBOL_LEVEL:
			this->symbolLevel = *reinterpret_cast<const int*>(value);
			break;
		case SRAL_PARAM_ENABLE_SPELLING:
			this->enable_spelling = *reinterpret_cast<const bool*>(value);
			//break;
		case SRAL_PARAM_USE_CHARACTER_DESCRIPTIONS:
			this->use_character_descriptions = *reinterpret_cast<const bool*>(value);
			break;
		default:
			return false;
		}
		return true;
	}

	bool Nvda::GetParameter(int param, void* value) {
		switch (param) {
		case SRAL_PARAM_SYMBOL_LEVEL:
			*(int*)value = this->symbolLevel;
			return true;
		case SRAL_PARAM_ENABLE_SPELLING:
			*(bool*)value = this->enable_spelling;
			return true;
		case SRAL_PARAM_USE_CHARACTER_DESCRIPTIONS:
			*(bool*)value = this->use_character_descriptions;
			return true;
		case SRAL_PARAM_NVDA_IS_CONTROL_EX:
			this->extended = nvda_active() == 0;
			*(bool*)value = this->extended;
			return true;
		default:
			return false;
		}
		return false;
	}

	bool Nvda::Braille(const char* text) {
		if (!GetActive())return false;
		if (this->extended)
			return nvda_braille(text) == 0;
		std::wstring out;
		UnicodeConvert(text, out);
		return nvdaController_brailleMessage(out.c_str()) == 0;
	}
	bool Nvda::StopSpeech() {
		if (!GetActive())return false;
		return 		this->extended ? nvda_cancel_speech() == 0 : nvdaController_cancelSpeech() == 0;
	}
	bool Nvda::PauseSpeech() {
		if (!GetActive())return false;
		if (this->extended)
			return nvda_pause_speech(true);
		INPUT input[2] = {};

		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = VK_SHIFT;
		input[0].ki.dwFlags = 0;

		input[1].type = INPUT_KEYBOARD;
		input[1].ki.wVk = VK_SHIFT;
		input[1].ki.dwFlags = KEYEVENTF_KEYUP;

		SendInput(2, input, sizeof(INPUT));

		return true;
	}
	bool Nvda::ResumeSpeech() {
		return 		this->extended ? nvda_pause_speech(false) == 0 : PauseSpeech();
	}
}
