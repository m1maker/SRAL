#include "Encoding.h"
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

bool UnicodeConvert(const std::string& input, std::wstring& output) {
#ifdef _WIN32
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, NULL, 0);
	if (size_needed == 0) {
		return false;
	}
	std::vector<wchar_t> wide_string(size_needed);
	if (MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, &wide_string[0], size_needed) == 0) {
		return false;
	}
	output.assign(wide_string.begin(), wide_string.end() - 1); // Remove null terminator
	return true;
#endif
}

bool UnicodeConvert(const std::wstring& input, std::string& output) {
#ifdef _WIN32
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, NULL, 0, NULL, NULL);
	if (size_needed == 0) {
		return false;
	}
	std::vector<char> multi_byte_string(size_needed);
	if (WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, &multi_byte_string[0], size_needed, NULL, NULL) == 0) {
		return false;
	}
	output.assign(multi_byte_string.begin(), multi_byte_string.end() - 1); // Remove null terminator
	return true;
#endif
}



void XmlEncode(std::string& data) {
	std::string encoded;
	encoded.reserve(data.size()); // Reserve space for efficiency

	for (char c : data) {
		switch (c) {
		case '&':
			encoded += "&amp;";
			break;
		case '<':
			encoded += "&lt;";
			break;
		case '>':
			encoded += "&gt;";
			break;
		case '"':
			encoded += "&quot;";
			break;
		case '\'':
			encoded += "&apos;";
			break;
		default:
			encoded += c; // Copy the character as is
			break;
		}
	}

	data = encoded; // Update the original string with the encoded version
}

