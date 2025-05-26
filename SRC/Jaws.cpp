#ifdef _WIN32
#include "Encoding.h"
#include "Jaws.h"
#include <string>
namespace Sral {

	bool Jaws::Initialize() {
		HRESULT hr = CoCreateInstance(CLSID_JawsApi, NULL, CLSCTX_INPROC_SERVER, IID_IJawsApi, (void**)&JawsAPI);
		if (JawsAPI || FAILED(hr)) {
			return false;
		}
		return true;
	}
	bool Jaws::Uninitialize() {
		if (JawsAPI != nullptr) {
			JawsAPI->Release();
			JawsAPI = NULL;
		}
		return true;
	}
	bool Jaws::GetActive() {
		return (!!FindWindowW(L"JFWUI2", NULL));
	}
	bool Jaws::Speak(const char* text, bool interrupt) {
		if (!GetActive() || !JawsAPI)return false;
		if (interrupt)JawsAPI->StopSpeech();
		std::wstring str;
		UnicodeConvert(text, str);
		const BSTR bstr = SysAllocString(str.c_str());
		VARIANT_BOOL result = VARIANT_FALSE;
		const VARIANT_BOOL flush = interrupt ? VARIANT_TRUE : VARIANT_FALSE;
		const bool succeeded = SUCCEEDED(JawsAPI->SayString(bstr, flush, &result));
		SysFreeString(bstr);
		return (succeeded && result == VARIANT_TRUE);
	}
	bool Jaws::Braille(const char* text) {
		if (!GetActive() || !JawsAPI)return false;
		std::wstring wstr;
		UnicodeConvert(text, wstr);
		std::wstring::size_type i = wstr.find_first_of(L"\"");
		while (i != std::wstring::npos) {
			wstr[i] = L'\'';
			i = wstr.find_first_of(L"\"", i + 1);
		}
		wstr.insert(0, L"BrailleString(\"");
		wstr.append(L"\")");
		const BSTR bstr = SysAllocString(wstr.c_str());
		VARIANT_BOOL result = VARIANT_FALSE;
		SysFreeString(bstr);
		const bool succeeded = SUCCEEDED(JawsAPI->RunFunction(bstr, &result));
		return (succeeded && result == VARIANT_TRUE);
	}

	bool Jaws::StopSpeech() {
		if (!GetActive() || !JawsAPI)return false;
		return SUCCEEDED(JawsAPI->StopSpeech());
	}
}
#endif