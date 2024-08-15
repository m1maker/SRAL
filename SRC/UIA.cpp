#include "../Dep/UIAProvider.h"
#include "Encoding.h"
#include "UIA.h"
#include <string>
static void WINAPI SendNotification(const std::wstring& message, NotificationProcessing flags) {
	CoInitialize(NULL);

	IUIAutomation* pAutomation = NULL;
	HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);

	if (SUCCEEDED(hr)) {
		IUIAutomationCondition* pCondition = NULL;
		VARIANT varName;
		varName.vt = VT_BSTR;
		varName.bstrVal = SysAllocString(L"");
		hr = pAutomation->CreatePropertyConditionEx(UIA_NamePropertyId, varName, PropertyConditionFlags_None, &pCondition);

		if (SUCCEEDED(hr)) {
			Provider* pProvider = new Provider(GetForegroundWindow());
			IUIAutomationElement* pElement = NULL;
			hr = pAutomation->ElementFromHandle(GetForegroundWindow(), &pElement);

			if (SUCCEEDED(hr)) {
				hr = UiaRaiseNotificationEvent(pProvider, NotificationKind_ActionCompleted, flags, SysAllocString(message.c_str()), SysAllocString(L""));

				if (SUCCEEDED(hr)) {
					pProvider->Release();
				}
			}

			pCondition->Release();
		}

		pAutomation->Release();
	}

	CoUninitialize();
}
bool UIA::Initialize() { return true; }
bool UIA::Uninitialize() { return true; }
bool UIA::Speak(const char* text, bool interrupt) {
	NotificationProcessing flags = NotificationProcessing_All;
	if (interrupt)
		flags = NotificationProcessing_ImportantAll;
	std::wstring str;
	UnicodeConvert(text, str);
	SendNotification(str, flags);
	return true;

}
bool UIA::StopSpeech() {
	return Speak("", true);
}
bool UIA::GetActive() {
	return true;
}