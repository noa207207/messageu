#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

#include "protocol.h"

class RSAPrivateWrapper;

constexpr const char* SERVER_INFO_FILE = "server.info";
constexpr const char* MY_INFO_FILE = "my.info";

class MessageUClient {
private:
    std::string server_ip;
    int server_port;
    int sock;
    uint8_t client_id[CLIENT_ID_SIZE];
    std::string username;
    RSAPrivateWrapper* rsa_private;
    bool registered;
    
    std::map<std::string, std::vector<uint8_t>> symmetric_keys;
    
    void loadServerInfo();
    bool loadMyInfo();
    void saveMyInfo();
    bool connect();
    void disconnect();
    bool sendRequest(const std::vector<uint8_t>& request);
    std::vector<uint8_t> receiveResponse();
    
    bool hasSymmetricKey(const uint8_t* target_id);
    void saveSymmetricKey(const uint8_t* target_id, const std::vector<uint8_t>& key);
    std::vector<uint8_t> getSymmetricKey(const uint8_t* target_id);
    
    void registerClient();
    void requestClientList();
    void requestPublicKey();
    void requestWaitingMessages();
    void sendTextMessage();
    void requestSymmetricKey();
    void sendSymmetricKey();
    void sendFile();
    void showMenu();
    
public:
    MessageUClient();
    ~MessageUClient();
    
    void run();
};

#endif // CLIENT_H

