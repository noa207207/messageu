#include "crypto/RSAPublicWrapper.h"
#include <stdexcept>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>

RSAPublicWrapper::RSAPublicWrapper(const std::vector<uint8_t>& public_key) 
    : public_key(public_key) {
    if (public_key.size() != KEY_SIZE) {
        throw std::runtime_error("Invalid RSA public key size");
    }
}

std::vector<uint8_t> RSAPublicWrapper::encrypt(const std::vector<uint8_t>& plaintext) {
    try {
        CryptoPP::RSA::PublicKey publicKey;
        CryptoPP::ArraySource as(public_key.data(), public_key.size(), true);
        publicKey.Load(as);
        
        CryptoPP::AutoSeededRandomPool rng;
        CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(publicKey);
        
        std::string ciphertext;
        CryptoPP::StringSource ss(
            plaintext.data(), plaintext.size(), true,
            new CryptoPP::PK_EncryptorFilter(
                rng, encryptor,
                new CryptoPP::StringSink(ciphertext)
            )
        );
        
        return std::vector<uint8_t>(ciphertext.begin(), ciphertext.end());
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("RSA encryption failed: ") + e.what());
    }
}

std::vector<uint8_t> RSAPublicWrapper::encrypt(const std::string& plaintext) {
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    return encrypt(data);
}

