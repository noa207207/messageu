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
    
    // Maps client_id (as hex string) to their AES key for encrypted communication
    std::map<std::string, std::vector<uint8_t>> symmetric_keys;
    
    void loadServerInfo();
    // Returns false if no prior session exists (first-time user)
    bool loadMyInfo();
    void saveMyInfo();
    bool connect();
    void disconnect();
    // Handles partial writes in case socket buffer is full
    bool sendRequest(const std::vector<uint8_t>& request);
    // Blocks until entire response (header + payload) is received
    std::vector<uint8_t> receiveResponse();
    
    bool hasSymmetricKey(const uint8_t* target_id);
    void saveSymmetricKey(const uint8_t* target_id, const std::vector<uint8_t>& key);
    std::vector<uint8_t> getSymmetricKey(const uint8_t* target_id);
    
    void registerClient();
    void requestClientList();
    void requestPublicKey();
    void requestWaitingMessages();
    void sendTextMessage();
    // Asks recipient to send us their AES key (encrypted with our RSA public key)
    void requestSymmetricKey();
    // Encrypts our AES key with recipient's RSA public key and sends it
    void sendSymmetricKey();
    void sendFile();
    void showMenu();
    
public:
    MessageUClient();
    ~MessageUClient();
    
    void run();
};

#endif // CLIENT_H

