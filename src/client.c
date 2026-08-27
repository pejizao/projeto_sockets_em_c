/*
 * Cliente do laboratório didático de sockets TCP.
 * Conecta exclusivamente ao servidor local 127.0.0.1:5050.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5050
#define BUFFER_SIZE 4096

static int send_all(int fd, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t result = send(fd, data + sent, length - sent, 0);
        if (result <= 0)
            return -1;
        sent += (size_t)result;
    }

    return 0;
}

static int receive_until_prompt(int fd, char *buffer, size_t buffer_size)
{
    size_t position = 0;
    char previous = '\0';
    char current;

    while (position < buffer_size - 1) {
        ssize_t received = recv(fd, &current, 1, 0);
        if (received <= 0)
            return -1;

        buffer[position++] = current;
        buffer[position] = '\0';

        if (previous == '>' && current == ' ')
            return 0;
        previous = current;
    }

    buffer[buffer_size - 1] = '\0';
    return 0;
}

int main(void)
{
    int client_fd;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("connect");
        close(client_fd);
        return EXIT_FAILURE;
    }

    while (receive_until_prompt(client_fd, buffer, sizeof(buffer)) == 0) {
        printf("%s", buffer);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;

        if (send_all(client_fd, buffer, strlen(buffer)) == -1)
            break;

        if (strncmp(buffer, "quit", 4) == 0 || strncmp(buffer, "exit", 4) == 0)
            break;
    }

    close(client_fd);
    return EXIT_SUCCESS;
}
