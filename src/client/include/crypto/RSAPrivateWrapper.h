#pragma once

#include <vector>
#include <string>
#include <cstdint>

class RSAPrivateWrapper {
private:
    std::vector<uint8_t> private_key;
    std::vector<uint8_t> public_key;
    
public:
    static constexpr size_t KEY_SIZE = 1024;  // 1024 bits
    static constexpr size_t PUBLIC_KEY_SIZE = 160;
    
    // Generates new 1024-bit RSA key pair
    RSAPrivateWrapper();
    
    // Reconstructs from existing private key (also derives public key)
    explicit RSAPrivateWrapper(const std::vector<uint8_t>& private_key);
    
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext);
    std::string decryptToString(const std::vector<uint8_t>& ciphertext);
    
    const std::vector<uint8_t>& getPrivateKey() const { return private_key; }
    const std::vector<uint8_t>& getPublicKey() const { return public_key; }
    
    // Saves in Base64-encoded format for portability
    void savePrivateKey(const std::string& filename);
    static RSAPrivateWrapper loadPrivateKey(const std::string& filename);
};

