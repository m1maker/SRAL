#include "Encoding.h"
#include <Windows.h>
bool UnicodeConvert(const char* input, wchar_t* output) {

	int required_length = MultiByteToWideChar(CP_UTF8, 0, input, -1, NULL, 0);
	if (required_length <= 0) {
		return false;
	}
	return !!MultiByteToWideChar(CP_UTF8, 0, input, -1, output, required_length);
}

bool UnicodeConvert(const wchar_t* input, char* output) {
	int required_length = WideCharToMultiByte(CP_UTF8, 0, input, -1, NULL, 0, NULL, NULL);
	if (required_length <= 0) {
		return false;
	}
	return !!WideCharToMultiByte(CP_UTF8, 0, input, -1, output, required_length, NULL, NULL);
}