| Nome do aluno | Enzo Mamede Santos |
| Matrícula do aluno | 202321411 |
| Professor | Tiago Marcelino |

# Laboratório de Sockets TCP em C — Shell Controlado

## Aviso de segurança

Este projeto foi criado exclusivamente para estudo em ambiente local. O servidor aceita conexões somente em `127.0.0.1` e não possui persistência, evasão, criptografia própria ou mecanismo de inicialização automática. Não altere o endereço para expor o programa à rede nem utilize o código em máquinas de terceiros.

## 1. Objetivo

O laboratório demonstra uma conexão TCP cliente/servidor usando sockets em C. O cliente conecta ao servidor, envia uma string digitada pelo aluno e recebe a saída produzida pelo shell local. O comando `quit` ou `exit` encerra a sessão de maneira limpa.

A execução no shell foi mantida para representar o conceito solicitado na atividade. Como esse comportamento é perigoso em um sistema real, a demonstração deve ser feita somente no loopback e em uma máquina de testes.

## 2. Estrutura

O arquivo `GUIA-DE-ESTUDO.md` explica a linha de desenvolvimento do laboratório, as funções de socket, o caminho dos dados e as perguntas para revisar o conteúdo.

```text
lab-sockets-c/
├── src/
│   ├── server.c
│   └── client.c
├── .gitignore
├── LICENSE
├── README.md
├── GUIA-DE-ESTUDO.md
```

## 3. Pré-requisitos

O exemplo foi escrito para Linux ou outro sistema POSIX com compilador GCC. São necessários `gcc`, `make` opcional e um terminal para executar o servidor e outro para executar o cliente.

## 4. Compilação simplificada

Também é possível compilar com o Makefile:

```bash
make
```

Para remover os executáveis compilados:

```bash
make clean
```

## 5. Execução

Abra o primeiro terminal e execute:

```bash
./server
```

Abra um segundo terminal e execute:

```bash
./client
```

No cliente, experimente comandos simples, como:

```text
help
pwd
date
uname -a
ls
quit
```

O programa exibirá no cliente a saída produzida pelo shell do servidor. O comando `quit` fecha o socket do cliente; o servidor detecta o encerramento e finaliza o processo.

## 7. Teste automático

O projeto possui um teste de fumaça que compila os programas, inicia o servidor local, executa `pwd` e `uname` pelo cliente e verifica as respostas:

```bash
./tests/smoke_test.sh
```

O resultado esperado é `Smoke test aprovado.`. Esse teste não substitui a compreensão do código; ele apenas confirma que o fluxo básico continua funcionando.

## 8. O que acontece internamente

O servidor cria um socket com `socket(AF_INET, SOCK_STREAM, 0)`. `AF_INET` indica IPv4 e `SOCK_STREAM` indica TCP. Depois, ele configura o endereço `127.0.0.1:5050`, associa o socket com `bind()` e passa a aguardar conexões com `listen()`.

Quando o cliente executa `connect()`, o servidor aceita a conexão com `accept()`. A partir desse momento, os dois processos usam `send()` e `recv()` para trocar bytes. O cliente envia uma linha de texto e o servidor armazena a mensagem em um buffer.

O servidor verifica se recebeu `quit` ou `exit`. Se recebeu, envia uma mensagem final, fecha a conexão com `close()` e encerra. Caso contrário, chama `popen()` para executar a string no shell local e ler a saída. Essa saída é enviada ao cliente com `send()`.

A sequência simplificada é:

```text
cliente: socket -> connect -> send(comando) -> recv(saída)
servidor: socket -> bind -> listen -> accept -> recv(comando) -> popen -> send(saída)
```

## 9. Conceitos de segurança demonstrados

O comportamento estudado se aproxima conceitualmente de um backdoor porque um processo remoto recebe uma entrada e tenta executar comandos no sistema. Em um cenário real, isso permitiria controle indevido da máquina e poderia causar perda de dados, instalação de programas ou exposição de informações.

O exemplo foi limitado ao loopback para reduzir o risco e tornar o processo observável pelo aluno. Mesmo assim, `popen()` não deve receber dados de um cliente não confiável em uma aplicação real. O servidor não possui autenticação, autorização robusta, criptografia, validação completa de entrada ou isolamento em sandbox.

## 10. Como uma equipe Blue Team poderia detectar

A equipe defensiva poderia observar uma porta TCP inesperada, identificar qual processo está escutando nela e verificar a relação entre o processo e o executável. Em Linux, comandos de diagnóstico como `ss -ltnp`, `lsof -iTCP:5050 -sTCP:LISTEN` e `ps aux` podem ajudar durante uma investigação autorizada.

Também poderiam ser analisados logs de firewall, conexões de saída, criação de processos filhos e chamadas anormais ao shell. Um processo que recebe dados pela rede e cria `/bin/sh` ou executa comandos deve ser tratado como um evento de alta prioridade.

As principais medidas de mitigação seriam bloquear portas não autorizadas, restringir o firewall, remover o serviço desconhecido, aplicar o princípio do menor privilégio, utilizar monitoramento de processos, manter o sistema atualizado e revisar regras de execução. Em produção, comandos recebidos não deveriam ser enviados diretamente ao shell; deveria existir uma lista explícita de operações permitidas ou uma API com funções específicas.
