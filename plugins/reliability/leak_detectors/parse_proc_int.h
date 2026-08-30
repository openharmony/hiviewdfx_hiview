#ifndef PARSE_PROC_INT_H
#define PARSE_PROC_INT_H

#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>

inline bool ParseProcInt32(const std::string &s, int32_t &out)
{
    if (s.empty()) {
        return false;
    }
    int32_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseProcInt64(const std::string &s, int64_t &out)
{
    if (s.empty()) {
        return false;
    }
    int64_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseProcU64(const std::string &s, uint64_t &out)
{
    if (s.empty()) {
        return false;
    }
    uint64_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

#endif
