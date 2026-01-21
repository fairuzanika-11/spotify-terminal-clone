# 🎵 Terminal Spotify Clone (C + Python)

A terminal-based music streaming application built from scratch using **C (Client)** and **Python (Server)**. 

This project demonstrates core **Operating Systems** concepts including:
- **Multithreading** (pthread)
- **Synchronization** (Mutexes & Condition Variables)
- **Inter-Process Communication** (TCP Sockets)
- **Memory Management** (Ring Buffers)
- **Audio I/O** (SDL2)



##  How It Works
The system follows the **Producer-Consumer** architecture:
1.  **Server (Python):** Reads a `.wav` file and streams raw bytes over a TCP socket.
2.  **Client (C):**
    - **Network Thread (Producer):** Downloads data and fills a **Ring Buffer**.
    - **Audio Thread (Consumer):** Reads from the buffer and plays audio using SDL2.
    - **Mutex/CondVars:** Ensures thread safety so data is never corrupted.

## 
Prerequisites
You need **Mac/Linux** to run this (Windows requires WSL).

### Install Dependencies (macOS)
```bash
# Install SDL2 for audio
brew install sdl2

# Ensure you have Python 3
python3 --version