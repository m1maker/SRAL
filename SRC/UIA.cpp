#include "Encoding.h"
#include "UIA.h"
#include<comdef.h>
#include <string>


bool UIA::Initialize() {
	HRESULT hr = CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
	if (FAILED(hr)) {
		return false;
	}

	varName.vt = VT_BSTR;
	varName.bstrVal = _bstr_t(L"");
	hr = pAutomation->CreatePropertyConditionEx(UIA_NamePropertyId, varName, PropertyConditionFlags_None, &pCondition);
	if (FAILED(hr)) {
		return false;
	}
	return true;
}

bool UIA::Uninitialize() {
	pProvider->Release();

	pCondition->Release();

	pAutomation->Release();

	return true;
}
bool UIA::Speak(const char* text, bool interrupt) {
	NotificationProcessing flags = NotificationProcessing_ImportantAll;
	if (interrupt)
		flags = NotificationProcessing_ImportantMostRecent;
	std::wstring str;
	UnicodeConvert(text, str);
	pProvider = new Provider(GetForegroundWindow());
	printf("Test");

	HRESULT hr = pAutomation->ElementFromHandle(GetForegroundWindow(), &pElement);

	if (FAILED(hr)) {
		return false;
	}
	hr = UiaRaiseNotificationEvent(pProvider, NotificationKind_ActionCompleted, flags, _bstr_t(str.c_str()), _bstr_t(L""));
	if (FAILED(hr)) {
		return false;
	}

	return true;

}
bool UIA::StopSpeech() {
	return Speak("", true);
}
bool UIA::GetActive() {
	return true;
}