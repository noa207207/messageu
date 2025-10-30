#!/usr/bin/env python3

import socket
import threading
import logging
import sys
import os
from database import Database
from message_handler import MessageHandler

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

DEFAULT_PORT = 1357
PORT_INFO_FILE = 'myport.info'

class MessageUServer:
    def __init__(self, port=None):
        self.port = port or self._read_port()
        self.database = Database()
        self.message_handler = MessageHandler(self.database)
        self.running = False
        self.server_socket = None
    
    def _read_port(self):
        try:
            if os.path.exists(PORT_INFO_FILE):
                with open(PORT_INFO_FILE, 'r') as f:
                    port = int(f.read().strip())
                    logger.info(f"Port {port} read from {PORT_INFO_FILE}")
                    return port
        except Exception as e:
            logger.warning(f"Could not read port from {PORT_INFO_FILE}: {e}")
        
        try:
            with open(PORT_INFO_FILE, 'w') as f:
                f.write(str(DEFAULT_PORT))
            logger.info(f"Created {PORT_INFO_FILE} with default port {DEFAULT_PORT}")
        except Exception as e:
            logger.error(f"Could not create {PORT_INFO_FILE}: {e}")
        
        return DEFAULT_PORT
    
    def start(self):
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind(('0.0.0.0', self.port))
            self.server_socket.listen(5)
            self.running = True
            
            logger.info(f"MessageU Server started on port {self.port}")
            logger.info("Waiting for connections...")
            
            while self.running:
                try:
                    client_socket, address = self.server_socket.accept()
                    logger.info(f"New connection from {address}")
                    
                    client_thread = threading.Thread(
                        target=self._handle_client,
                        args=(client_socket, address),
                        daemon=True
                    )
                    client_thread.start()
                    
                except Exception as e:
                    if self.running:
                        logger.error(f"Error accepting connection: {e}")
        
        except Exception as e:
            logger.error(f"Server error: {e}", exc_info=True)
        
        finally:
            self.stop()
    
    def _handle_client(self, client_socket, address):
        try:
            while True:
                data = self._receive_data(client_socket)
                if not data:
                    break
                
                response = self.message_handler.handle_request(data)
                
                if response:
                    client_socket.sendall(response)
                
        except Exception as e:
            logger.error(f"Error handling client {address}: {e}")
        
        finally:
            try:
                client_socket.close()
                logger.info(f"Connection closed from {address}")
            except:
                pass
    
    def _receive_data(self, client_socket):
        try:
            header_data = b''
            while len(header_data) < 23:
                chunk = client_socket.recv(23 - len(header_data))
                if not chunk:
                    return None
                header_data += chunk
            
            import struct
            payload_size = struct.unpack('<I', header_data[19:23])[0]
            
            payload_data = b''
            while len(payload_data) < payload_size:
                chunk = client_socket.recv(min(4096, payload_size - len(payload_data)))
                if not chunk:
                    break
                payload_data += chunk
            
            return header_data + payload_data
            
        except Exception as e:
            logger.error(f"Error receiving data: {e}")
            return None
    
    def stop(self):
        logger.info("Stopping server...")
        self.running = False
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass
        logger.info("Server stopped")

def main():
    port = None
    reset_db = True
    
    for arg in sys.argv[1:]:
        if arg in ['--no-reset', '--keep', '-k']:
            reset_db = False
            logger.info("Keeping existing database")
        elif arg in ['--reset', '-r', '--clean']:
            reset_db = True
            logger.info("Database reset requested")
        else:
            try:
                port = int(arg)
            except ValueError:
                logger.error(f"Invalid argument: {arg}")
                logger.info("Usage: python3 server.py [port] [--reset|--no-reset]")
                sys.exit(1)
    
    if reset_db:
        db_file = 'defensive.db'
        if os.path.exists(db_file):
            os.remove(db_file)
            logger.info(f"Database deleted: {db_file} (fresh start)")
        else:
            logger.info("Starting with fresh database")
    
    server = MessageUServer(port)
    
    try:
        server.start()
    except KeyboardInterrupt:
        logger.info("\nShutdown requested by user")
        server.stop()

if __name__ == '__main__':
    main()

