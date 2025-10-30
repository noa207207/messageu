#include "crypto/RSAPrivateWrapper.h"
#include <fstream>
#include <stdexcept>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/queue.h>

RSAPrivateWrapper::RSAPrivateWrapper() {
    try {
        CryptoPP::AutoSeededRandomPool rng;
        CryptoPP::RSA::PrivateKey privateKey;
        privateKey.GenerateRandomWithKeySize(rng, KEY_SIZE);
        
        std::string privateKeyStr;
        CryptoPP::StringSink privateKeySink(privateKeyStr);
        privateKey.Save(privateKeySink);
        private_key = std::vector<uint8_t>(privateKeyStr.begin(), privateKeyStr.end());
        
        CryptoPP::RSA::PublicKey publicKey(privateKey);
        std::string publicKeyStr;
        CryptoPP::StringSink publicKeySink(publicKeyStr);
        publicKey.Save(publicKeySink);
        public_key = std::vector<uint8_t>(publicKeyStr.begin(), publicKeyStr.end());
        
        if (public_key.size() > PUBLIC_KEY_SIZE) {
            public_key.resize(PUBLIC_KEY_SIZE);
        } else if (public_key.size() < PUBLIC_KEY_SIZE) {
            public_key.resize(PUBLIC_KEY_SIZE, 0);
        }
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("RSA key generation failed: ") + e.what());
    }
}

RSAPrivateWrapper::RSAPrivateWrapper(const std::vector<uint8_t>& private_key) 
    : private_key(private_key) {
    try {
        CryptoPP::RSA::PrivateKey privateKey;
        CryptoPP::ArraySource as(private_key.data(), private_key.size(), true);
        privateKey.Load(as);
        
        CryptoPP::RSA::PublicKey publicKey(privateKey);
        std::string publicKeyStr;
        CryptoPP::StringSink publicKeySink(publicKeyStr);
        publicKey.Save(publicKeySink);
        public_key = std::vector<uint8_t>(publicKeyStr.begin(), publicKeyStr.end());
        
        if (public_key.size() > PUBLIC_KEY_SIZE) {
            public_key.resize(PUBLIC_KEY_SIZE);
        } else if (public_key.size() < PUBLIC_KEY_SIZE) {
            public_key.resize(PUBLIC_KEY_SIZE, 0);
        }
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("RSA key loading failed: ") + e.what());
    }
}

std::vector<uint8_t> RSAPrivateWrapper::decrypt(const std::vector<uint8_t>& ciphertext) {
    try {
        CryptoPP::RSA::PrivateKey privateKey;
        CryptoPP::ArraySource as(private_key.data(), private_key.size(), true);
        privateKey.Load(as);
        
        CryptoPP::AutoSeededRandomPool rng;
        CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(privateKey);
        
        std::string plaintext;
        CryptoPP::StringSource ss(
            ciphertext.data(), ciphertext.size(), true,
            new CryptoPP::PK_DecryptorFilter(
                rng, decryptor,
                new CryptoPP::StringSink(plaintext)
            )
        );
        
        return std::vector<uint8_t>(plaintext.begin(), plaintext.end());
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("RSA decryption failed: ") + e.what());
    }
}

std::string RSAPrivateWrapper::decryptToString(const std::vector<uint8_t>& ciphertext) {
    auto plaintext = decrypt(ciphertext);
    return std::string(plaintext.begin(), plaintext.end());
}

void RSAPrivateWrapper::savePrivateKey(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    
    file.write(reinterpret_cast<const char*>(private_key.data()), private_key.size());
    file.close();
}

RSAPrivateWrapper RSAPrivateWrapper::loadPrivateKey(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file for reading: " + filename);
    }
    
    std::vector<uint8_t> key((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();
    
    return RSAPrivateWrapper(key);
}

