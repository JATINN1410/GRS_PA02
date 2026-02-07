// MT25027

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define PORT 8080

typedef struct {
    char *ip;
    int msg_size;
    int duration;
} thread_data_t;

void *client_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *buffer = malloc(data->msg_size);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, data->ip, &serv_addr.sin_addr) <= 0) {
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return NULL;
    }

    long total_received = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        int valread = recv(sock, buffer, data->msg_size, MSG_WAITALL);
        if (valread <= 0) break;
        total_received += valread;
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed >= data->duration) break;
    }

    double final_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (total_received * 8.0) / (final_elapsed * 1e9); // Gbps
    double latency = (final_elapsed * 1e6) / (total_received / data->msg_size); // us per msg

    printf("Throughput: %.4f Gbps, Latency: %.2f us\n", throughput, latency);

    close(sock);
    free(buffer);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <ip> <msg_size> <threads> <duration>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[1];
    int msg_size = atoi(argv[2]);
    int num_threads = atoi(argv[3]);
    int duration = atoi(argv[4]);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t *t_data = malloc(num_threads * sizeof(thread_data_t));

    for (int i = 0; i < num_threads; i++) {
        t_data[i].ip = ip;
        t_data[i].msg_size = msg_size;
        t_data[i].duration = duration;
        pthread_create(&threads[i], NULL, client_thread, &t_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(t_data);
    return 0;
}
