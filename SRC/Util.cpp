#include "Util.h"
#include <algorithm>
#include <regex>


bool IsSsml(const std::string& str) {
	std::string cpy_str = str;
	cpy_str.erase(std::remove_if(cpy_str.begin(), cpy_str.end(), ::isspace), cpy_str.end());
	size_t pos = cpy_str.find("<speak>");
	if (pos == 0 && cpy_str.find("</speak>") != std::string::npos) {
		return true;
	}
	return false;
}
bool AddSsml(std::string& str) {
	if (IsSsml(str))return true;
	str = "<speak>" + str + "</speak>";
	return IsSsml(str);
}
bool RemoveSsml(std::string& str) {
	if (!IsSsml(str))return true;
	std::regex ssml_tags("<[^>]+>");
	str = std::regex_replace(str, ssml_tags, ""); // Replace SSML tags with an empty string
	return true;
}

