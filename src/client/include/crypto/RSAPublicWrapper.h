#pragma once

#include <vector>
#include <string>
#include <cstdint>

class RSAPublicWrapper {
private:
    std::vector<uint8_t> public_key;
    
public:
    static constexpr size_t KEY_SIZE = 160;
    
    explicit RSAPublicWrapper(const std::vector<uint8_t>& public_key);
    
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> encrypt(const std::string& plaintext);
    
    const std::vector<uint8_t>& getKey() const { return public_key; }
};

