#include "client.h"
#include "protocol.h"
#include "message.h"
#include "crypto/RSAPrivateWrapper.h"
#include "crypto/RSAPublicWrapper.h"
#include "crypto/AESWrapper.h"
#include "crypto/Base64Wrapper.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

MessageUClient::MessageUClient() : sock(-1), rsa_private(nullptr), registered(false) {
    std::memset(client_id, 0, CLIENT_ID_SIZE);
    loadServerInfo();
}

MessageUClient::~MessageUClient() {
    disconnect();
    if (rsa_private) {
        delete rsa_private;
    }
}

void MessageUClient::loadServerInfo() {
    std::ifstream file(SERVER_INFO_FILE);
    if (!file) {
        throw std::runtime_error("Could not open " + std::string(SERVER_INFO_FILE));
    }
    
    std::string line;
    if (std::getline(file, line)) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            server_ip = line.substr(0, colon);
            server_port = std::stoi(line.substr(colon + 1));
        } else {
            server_ip = "127.0.0.1";
            server_port = std::stoi(line);
        }
    }
    file.close();
    
    std::cout << "Server: " << server_ip << ":" << server_port << std::endl;
}

bool MessageUClient::loadMyInfo() {
    std::ifstream file(MY_INFO_FILE);
    if (!file) {
        return false;
    }
    
    std::string line;
    
    if (!std::getline(file, username)) {
        return false;
    }
    
    std::string client_id_hex;
    if (!std::getline(file, client_id_hex)) {
        return false;
    }
    MessageUtils::hexToBytes(client_id_hex, client_id, CLIENT_ID_SIZE);
    
    std::string private_key_b64;
    if (!std::getline(file, private_key_b64)) {
        return false;
    }
    
    auto private_key = Base64Wrapper::decode(private_key_b64);
    rsa_private = new RSAPrivateWrapper(private_key);
    registered = true;
    
    file.close();
    return true;
}

void MessageUClient::saveMyInfo() {
    std::ofstream file(MY_INFO_FILE);
    if (!file) {
        throw std::runtime_error("Could not create " + std::string(MY_INFO_FILE));
    }
    
    file << username << std::endl;
    
    file << MessageUtils::bytesToHex(client_id, CLIENT_ID_SIZE) << std::endl;
    
    auto private_key = rsa_private->getPrivateKey();
    file << Base64Wrapper::encode(private_key) << std::endl;
    
    file.close();
}

bool MessageUClient::connect() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return false;
    }
    
    if (::connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return false;
    }
    
    std::cout << "Connected to server" << std::endl;
    return true;
}

void MessageUClient::disconnect() {
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
}

bool MessageUClient::sendRequest(const std::vector<uint8_t>& request) {
    ssize_t sent = send(sock, request.data(), request.size(), 0);
    return sent == static_cast<ssize_t>(request.size());
}

std::vector<uint8_t> MessageUClient::receiveResponse() {
    std::vector<uint8_t> header(7);
    // MSG_WAITALL blocks until all 7 bytes arrive or connection closes
    ssize_t received = recv(sock, header.data(), 7, MSG_WAITALL);
    if (received != 7) {
        throw std::runtime_error("Failed to receive response header");
    }
    
    auto resp_header = Protocol::unpackResponseHeader(header);
    
    std::vector<uint8_t> payload(resp_header.payload_size);
    if (resp_header.payload_size > 0) {
        received = recv(sock, payload.data(), resp_header.payload_size, MSG_WAITALL);
        if (received != static_cast<ssize_t>(resp_header.payload_size)) {
            throw std::runtime_error("Failed to receive response payload");
        }
    }
    
    if (resp_header.code == RES_GENERAL_ERROR) {
        throw std::runtime_error("Server responded with an error");
    }
    
    return payload;
}

void MessageUClient::registerClient() {
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    
    rsa_private = new RSAPrivateWrapper();
    auto public_key = rsa_private->getPublicKey();
    
    auto request = Protocol::packRegisterRequest(username, public_key);
    
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    if (!sendRequest(request)) {
        throw std::runtime_error("Failed to send registration request");
    }
    
    auto response = receiveResponse();
    
    if (response.size() >= CLIENT_ID_SIZE) {
        std::memcpy(client_id, response.data(), CLIENT_ID_SIZE);
        registered = true;
        saveMyInfo();
        std::cout << "Registration successful!" << std::endl;
        std::cout << "Client ID: " << MessageUtils::clientIdToString(client_id) << std::endl;
    }
    
    disconnect();
}

