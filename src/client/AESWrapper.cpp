#include "crypto/AESWrapper.h"
#include <stdexcept>
#include <cstring>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

constexpr uint8_t AESWrapper::IV[16];

AESWrapper::AESWrapper() {
    key = generateKey();
}

AESWrapper::AESWrapper(const std::vector<uint8_t>& key) : key(key) {
    if (key.size() != KEY_SIZE) {
        throw std::runtime_error("Invalid AES key size");
    }
}

std::vector<uint8_t> AESWrapper::generateKey() {
    std::vector<uint8_t> key(KEY_SIZE);
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(key.data(), KEY_SIZE);
    return key;
}

std::vector<uint8_t> AESWrapper::encrypt(const std::vector<uint8_t>& plaintext) {
    try {
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption;
        encryption.SetKeyWithIV(key.data(), KEY_SIZE, IV);
        
        std::string ciphertext;
        CryptoPP::StringSource ss(
            plaintext.data(), plaintext.size(), true,
            new CryptoPP::StreamTransformationFilter(
                encryption,
                new CryptoPP::StringSink(ciphertext)
            )
        );
        
        return std::vector<uint8_t>(ciphertext.begin(), ciphertext.end());
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("AES encryption failed: ") + e.what());
    }
}

std::vector<uint8_t> AESWrapper::encrypt(const std::string& plaintext) {
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    return encrypt(data);
}

std::vector<uint8_t> AESWrapper::decrypt(const std::vector<uint8_t>& ciphertext) {
    try {
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption;
        decryption.SetKeyWithIV(key.data(), KEY_SIZE, IV);
        
        std::string plaintext;
        CryptoPP::StringSource ss(
            ciphertext.data(), ciphertext.size(), true,
            new CryptoPP::StreamTransformationFilter(
                decryption,
                new CryptoPP::StringSink(plaintext)
            )
        );
        
        return std::vector<uint8_t>(plaintext.begin(), plaintext.end());
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("AES decryption failed: ") + e.what());
    }
}

std::string AESWrapper::decryptToString(const std::vector<uint8_t>& ciphertext) {
    auto plaintext = decrypt(ciphertext);
    return std::string(plaintext.begin(), plaintext.end());
}

