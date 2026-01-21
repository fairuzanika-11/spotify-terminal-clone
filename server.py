import socket
import time
import sys

# Usage: python3 server.py song.wav
PORT = 8080
CHUNK_SIZE = 4096

if len(sys.argv) < 2:
    print("Usage: python3 server.py <wav_file>")
    exit()

with open(sys.argv[1], 'rb') as f:
    # EVERYTHING below here is indented because it relies on the open file 'f'
    
    # Skip the WAV header (44 bytes)
    f.seek(44) 
    
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # This allows you to restart the server immediately without "Address already in use" errors
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    server_socket.bind(('0.0.0.0', PORT))
    server_socket.listen(1)
    print(f"Spotify Server running on port {PORT}...")

    conn, addr = server_socket.accept()
    print(f"User connected from {addr}")

    data = f.read(CHUNK_SIZE)
    while data:
        conn.sendall(data)
        data = f.read(CHUNK_SIZE)
        # Adjust this sleep to control stream speed (0.002 is roughly real-time for 44.1kHz)
        time.sleep(0.002) 

    conn.close()
    print("Song finished.")
