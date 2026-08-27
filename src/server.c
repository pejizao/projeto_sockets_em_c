/*
 * Laboratório didático de sockets TCP.
 *
 * Este servidor aceita somente conexões locais (127.0.0.1) e executa uma
 * lista pequena de comandos demonstrativos. Ele NÃO é um backdoor real e
 * não deve ser alterado para uso em máquinas ou redes sem autorização.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
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

    while (sent < length)
    {
        ssize_t result = send(fd, data + sent, length - sent, 0);
        if (result <= 0)
            return -1;
        sent += (size_t)result;
    }

    return 0;
}

static int execute_shell_command(const char *command, char *output, size_t output_size)
{
    FILE *process;
    size_t total = 0;

    /*
     * Esta é a parte que demonstra a interação com o shell.
     * O projeto está preso ao 127.0.0.1 e é somente para laboratório.
     */
    process = popen(command, "r");
    if (process == NULL)
        return snprintf(output, output_size, "Erro ao abrir o shell.\n");

    while (total < output_size - 1) {
        size_t read_now = fread(output + total, 1, output_size - 1 - total, process);
        total += read_now;
        if (read_now == 0)
            break;
    }

    output[total] = '\0';
    pclose(process);
    return (int)total;
}

static void handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];
    char output[BUFFER_SIZE];
    ssize_t received;
    size_t position;

    send_all(client_fd,
             "StockFlow Security Lab\nComandos: help, pwd, date, uname, ls, quit\n> ",
             strlen("StockFlow Security Lab\nComandos: help, pwd, date, uname, ls, quit\n> "));

    while (1) {
        position = 0;
        while (position < sizeof(buffer) - 1) {
            received = recv(client_fd, buffer + position, 1, 0);
            if (received <= 0)
                return;
            if (buffer[position] == '\n')
                break;
            position++;
        }

        buffer[position] = '\0';
        buffer[strcspn(buffer, "\r")] = '\0';

        if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "exit") == 0) {
            const char *message = "Conexao encerrada pelo cliente.\n";
            send_all(client_fd, message, strlen(message));
            break;
        }

        if (strcmp(buffer, "help") == 0) {
            const char *help = "Digite um comando Linux para o laboratorio ou quit para sair.\n";
            send_all(client_fd, help, strlen(help));
        } else {
            int size = execute_shell_command(buffer, output, sizeof(output));
            if (size > 0)
                send_all(client_fd, output, (size_t)size);
        }

        send_all(client_fd, "> ", 2);
    }
}

int main(void)
{
    int server_fd;
    int client_fd;
    int option = 1;
    struct sockaddr_in address;
    socklen_t address_length = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &address.sin_addr);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 1) == -1) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Servidor ouvindo somente em %s:%d\n", SERVER_IP, SERVER_PORT);
    printf("Aguardando cliente...\n");

    client_fd = accept(server_fd, (struct sockaddr *)&address, &address_length);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Cliente conectado.\n");
    handle_client(client_fd);
    close(client_fd);
    close(server_fd);
    printf("Servidor encerrado.\n");
    return EXIT_SUCCESS;
}
