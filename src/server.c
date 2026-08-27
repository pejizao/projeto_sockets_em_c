/*
 * Laboratório didático de sockets TCP.
 *
 * Este servidor aceita somente conexões locais (127.0.0.1) e executa comandos
 * enviados pelo cliente para demonstrar a interação com o shell.
 * NÃO use este exemplo em máquinas ou redes sem autorização.
 */

#include <arpa/inet.h>
#include <errno.h>
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
        ssize_t resultado = send(descritor, dados + enviados, tamanho - enviados, 0);
        if (resultado <= 0)
            return -1;
        enviados += (size_t)resultado;
    }

    return 0;
}

/* Executa o texto recebido no shell local e captura a saída produzida. */
static int executar_comando_shell(const char *comando, char *saida, size_t tamanho_saida)
{
    FILE *processo;
    size_t total_lido = 0;

    /*
     * Esta chamada existe para demonstrar o conceito da atividade.
     * Em uma aplicação real, não se deve passar entrada de rede diretamente
     * ao shell sem validação, autorização e isolamento.
     */
    processo = popen(comando, "r");
    if (processo == NULL)
        return snprintf(saida, tamanho_saida, "Erro ao abrir o shell.\n");

    while (total_lido < tamanho_saida - 1) {
        size_t lidos_agora = fread(saida + total_lido, 1,
                                   tamanho_saida - 1 - total_lido, processo);
        total_lido += lidos_agora;
        if (lidos_agora == 0)
            break;
    }

    saida[total_lido] = '\0';
    pclose(processo);
    return (int)total_lido;
}

/* Recebe um comando por linha e devolve a saída correspondente ao cliente. */
static void atender_cliente(int descritor_cliente)
{
    char buffer[TAMANHO_BUFFER];
    char saida[TAMANHO_BUFFER];
    ssize_t recebido;
    size_t posicao;
    const char *mensagem_inicial =
        "Laboratorio de Seguranca com Sockets TCP\n"
        "Digite um comando Linux ou quit para sair.\n> ";

    enviar_todos(descritor_cliente, mensagem_inicial, strlen(mensagem_inicial));

    while (1) {
        posicao = 0;

        /* TCP e um fluxo de bytes; aqui usamos '\n' como fim da mensagem. */
        while (posicao < sizeof(buffer) - 1) {
            recebido = recv(descritor_cliente, buffer + posicao, 1, 0);
            if (recebido <= 0)
                return;
            if (buffer[posicao] == '\n')
                break;
            posicao++;
        }

        buffer[posicao] = '\0';
        buffer[strcspn(buffer, "\r")] = '\0';

        if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "exit") == 0) {
            const char *mensagem = "Conexao encerrada pelo cliente.\n";
            enviar_todos(descritor_cliente, mensagem, strlen(mensagem));
            break;
        }

        if (strcmp(buffer, "help") == 0) {
            const char *ajuda =
                "Exemplos: pwd, date, uname -a, ls, help e quit.\n";
            enviar_todos(descritor_cliente, ajuda, strlen(ajuda));
        } else {
            int tamanho_saida = executar_comando_shell(
                buffer, saida, sizeof(saida));
            if (tamanho_saida > 0)
                enviar_todos(descritor_cliente, saida, (size_t)tamanho_saida);
        }

        enviar_todos(descritor_cliente, "> ", 2);
    }
}

int main(void)
{
    int descritor_servidor;
    int descritor_cliente;
    int reutilizar_endereco = 1;
    struct sockaddr_in endereco;
    socklen_t tamanho_endereco = sizeof(endereco);

    descritor_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (descritor_servidor == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    setsockopt(descritor_servidor, SOL_SOCKET, SO_REUSEADDR,
               &reutilizar_endereco, sizeof(reutilizar_endereco));

    memset(&endereco, 0, sizeof(endereco));
    endereco.sin_family = AF_INET;
    endereco.sin_port = htons(PORTA_SERVIDOR);
    inet_pton(AF_INET, IP_SERVIDOR, &endereco.sin_addr);

    if (bind(descritor_servidor, (struct sockaddr *)&endereco,
             sizeof(endereco)) == -1) {
        perror("bind");
        close(descritor_servidor);
        return EXIT_FAILURE;
    }

    if (listen(descritor_servidor, 1) == -1) {
        perror("listen");
        close(descritor_servidor);
        return EXIT_FAILURE;
    }

    printf("Servidor ouvindo somente em %s:%d\n",
           IP_SERVIDOR, PORTA_SERVIDOR);
    printf("Aguardando cliente...\n");

    descritor_cliente = accept(descritor_servidor,
                               (struct sockaddr *)&endereco,
                               &tamanho_endereco);
    if (descritor_cliente == -1) {
        perror("accept");
        close(descritor_servidor);
        return EXIT_FAILURE;
    }

    printf("Cliente conectado.\n");
    atender_cliente(descritor_cliente);
    close(descritor_cliente);
    close(descritor_servidor);
    printf("Servidor encerrado.\n");
    return EXIT_SUCCESS;
}
