# Guia de estudo do laboratório

## 1. O que a atividade ensina

A atividade combina quatro assuntos: comunicação TCP, programação cliente/servidor, criação de processos no Linux e segurança da informação. O objetivo não é apenas fazer o programa funcionar, mas compreender o caminho dos dados e reconhecer por que executar entrada de rede como comando é perigoso.

## 2. Ordem recomendada para estudar

Comece pelo cliente e servidor sem shell. Primeiro, faça o servidor abrir uma porta local e responder uma mensagem fixa. Depois, faça o cliente conectar e exibir essa mensagem. Em seguida, troque a mensagem fixa por uma linha digitada pelo usuário. Só depois analise a função que executa o comando e devolve a saída.

Essa ordem separa os problemas. Se algo falhar na primeira etapa, a causa provavelmente está no socket. Se a conexão funcionar, mas a resposta estiver incorreta, a análise deve se concentrar no envio e recebimento. Se a resposta do comando estiver vazia, deve-se verificar o processo criado e a leitura do pipe.

## 3. Funções do servidor

`socket()` cria o ponto de comunicação. O servidor usa IPv4 com `AF_INET` e TCP com `SOCK_STREAM`.

`bind()` associa o socket ao endereço local e à porta 5050. O endereço `127.0.0.1` significa que somente a própria máquina pode iniciar a conexão.

`listen()` coloca o servidor em modo de espera. O segundo argumento informa quantas conexões podem aguardar na fila.

`accept()` aceita uma conexão e devolve um novo descritor para conversar com aquele cliente. O socket original continua representando o servidor.

`recv()` lê dados enviados pelo cliente. Como TCP é um fluxo de bytes, o programa precisa definir como identifica o fim de uma mensagem. Neste laboratório, cada comando termina com uma quebra de linha.

`send()` envia dados ao cliente. A função auxiliar `send_all()` repete o envio se necessário, porque uma chamada de `send()` não é obrigada a transmitir todos os bytes solicitados.

`popen()` inicia um comando por meio do shell e permite ler sua saída. É a parte intencionalmente insegura do laboratório, pois uma string recebida pela rede chega ao shell. Em aplicações reais, deve ser evitada com entrada não confiável.

`pclose()` fecha o fluxo do processo e aguarda o término do comando.

`close()` libera os descritores quando a comunicação termina.

## 4. Funções do cliente

O cliente também usa `socket()`, mas não chama `bind()`, `listen()` ou `accept()`. Ele configura o endereço do servidor e chama `connect()`.

Depois da conexão, `recv()` recebe a mensagem inicial e cada saída. A função `receive_until_prompt()` lê até encontrar o prompt `> `, permitindo que o cliente saiba quando a resposta terminou.

`fgets()` lê o comando digitado no terminal. `send_all()` envia a linha ao servidor. Quando a linha começa com `quit` ou `exit`, o cliente interrompe o laço e fecha o socket.

## 5. Como os dados passam pelo sistema

O usuário digita `pwd` no cliente. O cliente acrescenta a quebra de linha e envia os bytes pelo socket TCP. O servidor recebe os bytes até encontrar `\n`, remove a quebra de linha e chama `popen("pwd", "r")`.

O shell executa o comando, e o servidor lê sua saída com `fread()`. Depois, os bytes são enviados de volta ao cliente. O cliente imprime a resposta e mostra o prompt para o próximo comando.

O caminho completo é:

```text
teclado -> fgets -> send -> TCP -> recv -> popen -> shell -> fread -> send -> TCP -> recv -> tela
```

## 6. Bind shell e reverse shell

Neste laboratório, o servidor abre uma porta e espera o cliente. Esse comportamento é chamado, de forma geral, de bind shell: o processo que oferece o shell fica aguardando uma conexão.

No reverse shell, o processo executado na máquina-alvo inicia uma conexão de saída para outra máquina, que fica aguardando. Essa diferença é importante para a defesa, pois conexões de saída inesperadas também podem indicar comprometimento.

A atividade pode ser compreendida usando o modelo local sem expor uma máquina. O conceito de socket e de troca de dados é o mesmo; o que muda é o endereço e o contexto de autorização.

## 7. Por que o código é inseguro

A rede é uma fonte de entrada não confiável. O código recebe texto e o entrega ao shell. Assim, qualquer cliente que consiga conectar poderia tentar executar operações com as permissões do processo servidor.

O laboratório limita a porta ao loopback, mas isso não transforma `popen()` em uma prática segura. A limitação apenas reduz a superfície de exposição para o exercício. O programa também não implementa autenticação forte, autorização, criptografia, sandbox ou controle de privilégios.

## 8. Como transformar o exemplo em algo mais seguro

A primeira melhoria é não executar strings diretamente. Em vez disso, o servidor pode aceitar apenas nomes de operações conhecidos, como `STATUS`, `DATA` e `INFO`, e mapear cada nome para uma função fixa.

A segunda melhoria é executar o processo com usuário sem privilégios e em um ambiente isolado. A terceira é registrar conexão, endereço, horário, comando solicitado e resultado. A quarta é definir timeout e limitar o tamanho das mensagens.

Também é importante fechar a porta quando o programa não estiver em uso, bloquear portas não autorizadas no firewall e monitorar processos que criam shells.

## 9. Perguntas para verificar o aprendizado

1. Qual é a diferença entre o socket do servidor e o socket retornado por `accept()`?
2. Por que `htons()` é usado ao configurar a porta?
3. Por que o servidor precisa saber quando uma mensagem termina?
4. O que acontece se o cliente for encerrado sem enviar `quit`?
5. Por que `popen()` é uma vulnerabilidade quando recebe entrada externa?
6. Qual é a diferença entre escutar em `127.0.0.1` e escutar em todas as interfaces?
7. Como um firewall e um monitor de processos poderiam detectar esse programa?
8. Qual seria uma alternativa segura para comandos fixos?

## 10. Critério de avanço

O aluno pode considerar a primeira etapa concluída quando conseguir explicar, sem copiar o código, o caminho `socket -> bind -> listen -> accept -> recv -> send -> close` no servidor e `socket -> connect -> send -> recv -> close` no cliente.

A segunda etapa é compreender o processo filho ou o shell. A terceira é identificar a vulnerabilidade e propor uma correção. A solução mais importante da atividade não é apenas executar comandos, mas saber justificar por que essa prática deve ser proibida ou fortemente controlada em um ambiente real.
