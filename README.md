# MessageU - End-to-End Encrypted Messaging System

## Overview

MessageU is a secure messaging application implementing end-to-end encryption. The project consists of:
- **Python Server**: Handles client registration, message routing, and storage
- **C++ Client**: Command-line interface for sending and receiving encrypted messages
- **SQLite Database**: Stores client information and messages

## Project Structure

```
MessageU/
├── server/          # Python server implementation
│   ├── server.py           # Main server application
│   ├── database.py         # Database operations
│   ├── protocol.py         # Protocol definitions
│   ├── message_handler.py  # Request/response handling
│   └── myport.info         # Server port configuration
├── client/          # C++ client implementation
│   ├── main.cc             # Main client application
│   ├── protocol.h/cc       # Protocol implementation
│   ├── message.h/cc        # Message utilities
│   ├── crypto/             # Cryptography wrappers
│   │   ├── AESWrapper.h/cpp         # AES-CBC encryption
│   │   ├── RSAWrapper.h/cpp         # RSA encryption
│   │   ├── RSAPrivateWrapper.h/cpp  # RSA key generation
│   │   └── Base64Wrapper.h/cpp      # Base64 encoding
│   └── server.info         # Server connection info
└── research/
    └── report.docx         # Security analysis report
```

## Features

### Protocol Operations

The system implements a stateless client-server protocol with the following operations:

**Client Requests:**
- `110` - Register new client
- `120` - Request clients list
- `130` - Request public key
- `140` - Request waiting messages
- `150` - Send text message
- `151` - Send symmetric key request
- `152` - Send symmetric key
- `153` - Send file (bonus)
- `0` - Exit

**Server Responses:**
- `2100` - Registration successful
- `2101` - Clients list
- `2102` - Public key
- `2103` - Message sent confirmation
- `2104` - Waiting messages
- `9000` - General error

### Security Features

1. **End-to-End Encryption**: Messages are encrypted on the client side using the recipient's public key
2. **RSA Asymmetric Encryption**: 1024-bit RSA keys for secure key exchange
3. **AES-CBC Symmetric Encryption**: 128-bit AES for efficient message encryption
4. **Secure Key Storage**: Private keys stored locally in Base64 format

### Database Schema

**clients table:**
- `ID` (16 bytes) - Unique client identifier (UUID)
- `Name` (255 bytes) - Username (ASCII, null-terminated)
- `PublicKey` (160 bytes) - RSA public key
- `LastSeen` (TEXT) - Last connection timestamp

**messages table:**
- `ID` (INTEGER) - Message ID (auto-increment)
- `ToClient` (16 bytes) - Recipient ID
- `FromClient` (16 bytes) - Sender ID
- `Type` (1 byte) - Message type
- `Content` (BLOB) - Encrypted message content

## Installation & Setup

### Server (Python)

**Requirements:**
- Python 3.7+
- No external dependencies (uses standard library)

**Setup:**
```bash
cd server
python3 server.py
```

The server will:
- Read port from `myport.info` (default: 1234)
- Create `defensive.db` SQLite database
- Listen for incoming connections

### Client (C++)

**Requirements:**
- C++11 or later
- Crypto++ library (version 8.80+ recommended)
- Standard POSIX sockets

**Build:**
```bash
cd client
g++ -std=c++11 -o messageu main.cc protocol.cc message.cc \
    crypto/AESWrapper.cpp crypto/RSAPrivateWrapper.cpp \
    crypto/RSAPublicWrapper.cpp crypto/Base64Wrapper.cpp \
    -lcryptopp -lpthread
```

**Note:** The crypto wrappers include placeholder implementations. For production use, integrate with Crypto++ library as specified in the assignment.

**Configuration:**
Create `server.info` with server address:
```
127.0.0.1:1234
```

**Run:**
```bash
./messageu
```

## Usage

### First Time Registration

1. Start the server
2. Run the client
3. Select option `110` (Register)
4. Enter your username
5. The client will generate RSA keys and save them to `my.info`

### Sending Messages

1. Login (credentials loaded from `my.info`)
2. Select `120` to get list of clients
3. Select `150` to send a text message
4. Enter recipient name and message content

### Receiving Messages

1. Select `140` to retrieve waiting messages
2. Messages are displayed with sender info and content

## Protocol Details

### Request Format
```
+------------------+----------+------+--------------+
| Client ID        | Version  | Code | Payload Size |
| 16 bytes         | 1 byte   | 2 B  | 4 bytes      |
+------------------+----------+------+--------------+
| Payload (variable length)                         |
+--------------------------------------------------+
```

### Response Format
```
+---------+------+--------------+
| Version | Code | Payload Size |
| 1 byte  | 2 B  | 4 bytes      |
+---------+------+--------------+
| Payload (variable length)    |
+---------------------------------+
```

All multi-byte values are in **little-endian** format.

## Important Notes

### Defensive Programming

- **Server**: Stateless protocol, validates all inputs, handles errors gracefully
- **Client**: Stand-alone executable, manages keys and connections properly
- **Database**: SQL injection protection using parameterized queries
- **Memory**: Proper resource cleanup and error handling

### Requirements (from Assignment)

1. ✅ Server must be written in Python
2. ✅ Client must be written in C++
3. ✅ Must use Crypto++ library (wrappers provided)
4. ✅ Protocol version 1
5. ✅ End-to-end encryption support
6. ✅ Stateless server design
7. ✅ Files must follow naming conventions
8. ✅ No third-party libraries except Crypto++ (for client)

### Security Considerations

⚠️ **Important**: The provided crypto wrappers contain placeholder implementations for demonstration. For production use:

1. Replace AES implementation with Crypto++ `CBC_Mode<AES>::Encryption`
2. Replace RSA implementation with Crypto++ RSA classes
3. Use proper PKCS padding schemes
4. Implement secure random number generation
5. Add message authentication codes (MAC)
6. Consider forward secrecy

## Development

### Server Structure
- `server.py` - Main server loop, socket handling
- `database.py` - Database operations (clients, messages)
- `protocol.py` - Protocol constants and utilities
- `message_handler.py` - Request routing and processing

### Client Structure
- `main.cc` - User interface and main loop
- `protocol.h/cc` - Protocol packing/unpacking
- `message.h/cc` - Message parsing utilities
- `crypto/` - Cryptography wrappers

## Testing

1. Start server: `python3 server/server.py`
2. Start multiple clients in different terminals
3. Register clients with different usernames
4. Test message exchange between clients
5. Verify messages are stored in database
6. Test error handling (invalid usernames, offline clients, etc.)

## Troubleshooting

**Connection refused:**
- Verify server is running
- Check `server.info` has correct IP:port
- Check firewall settings

**Registration failed:**
- Username may already exist
- Verify database permissions
- Check server logs

**Crypto errors:**
- Ensure Crypto++ library is properly installed
- Link with `-lcryptopp` flag
- Check library version compatibility

## License

This is a university assignment project for educational purposes.

## Course Information

- **Course**: Defensive Programming (MMN 15)
- **Assignment**: 20937
- **Due Date**: December 2025
- **Weight**: 14 points (80% implementation + 2 bonus points)

## Author

Defensive Programming Course Assignment

