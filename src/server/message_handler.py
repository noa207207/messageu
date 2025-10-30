import struct
import logging
from protocol import *
from database import Database

logger = logging.getLogger(__name__)

class MessageHandler:
    def __init__(self, database):
        self.db = database
    
    def handle_request(self, data):
        try:
            if len(data) < HEADER_SIZE:
                logger.error("Request too short")
                return self._error_response()
            
            header = unpack_request_header(data)
            client_id = header['client_id']
            code = header['code']
            payload_size = header['payload_size']
            payload = data[HEADER_SIZE:HEADER_SIZE + payload_size]
            
            logger.info(f"Request code {code} from client {client_id.hex()}")
            
            if code == REQ_REGISTER:
                return self._handle_register(payload)
            elif code == REQ_CLIENT_LIST:
                return self._handle_client_list(client_id)
            elif code == REQ_PUBLIC_KEY:
                return self._handle_public_key(client_id, payload)
            elif code == REQ_SEND_MESSAGE:
                return self._handle_send_message(client_id, code, payload)
            elif code == REQ_WAITING_MESSAGES:
                return self._handle_waiting_messages(client_id)
            elif code == REQ_EXIT:
                return b''
            else:
                logger.error(f"Unknown request code: {code}")
                return self._error_response()
                
        except Exception as e:
            logger.error(f"Error handling request: {e}", exc_info=True)
            return self._error_response()
    
    def _handle_register(self, payload):
        try:
            if len(payload) < USERNAME_MAX_SIZE + PUBLIC_KEY_SIZE:
                logger.error("Invalid registration payload size")
                return self._error_response()
            
            name_bytes = payload[:USERNAME_MAX_SIZE]
            public_key = payload[USERNAME_MAX_SIZE:USERNAME_MAX_SIZE + PUBLIC_KEY_SIZE]
            
            name = name_bytes.split(b'\x00')[0].decode('ascii', errors='ignore')
            
            if not name:
                logger.error("Empty username")
                return self._error_response()
            
            client_id = self.db.register_client(name, public_key)
            
            if client_id is None:
                logger.error(f"Registration failed for {name}")
                return self._error_response()
            
            logger.info(f"Registration successful for {name}")
            return pack_response(RES_REGISTRATION_SUCCESS, client_id)
            
        except Exception as e:
            logger.error(f"Registration error: {e}", exc_info=True)
            return self._error_response()
    
    def _handle_client_list(self, client_id):
        try:
            self.db.update_last_seen(client_id)
            
            clients = self.db.get_all_clients()
            
            payload = b''
            for cid, name in clients:
                payload += pack_client_info(cid, name)
            
            logger.info(f"Sending list of {len(clients)} clients")
            return pack_response(RES_CLIENT_LIST, payload)
            
        except Exception as e:
            logger.error(f"Client list error: {e}", exc_info=True)
            return self._error_response()
    
    def _handle_public_key(self, client_id, payload):
        try:
            self.db.update_last_seen(client_id)
            
            if len(payload) < CLIENT_ID_SIZE:
                logger.error("Invalid public key request payload")
                return self._error_response()
            
            target_client_id = payload[:CLIENT_ID_SIZE]
            
            public_key = self.db.get_client_public_key(target_client_id)
            
            if public_key is None:
                logger.error(f"Public key not found for {target_client_id.hex()}")
                return self._error_response()
            
            response_payload = target_client_id + public_key
            
            logger.info(f"Sending public key for {target_client_id.hex()}")
            return pack_response(RES_PUBLIC_KEY, response_payload)
            
        except Exception as e:
            logger.error(f"Public key error: {e}", exc_info=True)
            return self._error_response()
    
    def _handle_waiting_messages(self, client_id):
        try:
            self.db.update_last_seen(client_id)
            
            messages = self.db.get_client_messages(client_id)
            
            payload = b''
            for msg in messages:
                msg_payload = pack_message_info(
                    msg['from_client'],
                    msg['id'],
                    msg['type'],
                    msg['content']
                )
                payload += msg_payload
            
            logger.info(f"Sending {len(messages)} waiting messages to {client_id.hex()}")
            
            if messages:
                self.db.delete_messages(client_id)
            
            return pack_response(RES_WAITING_MESSAGES, payload)
            
        except Exception as e:
            logger.error(f"Waiting messages error: {e}", exc_info=True)
            return self._error_response()
    
    def _handle_send_message(self, from_client_id, code, payload):
        try:
            self.db.update_last_seen(from_client_id)
            
            if len(payload) < CLIENT_ID_SIZE + 1 + 4:
                logger.error("Invalid send message payload")
                return self._error_response()
            
            to_client_id = payload[:CLIENT_ID_SIZE]
            msg_type = payload[CLIENT_ID_SIZE]
            content_size = struct.unpack('<I', payload[CLIENT_ID_SIZE + 1:CLIENT_ID_SIZE + 5])[0]
            content = payload[CLIENT_ID_SIZE + 5:CLIENT_ID_SIZE + 5 + content_size]
            
            if not self.db.client_exists(to_client_id):
                logger.error(f"Recipient {to_client_id.hex()} does not exist")
                return self._error_response()
            
            message_id = self.db.save_message(to_client_id, from_client_id, msg_type, content)
            
            response_payload = to_client_id + struct.pack('<I', message_id)
            
            logger.info(f"Message {message_id} sent from {from_client_id.hex()} to {to_client_id.hex()}")
            return pack_response(RES_MESSAGE_SENT, response_payload)
            
        except Exception as e:
            logger.error(f"Send message error: {e}", exc_info=True)
            return self._error_response()
    
    def _error_response(self):
        return pack_response(RES_GENERAL_ERROR, b'')

