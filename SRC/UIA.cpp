#include "Encoding.h"
#include "UIA.h"
#include<comdef.h>
#include <string>

namespace Sral {
	bool Uia::Initialize() {
		HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pAutomation);
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

	bool Uia::Uninitialize() {
		if (pProvider)pProvider->Release();

		if (pCondition)pCondition->Release();

		if (pAutomation)pAutomation->Release();
		if (pElement) pElement->Release();
		return true;
	}

	bool Uia::Speak(const char* text, bool interrupt) {
		NotificationProcessing flags = NotificationProcessing_ImportantAll;
		if (interrupt)
			flags = NotificationProcessing_ImportantMostRecent;
		std::wstring str;
		UnicodeConvert(text, str);
		if (pProvider) {
			pProvider->Release();
		}
		if (pElement) {
			pElement->Release();
		}
		pProvider = new Provider(GetForegroundWindow());

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

	bool Uia::StopSpeech() {
		return Speak("", true);
	}

	bool Uia::GetActive() {
		if (!UiaClientsAreListening()) {
			return false;
		}
		BOOL screenReaderRunning;
		BOOL result = SystemParametersInfo(SPI_GETSCREENREADER, 0, &screenReaderRunning, 0);
		if (result && screenReaderRunning) {
			HWND window = GetForegroundWindow();
			DWORD windowPid;
			GetWindowThreadProcessId(window, &windowPid);
			return windowPid == GetCurrentProcessId();
		}
		return false;
	}
}
