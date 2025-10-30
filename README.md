# MessageU

A secure client-server messaging application with end-to-end encryption. The system consists of a C++ client with RSA and AES encryption capabilities and a Python server with SQLite database backend.

## Features

- **Secure Communication**: End-to-end encryption using RSA and AES
- **User Registration**: Client registration with unique ID generation
- **Messaging**: Send and receive encrypted text messages
- **File Transfer**: Send encrypted files between clients
- **Key Exchange**: Symmetric key request and distribution
- **Client Discovery**: List all registered clients
- **Message Queue**: Retrieve waiting messages from server

## 📋 Prerequisites

### Server Requirements
- **Python 3.7+**
- No external dependencies (uses Python standard library only)

### Client Requirements
- **C++11** compatible compiler (g++)
- **Crypto++** library (for cryptographic operations)

## Building

### Build the Client

```bash
cd client
make
```

The executable will be created at `client/build/messageu`

### Clean Build Files

```bash
cd client
make clean
```

## Running

### 1. Start the Server

```bash
cd server
python3 server.py
```

**Server Options:**
```bash
# Run on custom port
python3 server.py 8080

# Keep existing database (don't reset)
python3 server.py --no-reset

# Reset database (default)
python3 server.py --reset
```

The server will:
- Read port from `myport.info` or create it with default port 1357
- Initialize SQLite database (`defensive.db`)
- Listen for incoming connections

### 2. Configure Client

Create `server.info` in the client directory:
```
127.0.0.1:1357
```

Format: `<server_ip>:<port>`

### 3. Run the Client

```bash
cd client
./build/messageu
```

## 📡 Protocol

### Request Codes
- `600` - Register new client
- `601` - Request client list
- `602` - Request public key
- `603` - Send message
- `604` - Get waiting messages

### Response Codes
- `2100` - Registration successful
- `2101` - Client list response
- `2102` - Public key response
- `2103` - Message sent confirmation
- `2104` - Waiting messages response
- `9000` - General error

### Message Types
- `1` - Symmetric key request
- `2` - Symmetric key send
- `3` - Text message
- `4` - File

## 🔐 Security Features

### Encryption
- **RSA (160-byte keys)**: Asymmetric encryption for key exchange
- **AES**: Symmetric encryption for message content
- **Base64**: Encoding for binary data transmission

### Key Management
- Each client generates RSA key pair on registration
- Private keys stored locally (`priv.key`)
- Public keys stored on server
- AES symmetric keys exchanged securely

## Project Structure

```
messageu/
├── client/
│   ├── src/              # C++ source files
│   │   ├── main.cc
│   │   ├── client.cc
│   │   ├── protocol.cc
│   │   ├── message.cc
│   │   └── crypto wrappers (.cpp)
│   ├── include/          # Header files
│   │   ├── client.h
│   │   ├── protocol.h
│   │   ├── message.h
│   │   └── crypto/       # Crypto wrapper headers
│   ├── build/            # Build output (generated)
│   ├── Makefile
│   └── server.info       # Server connection info
│
└── server/
    ├── server.py         # Main server application
    ├── database.py       # SQLite database handler
    ├── message_handler.py # Request/response handler
    ├── protocol.py       # Protocol definitions
    ├── requirements.txt
    ├── myport.info       # Server port configuration
    └── defensive.db      # SQLite database (generated)
```

## lient Menu Options

**110** - Register with server  
**120** - Get list of registered clients  
**130** - Get public key of a client  
**140** - Get waiting messages  
**150** - Send text message  
**151** - Send symmetric key request  
**152** - Send symmetric key  
**153** - Send file  
**0** - Exit

## Database Schema

The server uses SQLite with the following tables:

- **clients**: Stores client ID, username, public key, and last seen
- **messages**: Stores message ID, sender, recipient, type, content, and timestamp
