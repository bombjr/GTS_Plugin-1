#include "Utils/Text.hpp"

namespace GTS {

	bool starts_with(std::string_view arg, std::string_view prefix) {
		return arg.compare(0, prefix.size(), prefix);
	}

	bool matches(std::string_view str, std::string_view reg) {
		std::regex the_regex(std::string(reg).c_str());
		return std::regex_match(std::string(str), the_regex);
	}

	std::string str_tolower(std::string s) {
		ranges::transform(s, s.begin(),[](unsigned char c){
			return std::tolower(c);
		});
		return s;
	}

	std::string str_toupper(std::string s) {
		ranges::transform(s, s.begin(),[](unsigned char c){
			return std::toupper(c);
		});
		return s;
	}

	// courtesy of https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
	void replace_first(std::string& s, std::string const& toReplace, std::string const& replaceWith) {
		std::size_t pos = s.find(toReplace);
		if (pos == std::string::npos) {
			return;
		}
		s.replace(pos, toReplace.length(), replaceWith);
	}

	std::string remove_whitespace(std::string s) {
		s.erase(remove(s.begin(),s.end(),' '), s.end());
		return s;
	}

	// Trims whitespace from the beginning and end of the string
	std::string trim(const std::string& s) {
		auto start = s.begin();
		while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) {
			++start;
		}

		auto end = s.end();
		do {
			--end;
		}
		while (end != start && std::isspace(static_cast<unsigned char>(*end)));

		return string(start, end + 1);
	}

	// In-place trimming functions for a std::string
	void ltrim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			[](unsigned char ch) { return !std::isspace(ch); }));
	}

	void rtrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(),
			[](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
	}


}
