#include "string_util.h"

void SplitString(const std::string& s, char c, std::vector<std::string>& v) {
    ::size_t pos = 0, from = 0;
    while ((pos = s.find_first_of(c, from)) != std::string::npos) {
        if (pos != from) v.push_back(s.substr(from, pos - from));
        from = pos + 1;
    }
    if (from < s.size()) v.push_back(s.substr(from));
}

void TrimString(std::string& s) {
    if (s.empty() || (s[0] != ' ' && *s.rbegin() != ' ')) return;

    ::size_t bp = s.find_first_not_of(' ');
    if (bp == std::string::npos) {
        s.clear();
        return;
    }

    ::size_t ep = s.find_last_not_of(' ');
    s = s.substr(bp, ep - bp + 1);
}
