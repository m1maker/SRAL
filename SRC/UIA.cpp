#include "Encoding.h"
#include "UIA.h"
#include <string>



bool UIA::Initialize() {
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		return false;
	}
	hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
	if (FAILED(hr)) {
		return false;
	}

	varName.vt = VT_BSTR;
	varName.bstrVal = SysAllocString(L"");
	hr = pAutomation->CreatePropertyConditionEx(UIA_NamePropertyId, varName, PropertyConditionFlags_None, &pCondition);
	if (FAILED(hr)) {
		SysFreeString(varName.bstrVal);
		return false;
	}
	return true;
}

bool UIA::Uninitialize() {
	pProvider->Release();

	SysFreeString(varName.bstrVal);

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
	HRESULT hr = pAutomation->ElementFromHandle(GetForegroundWindow(), &pElement);

	if (FAILED(hr)) {
		return false;
	}
	const BSTR string = SysAllocString(str.c_str());
	const BSTR stringUnused = SysAllocString(L"");
	hr = UiaRaiseNotificationEvent(pProvider, NotificationKind_ActionCompleted, flags, string, stringUnused);
	if (FAILED(hr)) {
		SysFreeString(string);
		SysFreeString(stringUnused);
		return false;
	}
	SysFreeString(string);
	SysFreeString(stringUnused);
	return true;

}
bool UIA::StopSpeech() {
	return Speak("", true);
}
bool UIA::GetActive() {
	return true;
}