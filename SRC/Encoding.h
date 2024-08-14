#ifndef ENCODING_H_
#define ENCODING_H_
#pragma once
#ifndef _WIN32
#error "Do not include Encoding.h in other platforms."
#endif
#include <string>
bool UnicodeConvert(const std::string& input, std::wstring& output);
bool UnicodeConvert(const std::wstring& input, std::string& output);
#endif // ENCODING_H