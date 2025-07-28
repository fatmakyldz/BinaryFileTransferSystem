#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <xxhash.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHUNK_SIZE (1024 * 1024 * 1024) // 64 MB
#define DEFAULT_FILE_PATH "/Users/fatmaakyildiz/testfile.bin"

double get_elapsed_seconds(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

void print_hash_ull(unsigned long long hash) {
    printf("%016llx", hash);
}

int main(int argc, char* argv[]) {
    const char* file_path;
    if (argc >= 2) {
        file_path = argv[1];
    } else {
        printf("⚠️  Dosya yolu verilmedi, varsayılan dosya kullanılacak:\n📍 %s\n", DEFAULT_FILE_PATH);
        file_path = DEFAULT_FILE_PATH;
    }

    FILE* file = fopen(file_path, "rb");
    if (!file) {
        perror("Dosya açılamadı");
        return EXIT_FAILURE;
    }

    struct stat st;
    off_t fileSize = 0;
    if (stat(file_path, &st) == 0) {
        fileSize = st.st_size;
        printf("📂 Gönderilecek dosya boyutu: %.2f MB (%.2f GB)\n",
            fileSize / (1024.0 * 1024.0), fileSize / (1024.0 * 1024.0 * 1024.0));
    } else {
        perror("Dosya boyutu okunamadı");
    }

    void* context = zmq_ctx_new();
    void* sender = zmq_socket(context, ZMQ_PUSH);
    int hwm = 1000;
    zmq_setsockopt(sender, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    if (zmq_bind(sender, "tcp://*:5555") != 0) {
        perror("ZeroMQ bind hatası");
        return EXIT_FAILURE;
    }

    unsigned char* buffer = malloc(CHUNK_SIZE);
    if (!buffer) {
        fprintf(stderr, "❌ Bellek ayrılamadı\n");
        return EXIT_FAILURE;
    }

    size_t bytesRead;
    int chunkIndex = 0;
    size_t totalBytesSent = 0;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while ((bytesRead = fread(buffer, 1, CHUNK_SIZE, file)) > 0) {
        struct timespec hash_start, hash_end;
        clock_gettime(CLOCK_MONOTONIC, &hash_start);

        unsigned long long hash = XXH64(buffer, bytesRead, 0);

        clock_gettime(CLOCK_MONOTONIC, &hash_end);
        double hash_duration = get_elapsed_seconds(hash_start, hash_end) * 1000.0;

        zmq_msg_t chunkMsg, hashMsg;
        zmq_msg_init_size(&chunkMsg, bytesRead);
        memcpy(zmq_msg_data(&chunkMsg), buffer, bytesRead);
        zmq_msg_send(&chunkMsg, sender, ZMQ_SNDMORE);
        zmq_msg_close(&chunkMsg);

        zmq_msg_init_size(&hashMsg, sizeof(hash));
        memcpy(zmq_msg_data(&hashMsg), &hash, sizeof(hash));
        zmq_msg_send(&hashMsg, sender, 0);
        zmq_msg_close(&hashMsg);

        printf("📦 Chunk %d gönderildi (%.2f MB) - ⏱️ Hash süresi: %.2f ms - 🔐 Hash: ", chunkIndex, bytesRead / (1024.0 * 1024.0), hash_duration);
        print_hash_ull(hash);
        printf("\n");

        totalBytesSent += bytesRead;
        chunkIndex++;
    }

    const char* endSignal = "END";
    zmq_send(sender, endSignal, strlen(endSignal), 0);
    printf("🛑 END sinyali gönderildi.\n");

    free(buffer);
    fclose(file);
    zmq_close(sender);
    zmq_ctx_term(context);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = get_elapsed_seconds(start_time, end_time);

    printf("✅ Gönderim tamamlandı. Toplam %d chunk, %.2f MB gönderildi.\n",
        chunkIndex, totalBytesSent / (1024.0 * 1024.0));
    printf("📂 Toplam dosya boyutu: %.2f MB (%.2f GB)\n",
        fileSize / (1024.0 * 1024.0), fileSize / (1024.0 * 1024.0 * 1024.0));
    printf("⏱ Toplam süre: %.2f saniye\n", elapsed);

    return EXIT_SUCCESS;
}
