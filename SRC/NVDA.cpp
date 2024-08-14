#include "Encoding.h"
#include "NVDA.h"
bool NVDA::Initialize() {
	lib = LoadLibraryW(L"nvdaControllerClient.dll");
	if (lib == nullptr)return false;
	nvdaController_speakText = (NVDAController_speakText)GetProcAddress(lib, "nvdaController_speakText");
	nvdaController_brailleMessage = (NVDAController_brailleMessage)GetProcAddress(lib, "nvdaController_brailleMessage");
	nvdaController_cancelSpeech = (NVDAController_cancelSpeech)GetProcAddress(lib, "nvdaController_cancelSpeech");
	nvdaController_testIfRunning = (NVDAController_testIfRunning)GetProcAddress(lib, "nvdaController_testIfRunning");
	return true;
}
bool NVDA::Uninitialize() {
	if (lib == nullptr)return false;
	FreeLibrary(lib);
	nvdaController_speakText = nullptr;
	nvdaController_brailleMessage = nullptr;
	nvdaController_cancelSpeech = nullptr;
	nvdaController_testIfRunning = nullptr;
	return true;
}
bool NVDA::GetActive() {
	if (lib == nullptr) return false;
	if (nvdaController_testIfRunning) return  (!!FindWindowW(L"wxWindowClassNR", L"NVDA") && nvdaController_testIfRunning() == 0);
	return false;
}
bool NVDA::Speak(const char* text, bool interrupt) {
	if (!GetActive())return false;
	if (interrupt)
		nvdaController_cancelSpeech();
	wchar_t* out = nullptr;
	UnicodeConvert(text, out);
	return nvdaController_speakText(out) == 0;
}
bool NVDA::Braille(const char* text) {
	if (!GetActive())return false;
	wchar_t* out = nullptr;
	UnicodeConvert(text, out);
	return nvdaController_speakText(out) == 0;
}
bool NVDA::StopSpeech() {
	if (!GetActive())return false;
	return nvdaController_cancelSpeech() == 0;
}