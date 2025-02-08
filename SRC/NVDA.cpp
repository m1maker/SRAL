#include "Encoding.h"
#include "NVDA.h"
#include <windows.h>
#include "../Dep/nvda_control.h"

bool NVDA::Initialize() {
	pipe = nvda_connect();
	if (pipe != INVALID_HANDLE_VALUE)
		return true;
	lib = LoadLibraryW(L"nvdaControllerClient.dll");
	if (lib == nullptr)return false;
	nvdaController_speakText = (NVDAController_speakText)GetProcAddress(lib, "nvdaController_speakText");
	nvdaController_brailleMessage = (NVDAController_brailleMessage)GetProcAddress(lib, "nvdaController_brailleMessage");
	nvdaController_cancelSpeech = (NVDAController_cancelSpeech)GetProcAddress(lib, "nvdaController_cancelSpeech");
	nvdaController_testIfRunning = (NVDAController_testIfRunning)GetProcAddress(lib, "nvdaController_testIfRunning");
	nvdaController_speakSsml = (NVDAController_speakSsml)GetProcAddress(lib, "nvdaController_speakSsml");
	return true;
}
bool NVDA::Uninitialize() {
	if (pipe != INVALID_HANDLE_VALUE) {
		nvda_disconnect(pipe);
		return true;
	}
	if (lib == nullptr)return false;
	FreeLibrary(lib);
	nvdaController_speakText = nullptr;
	nvdaController_brailleMessage = nullptr;
	nvdaController_cancelSpeech = nullptr;
	nvdaController_testIfRunning = nullptr;
	nvdaController_speakSsml = nullptr;

	return true;
}
bool NVDA::GetActive() {
	if (pipe != INVALID_HANDLE_VALUE)
		return true;
	if (lib == nullptr) return false;
	if (nvdaController_testIfRunning) return  (!!FindWindowW(L"wxWindowClassNR", L"NVDA") && nvdaController_testIfRunning() == 0);
	return false;
}
bool NVDA::Speak(const char* text, bool interrupt) {
	if (!GetActive())return false;
	if (interrupt) {
		pipe == INVALID_HANDLE_VALUE ? nvdaController_cancelSpeech() : nvda_cancel_speech(pipe);
	}
	if (pipe != INVALID_HANDLE_VALUE)
		return nvda_speak(pipe, text, this->symbolLevel) == 0;
	std::string text_str(text);
	XmlEncode(text_str);
	std::string final = "<speak>" + text_str + "</speak>";
	std::wstring out_ssml;
	UnicodeConvert(final, out_ssml);
	error_status_t result = nvdaController_speakSsml(out_ssml.c_str(), this->symbolLevel, 0, true);
	if (result == 1717) {
		std::wstring out;
		UnicodeConvert(text, out);
		return nvdaController_speakText(out.c_str()) == 0;
	}
	else {
		return result == 0;
	}
	return false;
}
bool NVDA::SpeakSsml(const char* ssml, bool interrupt) {
	if (!GetActive())return false;
	if (interrupt)
		pipe == INVALID_HANDLE_VALUE ? nvdaController_cancelSpeech() : nvda_cancel_speech(pipe);
	if (pipe != INVALID_HANDLE_VALUE)
		return nvda_speak_ssml(pipe, ssml, this->symbolLevel) == 0;
	std::string text_str(ssml);
	std::wstring out;
	UnicodeConvert(ssml, out);
	return nvdaController_speakSsml(out.c_str(), this->symbolLevel, 0, true) == 0;
}

bool NVDA::SetParameter(int param, int value) {
	switch (param) {
	case SYMBOL_LEVEL:
		this->symbolLevel = value;
		break;
	default:
		return false;
	}
	return true;
}

bool NVDA::Braille(const char* text) {
	if (!GetActive())return false;
	std::wstring out;
	UnicodeConvert(text, out);
	return nvdaController_brailleMessage(out.c_str()) == 0;
}
bool NVDA::StopSpeech() {
	if (!GetActive())return false;
	return pipe == INVALID_HANDLE_VALUE ? nvdaController_cancelSpeech() == 0 : nvda_cancel_speech(pipe) == 0;
}
bool NVDA::PauseSpeech() {
	if (!GetActive())return false;
	if (pipe != INVALID_HANDLE_VALUE)
		return nvda_pause_speech(pipe, true);
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
bool NVDA::ResumeSpeech() {
	return pipe == INVALID_HANDLE_VALUE ? PauseSpeech() : nvda_pause_speech(pipe, false);
}
