#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct ClientInfo {
    uint8_t id[16];
    std::string name;
};

struct Message {
    uint32_t id;
    uint8_t from_client[16];
    uint8_t type;
    std::vector<uint8_t> content;
};

class MessageUtils {
public:
    static std::vector<ClientInfo> parseClientList(const std::vector<uint8_t>& payload);
    // Also populates client_id output parameter with the key owner's ID
    static std::vector<uint8_t> parsePublicKey(const std::vector<uint8_t>& payload, uint8_t* client_id);
    static std::vector<Message> parseMessages(const std::vector<uint8_t>& payload);
    static std::string bytesToHex(const uint8_t* bytes, size_t length);
    static void hexToBytes(const std::string& hex, uint8_t* bytes, size_t length);
    static std::string clientIdToString(const uint8_t* client_id);
};

