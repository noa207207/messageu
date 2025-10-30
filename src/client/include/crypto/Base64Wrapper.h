#pragma once

#include <string>
#include <vector>

class Base64Wrapper {
public:
    static std::string encode(const std::vector<uint8_t>& data);
    static std::string encode(const uint8_t* data, size_t length);
    static std::vector<uint8_t> decode(const std::string& encoded);
};

