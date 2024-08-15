#ifndef ENCODING_H_
#define ENCODING_H_
#pragma once
#include <string>
bool UnicodeConvert(const std::string& input, std::wstring& output);
bool UnicodeConvert(const std::wstring& input, std::string& output);
#endif // ENCODING_H