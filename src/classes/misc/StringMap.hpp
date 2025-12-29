#pragma once
#include <string>
#include <unordered_map>

struct StringHash {
    using is_transparent = void;

    size_t operator()(const char* str) const {
        uint64_t hash = 0xcbf29ce484222325;
        while (*str) {
            hash ^= *str++;
            hash *= 0x100000001b3;
        }
        return hash;
    }
    size_t operator()(std::string_view str) const {
        uint64_t hash = 0xcbf29ce484222325;
        for (char c : str) {
            hash ^= c;
            hash *= 0x100000001b3;
        }
        return hash;
    }
    size_t operator()(const std::string& str) const {
        return (*this)(std::string_view(str));
    }
};

template <class T>
using StringMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;
