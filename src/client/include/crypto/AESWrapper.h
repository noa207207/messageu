#pragma once

#include <vector>
#include <string>
#include <cstdint>

class AESWrapper {
private:
    std::vector<uint8_t> key;
    // Using zero IV since we're in CBC mode - not ideal for production but matches protocol spec
    static constexpr uint8_t IV[16] = {0};
    
public:
    static constexpr size_t KEY_SIZE = 16;  // 128 bits
    static constexpr size_t IV_SIZE = 16;   // 128 bits
    
    AESWrapper();
    explicit AESWrapper(const std::vector<uint8_t>& key);
    
    static std::vector<uint8_t> generateKey();
    
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> encrypt(const std::string& plaintext);
    
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext);
    std::string decryptToString(const std::vector<uint8_t>& ciphertext);
    
    const std::vector<uint8_t>& getKey() const { return key; }
};

