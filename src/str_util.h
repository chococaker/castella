#pragma once

#include <string>
#include <vector>

namespace choco::util {
// https://stackoverflow.com/a/46931770
inline std::vector<std::string> split(const std::string& s,
                                      const std::string& delim) {
    if (s.empty()) return {};

    size_t pos_start = 0, pos_end, delim_len = delim.length();
    std::vector<std::string> res;
    while ((pos_end = s.find(delim, pos_start)) != std::string::npos) {
        std::string token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(s.substr(pos_start));
    res.shrink_to_fit();
    return res;
}

inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) {
                             return !std::isspace(ch);
                         })
                .base(),
            s.end());
}

inline int findRange(const std::string& string, const std::string& start,
                     const std::string& end) {
    const auto startPos = string.find(start);
    const auto endPos = string.find(end);

    if (startPos == std::string::npos || endPos == std::string::npos) return -1;

    const auto startPosOffset = startPos + start.length();

    return endPos - startPosOffset - 1;
}

template<typename T>
std::optional<T> findElement(const std::vector<std::string>& haystack,
                             std::string_view needle) {
    auto position = std::find(haystack.begin(), haystack.end(), needle);
    auto index = position - haystack.begin();
    if (position == haystack.end()) return std::nullopt;

    if constexpr (std::is_same_v<T, int>)
        return std::stoi(haystack[index + 1]);
    else if constexpr (std::is_same_v<T, float>)
        return std::stof(haystack[index + 1]);
    else if constexpr (std::is_same_v<T, uint64_t>)
        return std::stoull(haystack[index + 1]);
    else if constexpr (std::is_same_v<T, int64_t>)
        return std::stoll(haystack[index + 1]);
    else if constexpr (std::is_same_v<T, bool>)
        return haystack[index + 1] == "true";
    else if (static_cast<size_t>(index + 1) < haystack.size())
        return haystack[index + 1];

    return std::nullopt;
}
} // namespace choco::util
