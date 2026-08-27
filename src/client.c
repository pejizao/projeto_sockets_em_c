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

#define IP_SERVIDOR "127.0.0.1"
#define PORTA_SERVIDOR 5050
#define TAMANHO_BUFFER 4096

/* Garante que todos os bytes sejam enviados ao socket. */
static int enviar_todos(int descritor, const char *dados, size_t tamanho)
{
    size_t enviados = 0;

    while (enviados < tamanho) {
        ssize_t resultado = send(descritor, dados + enviados,
                                 tamanho - enviados, 0);
        if (resultado <= 0)
            return -1;
        enviados += (size_t)resultado;
    }

    return 0;
}

/* Lê a resposta até encontrar o prompt enviado pelo servidor. */
static int receber_ate_prompt(int descritor, char *buffer, size_t tamanho_buffer)
{
    size_t posicao = 0;
    char caractere_anterior = '\0';
    char caractere_atual;

    while (posicao < tamanho_buffer - 1) {
        ssize_t recebido = recv(descritor, &caractere_atual, 1, 0);
        if (recebido <= 0)
            return -1;

        buffer[posicao++] = caractere_atual;
        buffer[posicao] = '\0';

        if (caractere_anterior == '>' && caractere_atual == ' ')
            return 0;
        caractere_anterior = caractere_atual;
    }

    buffer[tamanho_buffer - 1] = '\0';
    return 0;
}

int main(void)
{
    int descritor_cliente;
    struct sockaddr_in endereco_servidor;
    char buffer[TAMANHO_BUFFER];

    descritor_cliente = socket(AF_INET, SOCK_STREAM, 0);
    if (descritor_cliente == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&endereco_servidor, 0, sizeof(endereco_servidor));
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_port = htons(PORTA_SERVIDOR);
    inet_pton(AF_INET, IP_SERVIDOR, &endereco_servidor.sin_addr);

    if (connect(descritor_cliente,
                (struct sockaddr *)&endereco_servidor,
                sizeof(endereco_servidor)) == -1) {
        perror("connect");
        close(descritor_cliente);
        return EXIT_FAILURE;
    }

    while (receber_ate_prompt(descritor_cliente,
                              buffer, sizeof(buffer)) == 0) {
        printf("%s", buffer);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;

        if (enviar_todos(descritor_cliente, buffer, strlen(buffer)) == -1)
            break;

        if (strncmp(buffer, "quit", 4) == 0 ||
            strncmp(buffer, "exit", 4) == 0)
            break;
    }

    close(descritor_cliente);
    return EXIT_SUCCESS;
}