void MessageUClient::requestClientList() {
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    auto request = Protocol::packClientListRequest(client_id);
    
    if (!sendRequest(request)) {
        throw std::runtime_error("Failed to send client list request");
    }
    
    auto response = receiveResponse();
    auto clients = MessageUtils::parseClientList(response);
    
    std::cout << "\n=== Client List ===" << std::endl;
    for (const auto& client : clients) {
        std::cout << client.name << " (" << MessageUtils::clientIdToString(client.id) << ")" << std::endl;
    }
    
    disconnect();
}

void MessageUClient::requestPublicKey() {
    std::string target_name;
    std::cout << "Enter client name: ";
    std::getline(std::cin, target_name);
    
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    auto list_req = Protocol::packClientListRequest(client_id);
    sendRequest(list_req);
    auto list_resp = receiveResponse();
    auto clients = MessageUtils::parseClientList(list_resp);
    
    uint8_t target_id[CLIENT_ID_SIZE];
    bool found = false;
    for (const auto& client : clients) {
        if (client.name == target_name) {
            std::memcpy(target_id, client.id, CLIENT_ID_SIZE);
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "Client not found" << std::endl;
        disconnect();
        return;
    }
    
    auto request = Protocol::packPublicKeyRequest(client_id, target_id);
    sendRequest(request);
    auto response = receiveResponse();
    
    uint8_t resp_id[CLIENT_ID_SIZE];
    auto public_key = MessageUtils::parsePublicKey(response, resp_id);
    
    std::cout << "Public key received (" << public_key.size() << " bytes)" << std::endl;
    
    disconnect();
}

void MessageUClient::requestWaitingMessages() {
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    auto list_req = Protocol::packClientListRequest(client_id);
    sendRequest(list_req);
    auto list_resp = receiveResponse();
    auto clients = MessageUtils::parseClientList(list_resp);
    
    // Build lookup table to display sender names instead of hex IDs
    std::map<std::string, std::string> id_to_name;
    for (const auto& client : clients) {
        std::string id_str = MessageUtils::clientIdToString(client.id);
        id_to_name[id_str] = client.name;
    }
    
    auto request = Protocol::packWaitingMessagesRequest(client_id);
    
    if (!sendRequest(request)) {
        throw std::runtime_error("Failed to send waiting messages request");
    }
    
    auto response = receiveResponse();
    auto messages = MessageUtils::parseMessages(response);
    
    std::cout << "\n=== Waiting Messages ===" << std::endl;
    if (messages.empty()) {
        std::cout << "No messages" << std::endl;
    } else {
        for (const auto& msg : messages) {
            std::string sender_id = MessageUtils::clientIdToString(msg.from_client);
            std::string sender_name = id_to_name.count(sender_id) ? id_to_name[sender_id] : "Unknown";
            
            std::cout << "From: " << sender_name << std::endl;
            
            std::cout << "Type: ";
            switch (msg.type) {
                case MSG_TYPE_SYM_KEY_REQUEST:
                    std::cout << "Request for symmetric key" << std::endl;
                    break;
                case MSG_TYPE_SYM_KEY_SEND:
                    std::cout << "Symmetric key (encrypted)" << std::endl;
                    std::cout << "Received encrypted symmetric key (" << msg.content.size() << " bytes)" << std::endl;
                    
                    try {
                        std::cout << "Decrypting symmetric key..." << std::endl;
                        // RSA decrypt returns the AES key that was encrypted with our public key
                        auto decrypted_key_str = rsa_private->decrypt(msg.content);
                        std::vector<uint8_t> decrypted_key(decrypted_key_str.begin(), decrypted_key_str.end());
                        
                        saveSymmetricKey(msg.from_client, decrypted_key);
                        std::cout << "Symmetric key decrypted and saved (" << decrypted_key.size() << " bytes)" << std::endl;
                        std::cout << "Secure channel established with " << sender_name << std::endl;
                        std::cout << "End-to-end encryption active" << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error: Failed to decrypt symmetric key: " << e.what() << std::endl;
                        std::cout << "   The key might not have been encrypted for you." << std::endl;
                    }
                    break;
                case MSG_TYPE_TEXT_MESSAGE:
                    std::cout << "Text message" << std::endl;
                    if (!msg.content.empty()) {
                        if (hasSymmetricKey(msg.from_client)) {
                            try {
                                auto sym_key = getSymmetricKey(msg.from_client);
                                AESWrapper aes(sym_key);
                                auto decrypted = aes.decrypt(msg.content);
                                std::string text(decrypted.begin(), decrypted.end());
                                std::cout << "Content: " << text << std::endl;
                            } catch (...) {
                                std::cout << "Content: [encrypted, " << msg.content.size() << " bytes]" << std::endl;
                                std::cout << "Warning: Could not decrypt message - key mismatch" << std::endl;
                            }
                        } else {
                            std::cout << "Content: [encrypted, " << msg.content.size() << " bytes]" << std::endl;
                            std::cout << "Warning: No decryption key available from " << sender_name << std::endl;
                        }
                    }
                    break;
                case MSG_TYPE_FILE:
                    std::cout << "File" << std::endl;
                    
                    if (hasSymmetricKey(msg.from_client)) {
                        try {
                            auto sym_key = getSymmetricKey(msg.from_client);
                            AESWrapper aes(sym_key);
                            auto decrypted_file = aes.decrypt(msg.content);
                            
                            // Cross-platform temp directory resolution (Windows/Unix)
                            const char* tmp_dir = std::getenv("TMP");
                            if (!tmp_dir) {
                                tmp_dir = std::getenv("TEMP");
                            }
                            if (!tmp_dir) {
                                tmp_dir = "/tmp";
                            }
                            
                            std::string filename = std::string(tmp_dir) + "/received_" + 
                                                  std::to_string(msg.id) + ".bin";
                            
                            std::ofstream out(filename, std::ios::binary);
                            if (out) {
                                out.write(reinterpret_cast<const char*>(decrypted_file.data()), 
                                        decrypted_file.size());
                                out.close();
                                
                                std::cout << "Content: " << filename << std::endl;
                                std::cout << "         (File saved to TMP folder, " << decrypted_file.size() << " bytes)" << std::endl;
                            } else {
                                std::cout << "Content: [Failed to save file]" << std::endl;
                            }
                        } catch (...) {
                            std::cout << "Content: [Could not decrypt - key mismatch]" << std::endl;
                        }
                    } else {
                        std::cout << "Content: [No decryption key available]" << std::endl;
                    }
                    break;
                default:
                    std::cout << "Unknown (" << static_cast<int>(msg.type) << ")" << std::endl;
                    std::cout << "Content: " << msg.content.size() << " bytes" << std::endl;
                    break;
            }
            
            std::cout << "---" << std::endl;
        }
    }
    
    disconnect();
}

bool MessageUClient::hasSymmetricKey(const uint8_t* target_id) {
    std::string id_str = MessageUtils::clientIdToString(target_id);
    return symmetric_keys.count(id_str) > 0;
}

void MessageUClient::saveSymmetricKey(const uint8_t* target_id, const std::vector<uint8_t>& key) {
    std::string id_str = MessageUtils::clientIdToString(target_id);
    symmetric_keys[id_str] = key;
}

std::vector<uint8_t> MessageUClient::getSymmetricKey(const uint8_t* target_id) {
    std::string id_str = MessageUtils::clientIdToString(target_id);
    return symmetric_keys[id_str];
}

void MessageUClient::sendTextMessage() {
    std::string target_name, message;
    std::cout << "Enter recipient name: ";
    std::getline(std::cin, target_name);
    
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    auto list_req = Protocol::packClientListRequest(client_id);
    sendRequest(list_req);
    auto list_resp = receiveResponse();
    auto clients = MessageUtils::parseClientList(list_resp);
    
    uint8_t target_id[CLIENT_ID_SIZE];
    bool found = false;
    for (const auto& client : clients) {
        if (client.name == target_name) {
            std::memcpy(target_id, client.id, CLIENT_ID_SIZE);
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "Client not found" << std::endl;
        disconnect();
        return;
    }
    
    if (!hasSymmetricKey(target_id)) {
        std::cout << "\nError: Encryption required" << std::endl;
        std::cout << "No symmetric key established with " << target_name << std::endl;
        std::cout << "\nTo send encrypted messages, follow these steps:" << std::endl;
        std::cout << "  1. Request symmetric key exchange (option 151)" << std::endl;
        std::cout << "  2. Wait for " << target_name << " to respond with their key" << std::endl;
        std::cout << "  3. Check messages (option 140) to receive the key" << std::endl;
        std::cout << "  4. Then you can send encrypted messages" << std::endl;
        std::cout << "\nMessage NOT sent (encryption required)" << std::endl;
        disconnect();
        return;
    }
    
    std::cout << "Enter message: ";
    std::getline(std::cin, message);
    
    auto sym_key = getSymmetricKey(target_id);
    AESWrapper aes(sym_key);
    std::vector<uint8_t> plaintext(message.begin(), message.end());
    std::vector<uint8_t> encrypted = aes.encrypt(plaintext);
    
    auto request = Protocol::packSendMessageRequest(
        client_id, target_id, MSG_TYPE_TEXT_MESSAGE, encrypted
    );
    
    sendRequest(request);
    receiveResponse();
    
    std::cout << "Message sent successfully to " << target_name << std::endl;
    std::cout << "   (encrypted with AES-128, " << encrypted.size() << " bytes)" << std::endl;
    
    disconnect();
}

void MessageUClient::requestSymmetricKey() {
    std::string target_name;
    std::cout << "Enter client name to request symmetric key from: ";
    std::getline(std::cin, target_name);
    
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    auto list_req = Protocol::packClientListRequest(client_id);
    sendRequest(list_req);
    auto list_resp = receiveResponse();
    auto clients = MessageUtils::parseClientList(list_resp);
    
    uint8_t target_id[CLIENT_ID_SIZE];
    bool found = false;
    for (const auto& client : clients) {
        if (client.name == target_name) {
            std::memcpy(target_id, client.id, CLIENT_ID_SIZE);
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "Client not found" << std::endl;
        disconnect();
        return;
    }
    
    std::vector<uint8_t> content;
    auto request = Protocol::packSendMessageRequest(
        client_id, target_id, MSG_TYPE_SYM_KEY_REQUEST, content
    );
    
    sendRequest(request);
    receiveResponse();
    
    std::cout << "Symmetric key request sent to " << target_name << std::endl;
    std::cout << "The recipient will receive your request and can send their key using option 152." << std::endl;
    
    disconnect();
}

void MessageUClient::sendSymmetricKey() {
    std::string target_name;
    std::cout << "Enter client name to send symmetric key to: ";
    std::getline(std::cin, target_name);
    
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    auto list_req = Protocol::packClientListRequest(client_id);
    sendRequest(list_req);
    auto list_resp = receiveResponse();
    auto clients = MessageUtils::parseClientList(list_resp);
    
    uint8_t target_id[CLIENT_ID_SIZE];
    bool found = false;
    for (const auto& client : clients) {
        if (client.name == target_name) {
            std::memcpy(target_id, client.id, CLIENT_ID_SIZE);
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "Client not found" << std::endl;
        disconnect();
        return;
    }
    
    std::cout << "Fetching public key from server..." << std::endl;
    auto pubkey_req = Protocol::packPublicKeyRequest(client_id, target_id);
    sendRequest(pubkey_req);
    auto pubkey_resp = receiveResponse();
    
    uint8_t resp_id[CLIENT_ID_SIZE];
    auto target_public_key = MessageUtils::parsePublicKey(pubkey_resp, resp_id);
    std::cout << "Public key received (" << target_public_key.size() << " bytes)" << std::endl;
    
    // Default constructor generates fresh AES-128 key automatically
    AESWrapper aes;
    auto symmetric_key = aes.getKey();
    std::cout << "Generated AES symmetric key (" << symmetric_key.size() << " bytes)" << std::endl;
    
    std::cout << "Encrypting symmetric key..." << std::endl;
    RSAPublicWrapper rsa_public(target_public_key);
    std::string sym_key_str(symmetric_key.begin(), symmetric_key.end());
    auto encrypted_sym_key = rsa_public.encrypt(sym_key_str);
    std::cout << "Symmetric key encrypted (" << encrypted_sym_key.size() << " bytes)" << std::endl;
    
    auto request = Protocol::packSendMessageRequest(
        client_id, target_id, MSG_TYPE_SYM_KEY_SEND, encrypted_sym_key
    );
    
    sendRequest(request);
    receiveResponse();
    
    saveSymmetricKey(target_id, symmetric_key);
    
    std::cout << "\nKey exchange completed successfully" << std::endl;
    std::cout << "Symmetric key sent to " << target_name << std::endl;
    std::cout << "Secure channel established" << std::endl;
    
    disconnect();
}

void MessageUClient::sendFile() {
    std::string target_name, filename;
    std::cout << "Enter recipient name: ";
    std::getline(std::cin, target_name);
    
    if (!connect()) {
        throw std::runtime_error("Could not connect to server");
    }
    
    auto list_req = Protocol::packClientListRequest(client_id);
    sendRequest(list_req);
    auto list_resp = receiveResponse();
    auto clients = MessageUtils::parseClientList(list_resp);
    
    uint8_t target_id[CLIENT_ID_SIZE];
    bool found = false;
    for (const auto& client : clients) {
        if (client.name == target_name) {
            std::memcpy(target_id, client.id, CLIENT_ID_SIZE);
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "Client not found" << std::endl;
        disconnect();
        return;
    }
    
    if (!hasSymmetricKey(target_id)) {
        std::cout << "\nError: Encryption required" << std::endl;
        std::cout << "No symmetric key established with " << target_name << std::endl;
        std::cout << "\nTo send encrypted files, follow these steps:" << std::endl;
        std::cout << "  1. Request symmetric key exchange (option 151)" << std::endl;
        std::cout << "  2. Wait for " << target_name << " to respond with their key" << std::endl;
        std::cout << "  3. Check messages (option 140) to receive the key" << std::endl;
        std::cout << "  4. Then you can send encrypted files" << std::endl;
        std::cout << "\nFile NOT sent (encryption required)" << std::endl;
        disconnect();
        return;
    }
    
    std::cout << "Enter filename: ";
    std::getline(std::cin, filename);
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "Error: file not found" << std::endl;
        disconnect();
        return;
    }
    
    std::vector<uint8_t> file_contents((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
    file.close();
    
    if (file_contents.empty()) {
        std::cout << "Warning: file is empty" << std::endl;
    }
    
    std::cout << "File size: " << file_contents.size() << " bytes" << std::endl;
    
    auto sym_key = getSymmetricKey(target_id);
    AESWrapper aes(sym_key);
    std::vector<uint8_t> encrypted = aes.encrypt(file_contents);
    
    auto request = Protocol::packSendMessageRequest(
        client_id, target_id, MSG_TYPE_FILE, encrypted
    );
    
    sendRequest(request);
    receiveResponse();
    
    std::cout << "File sent successfully to " << target_name << std::endl;
    std::cout << "Original size: " << file_contents.size() << " bytes" << std::endl;
    std::cout << "Encrypted size: " << encrypted.size() << " bytes" << std::endl;
    
    disconnect();
}

void MessageUClient::showMenu() {
    std::cout << "\n=== MessageU client at your service ===" << std::endl;
    std::cout << "110) Register" << std::endl;
    std::cout << "120) Request for clients list" << std::endl;
    std::cout << "130) Request for public key" << std::endl;
    std::cout << "140) Request for waiting messages" << std::endl;
    std::cout << "150) Send a text message" << std::endl;
    std::cout << "151) Send a request for symmetric key" << std::endl;
    std::cout << "152) Send your symmetric key" << std::endl;
    std::cout << "153) Send a file" << std::endl;
    std::cout << "0) Exit client" << std::endl;
    std::cout << "? ";
}

void MessageUClient::run() {
    if (loadMyInfo()) {
        std::cout << "Loaded existing registration for: " << username << std::endl;
    }
    
    while (true) {
        showMenu();
        
        std::string choice;
        std::getline(std::cin, choice);
        
        if (choice.empty()) {
            continue;
        }
        
        try {
            for (char c : choice) {
                if (!std::isdigit(c) && c != '-') {
                    std::cout << "Invalid input. Please enter a number (110, 120, 130, 140, 150, 151, 152, 153, or 0)." << std::endl;
                    goto next_iteration;
                }
            }
            
            int cmd = std::stoi(choice);
            
            switch (cmd) {
                case 110:
                    registerClient();
                    break;
                case 120:
                    if (!registered) {
                        std::cout << "Please register first" << std::endl;
                        break;
                    }
                    requestClientList();
                    break;
                case 130:
                    if (!registered) {
                        std::cout << "Please register first" << std::endl;
                        break;
                    }
                    requestPublicKey();
                    break;
                case 140:
                    if (!registered) {
                        std::cout << "Please register first" << std::endl;
                        break;
                    }
                    requestWaitingMessages();
                    break;
                case 150:
                    if (!registered) {
                        std::cout << "Please register first" << std::endl;
                        break;
                    }
                    sendTextMessage();
                    break;
                case 151:
                    if (!registered) {
                        std::cout << "Please register first" << std::endl;
                        break;
                    }
                    requestSymmetricKey();
                    break;
                case 152:
                    if (!registered) {
                        std::cout << "Please register first" << std::endl;
                        break;
                    }
                    sendSymmetricKey();
                    break;
                case 153:
                    if (!registered) {
                        std::cout << "Please register first" << std::endl;
                        break;
                    }
                    sendFile();
                    break;
                case 0:
                    std::cout << "Goodbye!" << std::endl;
                    return;
                default:
                    std::cout << "Invalid choice. Please select a valid option." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        
        next_iteration:
        continue;
    }
}

