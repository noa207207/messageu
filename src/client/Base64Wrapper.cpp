#include "crypto/Base64Wrapper.h"
#include <stdexcept>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>

std::string Base64Wrapper::encode(const std::vector<uint8_t>& data) {
    return encode(data.data(), data.size());
}

std::string Base64Wrapper::encode(const uint8_t* data, size_t length) {
    try {
        std::string encoded;
        CryptoPP::StringSource ss(
            data, length, true,
            new CryptoPP::Base64Encoder(
                new CryptoPP::StringSink(encoded),
                false
            )
        );
        return encoded;
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("Base64 encoding failed: ") + e.what());
    }
}

std::vector<uint8_t> Base64Wrapper::decode(const std::string& encoded_string) {
    try {
        std::string decoded;
        CryptoPP::StringSource ss(
            encoded_string, true,
            new CryptoPP::Base64Decoder(
                new CryptoPP::StringSink(decoded)
            )
        );
        return std::vector<uint8_t>(decoded.begin(), decoded.end());
    } catch (const CryptoPP::Exception& e) {
        throw std::runtime_error(std::string("Base64 decoding failed: ") + e.what());
    }
}

