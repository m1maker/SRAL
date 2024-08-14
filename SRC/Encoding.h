#ifndef ENCODING_H_
#define ENCODING_H_
#pragma once
#ifndef _WIN32
#error "Do not include Encoding.h in other platforms."
#endif
bool UnicodeConvert(const char* input, wchar_t* output);
bool UnicodeConvert(const wchar_t* input, char* output);
#endif // ENCODING_H