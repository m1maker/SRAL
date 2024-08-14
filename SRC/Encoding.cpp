#include "Encoding.h"
#include <codecvt>
bool UnicodeConvert(const std::string& input, std::wstring& output) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	try {
		output = converter.from_bytes(input);
	}
	catch (const std::exception& e) { return false; }
	return true;
}
bool UnicodeConvert(const std::wstring& input, std::string& output) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	try {
		output = converter.to_bytes(input);
	}
	catch (const std::exception& e) { return false; }
	return true;
}
