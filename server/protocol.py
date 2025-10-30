# Request codes
REQ_REGISTER = 600
REQ_CLIENT_LIST = 601
REQ_PUBLIC_KEY = 602
REQ_SEND_MESSAGE = 603
REQ_WAITING_MESSAGES = 604
REQ_EXIT = 0

# Response codes
RES_REGISTRATION_SUCCESS = 2100
RES_CLIENT_LIST = 2101
RES_PUBLIC_KEY = 2102
RES_MESSAGE_SENT = 2103
RES_WAITING_MESSAGES = 2104
RES_GENERAL_ERROR = 9000

# Message types
MSG_TYPE_SYM_KEY_REQUEST = 1
MSG_TYPE_SYM_KEY_SEND = 2
MSG_TYPE_TEXT_MESSAGE = 3
MSG_TYPE_FILE = 4

VERSION = 2
HEADER_SIZE = 23
CLIENT_ID_SIZE = 16
UUID_SIZE = 16
USERNAME_MAX_SIZE = 255
PUBLIC_KEY_SIZE = 160

class ProtocolError(Exception):
    pass

def pack_header(version, code, payload_size):
    import struct
    return struct.pack('<BHI', version, code, payload_size)

def unpack_request_header(data):
    import struct
    if len(data) < HEADER_SIZE:
        raise ProtocolError("Invalid header size")
    
    client_id = data[:CLIENT_ID_SIZE]
    version, code, payload_size = struct.unpack('<BHI', data[CLIENT_ID_SIZE:HEADER_SIZE])
    
    return {
        'client_id': client_id,
        'version': version,
        'code': code,
        'payload_size': payload_size
    }

def pack_response(code, payload):
    header = pack_header(VERSION, code, len(payload))
    return header + payload

def pack_client_info(client_id, name):
    import struct
    name_bytes = name.encode('ascii')[:254]
    name_bytes += b'\x00' * (USERNAME_MAX_SIZE - len(name_bytes))
    return client_id + name_bytes

def pack_message_info(client_id, message_id, msg_type, content):
    import struct
    content_size = len(content)
    header = client_id + struct.pack('<IBI', message_id, msg_type, content_size)
    return header + content

