# Laboratório de Sockets TCP em C — Shell Controlado

## Aviso de segurança

Este projeto foi criado exclusivamente para estudo em ambiente local. O servidor aceita conexões somente em `127.0.0.1` e não possui persistência, evasão, criptografia própria ou mecanismo de inicialização automática. Não altere o endereço para expor o programa à rede nem utilize o código em máquinas de terceiros.

## 1. Objetivo

O laboratório demonstra uma conexão TCP cliente/servidor usando sockets em C. O cliente conecta ao servidor, envia uma string digitada pelo aluno e recebe a saída produzida pelo shell local. O comando `quit` ou `exit` encerra a sessão de maneira limpa.

A execução no shell foi mantida para representar o conceito solicitado na atividade. Como esse comportamento é perigoso em um sistema real, a demonstração deve ser feita somente no loopback e em uma máquina de testes.

## 2. Estrutura


```text
lab-sockets-c/
├── src/
│   ├── server.c
│   └── client.c
├── .gitignore
├── LICENSE
├── README.md
```

## 3. Pré-requisitos

O exemplo foi escrito para Linux ou outro sistema POSIX com compilador GCC. São necessários `gcc`, `make` opcional e um terminal para executar o servidor e outro para executar o cliente.

## 4. Compilação

Na pasta do projeto, execute:

```bash
gcc -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L src/server.c -o server
gcc -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L src/client.c -o client
```

O parâmetro `-Wall -Wextra` solicita avisos úteis do compilador. A macro `_POSIX_C_SOURCE` disponibiliza as interfaces POSIX utilizadas pelo exemplo.

## 5. Compilação simplificada

Também é possível compilar com o Makefile:

```bash
make
```

Para remover os executáveis compilados:

```bash
make clean
```

## 6. Execução

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

## 11. Limitações didáticas

O programa aceita comandos arbitrários porque essa é uma parte do comportamento solicitado para demonstrar a interação com o shell. Essa característica é intencionalmente insegura e não deve ser usada como modelo de desenvolvimento de software de produção.

Para uma versão mais segura, o aluno pode substituir `popen()` por uma lista de comandos permitidos, como `pwd`, `date` e `uname`, utilizando `execv()` com argumentos fixos. Também pode adicionar autenticação para o laboratório, registro de eventos e validação do tamanho das mensagens.

## 12. Sugestão de evolução

Uma sequência de evolução compreensível para o curso é começar com mensagens simples, depois implementar a conexão TCP, em seguida transmitir comandos, capturar a saída e, finalmente, adicionar o encerramento. Depois disso, o aluno pode comparar a versão insegura com uma versão segura que não executa strings diretamente.

Outras extensões acadêmicas possíveis são implementar um protocolo de mensagens com tamanho definido, adicionar timeout, tratar desconexão abrupta, criar testes de unidade para validação de comandos e comparar bind shell com reverse shell apenas conceitualmente. Não é recomendado expor o laboratório à rede ou adicionar persistência.

## 13. Versionamento Git

Exemplo de sequência de commits:

```bash
git init
git add README.md .gitignore LICENSE src/
git commit -m "estrutura inicial do laboratorio"

git add src/server.c
git commit -m "implementa servidor TCP local"

git add src/client.c
git commit -m "implementa cliente TCP local"

git add README.md
git commit -m "documenta teoria e demonstracao"
```

Antes de publicar o repositório, substitua os campos de identificação abaixo:

| Campo | Preencher |
|---|---|
| Nome do aluno 1 | ______________________________ |
| Matrícula do aluno 1 | ______________________________ |
| Nome do aluno 2 | ______________________________ |
| Matrícula do aluno 2 | ______________________________ |
| Instituição | ______________________________ |
| Professor | ______________________________ |

## 14. Conclusão

O laboratório apresenta o ciclo completo de uma comunicação TCP local: criação do socket, associação a um endereço, escuta, conexão, troca de mensagens, execução do processo local e encerramento. A parte mais importante da análise de segurança é perceber que transformar entrada de rede em comando de shell cria uma superfície de ataque grave. Por isso, o código deve ser usado somente em ambiente controlado e acompanhado de medidas de detecção e mitigação.
