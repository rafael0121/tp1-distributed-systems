# Trabalho Prático 1 Sistemas distribuídos

## Grupo 7

* Arthur Gonçalves Ayres Lanna 
* Marcus Leandro Gomes Campos Oliveira 
* Rafael Ramos de Andrade
* Samuel Clementino de Oliveira Rocha


> Obs.: O código e comentários foram escritos em Inglês.

# Setup

## Dependências

Para compilar e executar o código é necessário ter instalado no sistema os pacotes:
### Arch

> sudo pacman -S grpc protobuf cmake gcc

### Ubuntu
> ...

## Instalação e execução

Para compilar o projeto é necessário construir o ambiente de compilação, para isso utilizaremos o programa *cmake*.

A partir da pasta raiz do projeto, execute:
> ./setup.sh

Agora, para executar os processos, basta configurar o arquivo run.sh.

Altere as portas dos clientes e do servidor para uma porta livre no sistema, o processo não executará se a porta estiver ocupada:
```
# 1. Setup clients network address
CLIENT_1_ADDRESS=0.0.0.0:55001
CLIENT_2_ADDRESS=0.0.0.0:55002
CLIENT_3_ADDRESS=0.0.0.0:55003

# 2. Setup server network
SERVER_ADDRESS=0.0.0.0:55000
```


Configure o comando da forma que seu terminal utiliza para executar os binários. 
```
# 3. Setup 
EXEC="alacritty -e bash -c"
```


É possível aumentar a quantidade de clientes que são executados basta adicionar mais clientes seguindo o padrão na seção **#1** e **#5** do script .

Agora basta executar a partir do diretório raiz o comando: 
> ./run.sh

Se tudo ocorrer bem os programas vão executar em terminais diferentes, mostrando o estado em tempo real de cada um.

# Protocolo GRPC

> ./proto/myproto

O protocolo consiste de 2 serviços principais, *PrintingService* e *MutualExclusionService*.

* *PrintingService* é o serviço da impressora (server) com um único método chamado *SendToPrinter* utilizado para enviar uma mensagem que será imprimida. A implementação não possui nenhum mecanismo de exclusão mútua, sendo responsabilidade dos processos coordenarem a utilização do serviço.
* MutualExclusionService é o serviço utilizado pelos processos para controlar o acesso ao recurso, baseados no algorítmo de Ricart-Agrawala, os métodos desse serviço são, *RequestAccess*, utilizado para solicitar uma permissão de acesso a região para os outros processos e *ReleaseAccess*, utilizado pelo processo que estava imprimindo na região, para informar os outros processos que a impressora está livre para ser utilizada.

O protocolo também define as mensagens que serão passada através desse métodos, além dos atributos necessários para realizar as operações dos métodos as menagens também carregam uma importante variável. A variável *lamport_timestamp*,  que armazena o valor do relógio lógico do processo.

# Lamport

> ./lamport

Na pasta **lamport** está a implementação dos relógios lógicos de Lamport, utilizado para sincronizar as mensagens trocadas entre os processos, definindo assim uma ordem de eventos. 

A class Lamport implementa o método de *updateTimestamp*, que atualiza o valor da variável de relógio somando um ao valor atual ou, caso seja passado uma valor inteiro como parâmetro, é comparado o maior valor entre o valor passado por parâmetro e o valor do tempo atual, armazenando o maior valor mais um na variável *curTimestamp*.

A variável *curTimestamp* é protegida contra concorrência através do mutex *mt_curTimestamp*.

# Server (Servidor ou Impressora)

> ./server/src

O servidor é responsável  por imprimir as mensagens enviados pelo processo através do método *SendToPrinter* do serviço *PrintingService*.

# Client (Cliente)

> ./client/src

Os processos clientes utilizam a impressora de forma coordenada, peguntando aos outros processo se podem acessar a impressora.

Os clientes possuem 2 threads principais, a thread do servidor GRPC, construído na função *main* e a thread principal que executa a função *wanna_print*. Essa função é responsável por aguardar um tempo aleatório, e então solicita, aos outros processos, permissão de acesso ao servidor de impressora. 

A função *wanna_print*, realiza uma loop infinito, onde é gerado um valor aleatório, o processo aguarde o tempo equivalente. Após essa etapa o processo altera seu estado para *WAITING* e solicita aos outros processos acesso ao servidor através da função *request_access*, nesse momento, caso os processos permitam o acesso, é desbloqueado o mutex que trava a função de continuar a execução, mudando assim seu estado para *HOLDING* e executando a função *send_print* que realiza o envio da mensagem para o servidor de impressão. Após realizar a chamada do método e obter o retorno do servidor, o estado é alterado para RELEASED e é feito um broadcast para todos os outros processos utilizando a função *release_access*. Liberando o acesso a impressora para os outros processos.

Outra forma de liberar a execução do *wanna_print* acontece quando o processo está no estado *WAITING* e os outros processos liberam acessos para o processo através do método *ReleaseAccess*. 

Para gerenciar as duas formas de liberação, foi criado a classe *NeighborsReleased* que manipula um vetor que armazena o ID dos processos vizinhos que liberaram acesso a impressora. A classe também utiliza *mutex* para controle do acesso ao vetor, garantindo que não haja concorrência entre as threads do cliente.

# Testes

Os vários testes feitos durante o desenvolvimento contribuíram para identificar bugs na implementação. No entanto, mesmo realizando os testes, é extremamente difícil à identificação de bugs que dependem de eventos específicos acontecerem em sequência, evidenciando a complexidade do desenvolvimento desses sistemas.