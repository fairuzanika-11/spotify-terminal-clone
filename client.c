#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <SDL.h>

#define BUFFER_SIZE 1024 * 1024 // 1MB Ring Buffer
#define PORT 8080

// --- THE SHARED MEMORY (CRITICAL SECTION) ---
typedef struct {
    char data[BUFFER_SIZE];
    int write_pos;
    int read_pos;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} RingBuffer;

RingBuffer rb;
int is_running = 1;

// --- AUDIO SETTINGS ---
// Standard WAV settings: 44.1kHz, 16-bit, Stereo
#define SAMPLE_RATE 44100
#define CHANNELS 2
#define SAMPLES 1024 

// --- THREAD 1: THE DOWNLOADER (PRODUCER) ---
void* network_thread(void* arg) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char chunk[4096];

    // 1. Connect to Server
    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return NULL;
    }

    // 2. Read Loop
    while (is_running) {
        int bytes_read = read(sock, chunk, sizeof(chunk));
        if (bytes_read <= 0) break;

        // LOCK MEMORY
        pthread_mutex_lock(&rb.lock);

        // If buffer full, wait
        while (rb.count + bytes_read > BUFFER_SIZE) {
            pthread_cond_wait(&rb.not_full, &rb.lock);
        }

        // Write to Ring Buffer
        for (int i = 0; i < bytes_read; i++) {
            rb.data[rb.write_pos] = chunk[i];
            rb.write_pos = (rb.write_pos + 1) % BUFFER_SIZE;
            rb.count++;
        }

        // Signal audio thread that data is ready
        pthread_cond_signal(&rb.not_empty);
        pthread_mutex_unlock(&rb.lock);
    }
    
    close(sock);
    return NULL;
}

// --- THREAD 2: THE AUDIO DEVICE (CONSUMER) ---
// SDL calls this function automatically when it needs more audio data
void audio_callback(void* userdata, Uint8* stream, int len) {
    pthread_mutex_lock(&rb.lock);

    if (rb.count < len) {
        // Not enough data (Buffering...)
        // Fill with silence
        memset(stream, 0, len); 
    } else {
        // Copy from Ring Buffer to Audio Device
        for (int i = 0; i < len; i++) {
            stream[i] = rb.data[rb.read_pos];
            rb.read_pos = (rb.read_pos + 1) % BUFFER_SIZE;
            rb.count--;
        }
        // Signal network thread that we have space
        pthread_cond_signal(&rb.not_full);
    }

    pthread_mutex_unlock(&rb.lock);
}

int main() {
    // 1. Init Pthreads primitives
    pthread_mutex_init(&rb.lock, NULL);
    pthread_cond_init(&rb.not_empty, NULL);
    pthread_cond_init(&rb.not_full, NULL);
    rb.write_pos = 0;
    rb.read_pos = 0;
    rb.count = 0;

    // 2. Init SDL Audio
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = CHANNELS;
    want.samples = SAMPLES;
    want.callback = audio_callback;

    SDL_OpenAudio(&want, &have);
    SDL_PauseAudio(0); // Start playing

    // 3. Start Network Thread
    pthread_t net_tid;
    pthread_create(&net_tid, NULL, network_thread, NULL);

    // 4. Main UI Loop (Simple version)
    printf("Playing... Press Enter to quit.\n");
    getchar();

    // Cleanup
    is_running = 0;
    pthread_join(net_tid, NULL);
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}
