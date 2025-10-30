#include "protocol.h"
#include <cstring>
#include <stdexcept>

std::vector<uint8_t> Protocol::packRequestHeader(
    const uint8_t* client_id,
    uint16_t code,
    uint32_t payload_size
) {
    std::vector<uint8_t> header(HEADER_SIZE);
    
    std::memcpy(header.data(), client_id, CLIENT_ID_SIZE);
    
    header[CLIENT_ID_SIZE] = VERSION;
    
    header[CLIENT_ID_SIZE + 1] = code & 0xFF;
    header[CLIENT_ID_SIZE + 2] = (code >> 8) & 0xFF;
    
    header[CLIENT_ID_SIZE + 3] = payload_size & 0xFF;
    header[CLIENT_ID_SIZE + 4] = (payload_size >> 8) & 0xFF;
    header[CLIENT_ID_SIZE + 5] = (payload_size >> 16) & 0xFF;
    header[CLIENT_ID_SIZE + 6] = (payload_size >> 24) & 0xFF;
    
    return header;
}

ResponseHeader Protocol::unpackResponseHeader(const std::vector<uint8_t>& data) {
    if (data.size() < 7) {
        throw std::runtime_error("Invalid response header size");
    }
    
    ResponseHeader header;
    header.version = data[0];
    header.code = data[1] | (data[2] << 8);
    header.payload_size = data[3] | (data[4] << 8) | (data[5] << 16) | (data[6] << 24);
    
    return header;
}

std::vector<uint8_t> Protocol::packRegisterRequest(
    const std::string& username,
    const std::vector<uint8_t>& public_key
) {
    uint8_t empty_id[CLIENT_ID_SIZE] = {0};
    uint32_t payload_size = USERNAME_MAX_SIZE + PUBLIC_KEY_SIZE;
    
    auto request = packRequestHeader(empty_id, REQ_REGISTER, payload_size);
    
    std::vector<uint8_t> username_bytes(USERNAME_MAX_SIZE, 0);
    size_t len = std::min(username.length(), size_t(USERNAME_MAX_SIZE - 1));
    std::memcpy(username_bytes.data(), username.c_str(), len);
    request.insert(request.end(), username_bytes.begin(), username_bytes.end());
    
    if (public_key.size() != PUBLIC_KEY_SIZE) {
        throw std::runtime_error("Invalid public key size");
    }
    request.insert(request.end(), public_key.begin(), public_key.end());
    
    return request;
}

std::vector<uint8_t> Protocol::packClientListRequest(const uint8_t* client_id) {
    return packRequestHeader(client_id, REQ_CLIENT_LIST, 0);
}

std::vector<uint8_t> Protocol::packPublicKeyRequest(
    const uint8_t* client_id,
    const uint8_t* target_client_id
) {
    auto request = packRequestHeader(client_id, REQ_PUBLIC_KEY, CLIENT_ID_SIZE);
    
    for (size_t i = 0; i < CLIENT_ID_SIZE; i++) {
        request.push_back(target_client_id[i]);
    }
    
    return request;
}

std::vector<uint8_t> Protocol::packWaitingMessagesRequest(const uint8_t* client_id) {
    return packRequestHeader(client_id, REQ_WAITING_MESSAGES, 0);
}

std::vector<uint8_t> Protocol::packSendMessageRequest(
    const uint8_t* from_client_id,
    const uint8_t* to_client_id,
    uint8_t msg_type,
    const std::vector<uint8_t>& content
) {
    uint32_t payload_size = CLIENT_ID_SIZE + 1 + 4 + content.size();
    auto request = packRequestHeader(from_client_id, REQ_SEND_MESSAGE, payload_size);
    
    for (size_t i = 0; i < CLIENT_ID_SIZE; i++) {
        request.push_back(to_client_id[i]);
    }
    
    request.push_back(msg_type);
    
    uint32_t content_size = content.size();
    request.push_back(content_size & 0xFF);
    request.push_back((content_size >> 8) & 0xFF);
    request.push_back((content_size >> 16) & 0xFF);
    request.push_back((content_size >> 24) & 0xFF);
    
    request.insert(request.end(), content.begin(), content.end());
    
    return request;
}

