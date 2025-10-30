#include "message.h"
#include "protocol.h"
#include <cstring>
#include <sstream>
#include <iomanip>

std::vector<ClientInfo> MessageUtils::parseClientList(const std::vector<uint8_t>& payload) {
    std::vector<ClientInfo> clients;
    
    size_t offset = 0;
    while (offset + CLIENT_ID_SIZE + USERNAME_MAX_SIZE <= payload.size()) {
        ClientInfo client;
        
        std::memcpy(client.id, payload.data() + offset, CLIENT_ID_SIZE);
        offset += CLIENT_ID_SIZE;
        
        const char* name_ptr = reinterpret_cast<const char*>(payload.data() + offset);
        client.name = std::string(name_ptr);
        offset += USERNAME_MAX_SIZE;
        
        clients.push_back(client);
    }
    
    return clients;
}

std::vector<uint8_t> MessageUtils::parsePublicKey(const std::vector<uint8_t>& payload, uint8_t* client_id) {
    if (payload.size() < CLIENT_ID_SIZE + PUBLIC_KEY_SIZE) {
        throw std::runtime_error("Invalid public key response");
    }
    
    std::memcpy(client_id, payload.data(), CLIENT_ID_SIZE);
    
    std::vector<uint8_t> public_key(
        payload.begin() + CLIENT_ID_SIZE,
        payload.begin() + CLIENT_ID_SIZE + PUBLIC_KEY_SIZE
    );
    
    return public_key;
}

std::vector<Message> MessageUtils::parseMessages(const std::vector<uint8_t>& payload) {
    std::vector<Message> messages;
    
    // Parse variable-length message queue by tracking byte offset
    size_t offset = 0;
    while (offset < payload.size()) {
        if (offset + CLIENT_ID_SIZE + 4 + 1 + 4 > payload.size()) {
            break;
        }
        
        Message msg;
        
        std::memcpy(msg.from_client, payload.data() + offset, CLIENT_ID_SIZE);
        offset += CLIENT_ID_SIZE;
        
        msg.id = payload[offset] | (payload[offset + 1] << 8) | 
                 (payload[offset + 2] << 16) | (payload[offset + 3] << 24);
        offset += 4;
        
        msg.type = payload[offset];
        offset += 1;
        
        uint32_t content_size = payload[offset] | (payload[offset + 1] << 8) |
                                (payload[offset + 2] << 16) | (payload[offset + 3] << 24);
        offset += 4;
        
        if (offset + content_size <= payload.size()) {
            msg.content = std::vector<uint8_t>(
                payload.begin() + offset,
                payload.begin() + offset + content_size
            );
            offset += content_size;
        }
        
        messages.push_back(msg);
    }
    
    return messages;
}

std::string MessageUtils::bytesToHex(const uint8_t* bytes, size_t length) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; i++) {
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

void MessageUtils::hexToBytes(const std::string& hex, uint8_t* bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        std::string byteStr = hex.substr(i * 2, 2);
        bytes[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
    }
}

std::string MessageUtils::clientIdToString(const uint8_t* client_id) {
    return bytesToHex(client_id, CLIENT_ID_SIZE);
}

