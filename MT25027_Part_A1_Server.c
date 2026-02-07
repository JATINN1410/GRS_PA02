// MT25027

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080

struct message_struct {
    char *fields[8];
};

typedef struct {
    int client_socket;
    int msg_size;
    int duration;
} thread_data_t;

void *handle_client(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int client_socket = data->client_socket;
    int total_msg_size = data->msg_size;
    int duration = data->duration;
    int field_size = total_msg_size / 8;

    struct message_struct msg;
    for (int i = 0; i < 8; i++) {
        msg.fields[i] = (char *)malloc(field_size);
        memset(msg.fields[i], 'A' + i, field_size);
    }

    char *linear_buffer = (char *)malloc(total_msg_size);
    
    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration) {
        // Copy 1: User-space linearization (Manual copy)
        for (int i = 0; i < 8; i++) {
            memcpy(linear_buffer + (i * field_size), msg.fields[i], field_size);
        }

        // Copy 2: send() syscall (Kernel copy)
        int sent = 0;
        while (sent < total_msg_size) {
            int ret = send(client_socket, linear_buffer + sent, total_msg_size - sent, 0);
            if (ret <= 0) break;
            sent += ret;
        }
        if (sent <= 0) break;
    }

    free(linear_buffer);
    for (int i = 0; i < 8; i++) {
        free(msg.fields[i]);
    }
    close(client_socket);
    free(data);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <msg_size> <threads> <duration>\n", argv[0]);
        exit(1);
    }

    int msg_size = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    int duration = atoi(argv[3]);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 100) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d with msg_size %d, threads %d, duration %d\n", PORT, msg_size, num_threads, duration);

    // To handle fixed thread count as requested by param
    // But server usually accepts clients dynamically. 

    
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        thread_data_t *t_data = malloc(sizeof(thread_data_t));
        t_data->client_socket = new_socket;
        t_data->msg_size = msg_size;
        t_data->duration = duration;
        pthread_create(&threads[i], NULL, handle_client, t_data);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    close(server_fd);
    free(threads);
    return 0;
}
