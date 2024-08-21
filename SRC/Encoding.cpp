#include "Encoding.h"
#include <windows.h>
#include <vector>


bool UnicodeConvert(const std::string& input, std::wstring& output) {
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
}

bool UnicodeConvert(const std::wstring& input, std::string& output) {
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
}