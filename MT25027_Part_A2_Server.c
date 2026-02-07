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

    struct iovec iov[8];
    for (int i = 0; i < 8; i++) {
        iov[i].iov_base = msg.fields[i];
        iov[i].iov_len = field_size;
    }

    struct msghdr message;
    memset(&message, 0, sizeof(message));
    message.msg_iov = iov;
    message.msg_iovlen = 8;

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration) {
        // One copy eliminated: No user-space linearization.
        // gather-send directly into kernel buffer.
        ssize_t sent = sendmsg(client_socket, &message, 0);
        if (sent <= 0) break;
    }

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

    printf("Server (One-Copy) listening on port %d...\n", PORT);

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
