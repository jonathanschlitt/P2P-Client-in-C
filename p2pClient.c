#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>  // Threads
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
// For Client Connect
#include <arpa/inet.h>

#define MESG_SIZE 80
#define SA struct sockaddr  // ==> (SA*)&servaddr

#define MAX_CLIENTS 5
#define MAX_MSG_SIZE 128

// +++++++++++++++++++ Server Logic +++++++++++++++++++
int connectionCount = 0;
int sockfds[50];

int establishedConnections = 0;

// socket initialisation
int createFeed(int _port) {
  int sockfd;
  struct sockaddr_in servaddr, client;
  // socket create and verification

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // make socket reusable for all clients
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <
      0) {
    printf("setsockopt(SO_REUSEABLE) failed");
  }

  // creating socket to listen on
  if (sockfd == -1) {
    fprintf(stderr, "Error: Cannot create socket --- exit\n");
    exit(3);
  } else {
    printf("Socket created.\n");
  }

  bzero(&servaddr, sizeof(servaddr));

  // set up socket
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(_port);

  // Binding socket to any IP
  if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
    fprintf(stderr, "Error: Cannot bind socket --- exit\n");
    exit(4);
  } else {
    printf("Socket bound.\n");
  }

  return sockfd;
}

// Socket listen and connection
int setupConnection(int sockfd) {
  // printf("Verbindung aufbauen\n");
  int len;
  struct sockaddr_in servaddr, client;

  // Server listens
  if ((listen(sockfd, 5)) != 0) {
    fprintf(stderr, "Error: Cannot listen --- exit\n");
    exit(5);
  } else {
    printf("Server listening.\n");
  }
  len = sizeof(client);

  // Accept data packet from client
  int connfd = accept(sockfd, (SA*)&client, &len);
  if (connfd < 0) {
    fprintf(stderr, "Error: Server accept failed --- exit\n");
    exit(6);
  } else {
    printf("Server accept client.\n");
  }

  // fprintf(stderr, "connfd %d\n", connfd);

  return connfd;
}

int firstTouch(int sockfd, int _port) {
  // incremeting connection count, because one client has been arrived
  connectionCount += 1;

  // creating a new socket to do first communication step with client
  // printf("touch %d\n", _port);

  // int sockfd = setupConnection(_port);  // ==> Looking at permannt
  // connection!
  char buffer[MESG_SIZE];
  bzero(buffer, MESG_SIZE);
  read(sockfd, buffer, MESG_SIZE);
  printf("received: %s\n", buffer);

  ((int*)buffer)[0] = _port + 1;
  // fprintf(stderr, "sending Port %d\n", _port + 1);
  write(sockfd, buffer, MESG_SIZE);

  // closing socket for this client ==> going further in main function
  close(sockfd);

  return _port + 1;
}

const int Distance = 'a' - 'A';

// Function designed for chat between client and server.
void changeCase(char* _str) {
  while (*_str != 0) {
    if (*_str >= 'a' && *_str <= 'z') {
      *_str -= Distance;
    } else if (*_str >= 'A' && *_str <= 'Z') {
      *_str += Distance;
    }
    _str++;
  }
}

void* clientLeft(int _sockfd) {
  for (int i = 0; i <= 50; i++) {
    if (sockfds[i] == _sockfd) {
      close(sockfds[i]);
      sockfds[i] = 0;

      connectionCount -= 1;
    }
  }
}

void* serverFunction(void* arg) {
  int sockfd = *((int*)arg);
  // printf("server sockfd: %d\n", sockfd);
  char buffer[MESG_SIZE];

  // printf("Thread started for new client!\n");

  // infinite loop for chat
  while (1) {
    // clear buffer
    bzero(buffer, MESG_SIZE);
    // read the message from client and copy it in buffer
    read(sockfd, buffer, sizeof(buffer));
    // exchange upper-case letters by lower-case letter and vice versa.
    changeCase(buffer);

    // and send that buffer to client
    if (write(sockfd, buffer, sizeof(buffer)) == -1) break;

    // if msg contains "QUIT" then server exit and chat ended.
    if (strncmp("QUIT", buffer, 4) == 0) {
      clientLeft(sockfd);

      // printf("Server Exit...\n");
      // break;
    }
  }
  // return 0;
}

void* runServer() {
  int port = 4567;
  int stdPort = 4567;

  // if (argc > 2) {
  //   fprintf(stderr, "usage: %s [port] --- exit\n", argv[0]);
  //   return 1;
  // }
  // if (argc == 2) {
  //   char* tmp = argv[1];
  //   while (tmp[0] != 0) {
  //     if (!isdigit((int)tmp[0])) {
  //       fprintf(stderr, "Error: %s is no valid port --- exit\n", argv[1]);
  //       return 2;
  //     }
  //     tmp++;
  //   }
  //   port = atoi(argv[1]);
  // }

  // 50 clients

  pthread_t thread;

  int sockfd = createFeed(stdPort);  // for first touch ==> creating socket
  // fprintf(stderr, "Port: %d\n", port);

  while (1) {
    while (connectionCount > 50) {
      usleep(100);
    }

    // for (int i = 0; i < 50; i++) {
    //   printf("array[%d] = %d\n\n", i + 1, sockfds[i]);
    // }
    // printf("connectionCount: %d\n", connectionCount);

    int connfd, len;
    struct sockaddr_in client;

    // fprintf(stderr, "sockfd: %d\n", sockfd);

    connfd = setupConnection(sockfd);

    if (connfd < 0) {
      fprintf(stderr, "Error: Server accept failed --- exit\n");
      exit(6);
    } else {
      // printf("Server accept client.\n");
    }

    port = firstTouch(connfd, port);  // ==> now got to firstTouch

    // fprintf(stderr, "PORT: %d\n", port);
    // fprintf(stderr, "CONNFD: %d\n", connfd);

    int permanentSockfd = createFeed(port);

    int nextFreeIndex = 0;

    for (int i = 0; i < 50; i++) {
      if (sockfds[i] == 0) {
        nextFreeIndex = i;
        break;
      }
    }

    sockfds[nextFreeIndex] = setupConnection(permanentSockfd);
    int* arg = malloc(sizeof(*arg));
    if (arg == NULL) {
      exit(EXIT_FAILURE);
    }
    *arg = sockfds[nextFreeIndex];
    pthread_create(&thread, NULL, serverFunction, arg);
  }

  pthread_join(thread, NULL);

  // Closing all sockets when server ist not used anymore
  // for (int i = connectionCount; i > 0; i--) {
  //   printf("closing socket: %i\n", sockfds[i]);
  //   close(sockfds[i]);
  // }
}

// +++++++++++++++++++  Logic Client +++++++++++++++++++

int setupConnectionClient(char* _serverAddress, int _port) {
  int sockfd;
  struct sockaddr_in servaddr;

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    fprintf(stderr, "Error: Cannot create socket --- exit\n");
    return -3;
  } else {
    printf("Socket successfully created..\n");
  }
  bzero(&servaddr, sizeof(servaddr));

  // Set up socket
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(_serverAddress);
  servaddr.sin_port = htons(_port);

  // Connect to server
  if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
    fprintf(stderr, "Error: Cannot connect server on port %d --- exit\n",
            _port);
    return -4;
  } else {
    printf("connected to the server..\n");
  }

  return sockfd;
}

int firstTouchClient(char* _serverAddress, int _port) {
  int sockfd = setupConnectionClient(_serverAddress, _port);
  if (sockfd < 0) exit(-1 * sockfd);
  char buffer[MESG_SIZE];
  bzero(buffer, MESG_SIZE);
  strcpy(buffer, "HelloServer");
  write(sockfd, buffer, MESG_SIZE);
  bzero(buffer, MESG_SIZE);
  read(sockfd, buffer, MESG_SIZE);
  _port = ((int*)buffer)[0];
  close(sockfd);
  return _port;
}

void clientFunction(int sockfd) {
  char buffer[MESG_SIZE];
  int n;
  while (1) {
    // // clear buffer
    // bzero(buffer, sizeof(buffer));
    // printf("Enter string: ");
    // n = 0;
    // while ((buffer[n++] = getchar()) != '\n')
    //   ;
    // write(sockfd, buffer, sizeof(buffer));
    // bzero(buffer, sizeof(buffer));

    // read(sockfd, buffer, sizeof(buffer));
    // printf("From Server : %s", buffer);

    // if ((strncmp(buffer, "QUIT", 4)) == 0) {
    //   printf("Client exit.\n");
    //   break;
    // }
  }
}

void permanentConnectionClient(char* _serverAddress, int _port) {
  int sockfd = setupConnectionClient(_serverAddress, _port);
  int retries = 5;
  printf("Permanent port: %d\n", _port);
  while (sockfd < 0 && retries > 0) {
    sleep(1);
    sockfd = setupConnectionClient(_serverAddress, _port);
    retries--;
  }
  if (retries == 0 && sockfd < 0) {
    fprintf(stderr, "Error: Cannot reconnect server on port %d --- exit\n",
            _port);
    exit(-1 * sockfd);
  }

  // clientFunction(sockfd, );
  close(sockfd);
}

// +++++++++++++++++++ ClientServer Logic +++++++++++++++++++

typedef struct {
  int clientID;             // ID von der verschicken soll
  char mesg[MAX_MSG_SIZE];  // Nachricht die verschickt werden
} MessageForClient;

typedef struct {
  int ID;
  MessageForClient* mfc;
  int connfd;
  int address;

} ThreadData;

MessageForClient Message;        // Globale Nachricht zum austausch für alle
pthread_t clients[MAX_CLIENTS];  // Alle Thread/Clients
pthread_mutex_t mut;

// preparing message for sending to client
int setMessageForClient(int _id, char* _str) {
  if (_id >= MAX_CLIENTS) {
    fprintf(stderr, "Error: no such client\n");
    return -1;
  }
  pthread_mutex_lock(&mut);
  if (Message.clientID != -1) {
    pthread_mutex_unlock(&mut);
    return 0;
  }
  Message.clientID = _id;
  strncpy(Message.mesg, _str, MAX_MSG_SIZE);
  pthread_mutex_unlock(&mut);
  return 1;
}

// accessing the client thread and printing out the message
void* ThreadFunc(void* _data) {
  ThreadData data = *((ThreadData*)_data);
  // id from client
  int myID = data.ID;
  MessageForClient* mfc = data.mfc;
  char buffer[MAX_MSG_SIZE];

  while (1) {
    pthread_mutex_lock(&mut);
    if (mfc->clientID == myID) {
      strncpy(buffer, mfc->mesg, MAX_MSG_SIZE);
      // printf("%d: got message: %s\n", myID, buffer);
      mfc->clientID = -1;
      pthread_mutex_unlock(&mut);
      if (strncmp(buffer, "quit", 4) == 0) break;
    } else {
      pthread_mutex_unlock(&mut);
      usleep(100);
    }
  }
  return NULL;
}

void readAndSendMessage(int clientNumber) {
  // reading message from destination and message from commandline
  char buffer[MAX_MSG_SIZE];
  int target = clientNumber;
  Message.clientID = -1;
  // char buffer[MESG_SIZE];
  int n;

  // clear buffer
  bzero(buffer, sizeof(buffer));
  printf("Enter string: ");
  n = 0;
  while ((buffer[n++] = getchar()) != '\n')
    ;

  // while (Message.clientID != -1) {
  //   usleep(100);
  // }
  // // reading inputs
  // printf("Ziel: ");
  // scanf("%d", &target);
  // printf("Nachricht: ");
  // scanf("%s", buffer);

  while (Message.clientID != -1) {
    usleep(100);
  }

  // Sleep when sending message to client
  while (!setMessageForClient(target, buffer)) usleep(100);
  printf("%s", Message.mesg);

  // write(sockfd, buffer, sizeof(buffer));
  // bzero(buffer, sizeof(buffer));

  // read(sockfd, buffer, sizeof(buffer));
  // printf("From Server : %s", buffer);
}

void establishConnection() {
  while (1) {
    int port;
    char buffer1[MESG_SIZE];
    int n1;

    // clear buffer
    bzero(buffer1, sizeof(buffer1));
    printf("Enter IP Address: ");
    n1 = 0;
    while ((buffer1[n1++] = getchar()) != '\n')
      ;

    char buffer2[MESG_SIZE];
    int n2;

    // clear buffer
    bzero(buffer2, sizeof(buffer2));
    printf("Enter Port: ");
    n2 = 0;
    while ((buffer2[n2++] = getchar()) != '\n')
      ;

    printf("IP Address: %s", buffer1);
    printf("Port: %s", buffer2);

    port = firstTouchClient(buffer1, atoi(buffer2));

    permanentConnectionClient(buffer1, port);

    establishedConnections++;

    break;
  }
}

void stopConnection() {
  char buffer[MESG_SIZE];
  int n;

  // clear buffer
  bzero(buffer, sizeof(buffer));
  printf("Client Number: ");
  n = 0;
  while ((buffer[n++] = getchar()) != '\n')
    ;

  printf("Stop connection: Client %s", buffer);

  // socket
}

void quit() {
  printf("Stopping server... \n");
  // sleep(100);
  exit(0);
}

void waitForInput() {
  char buffer[MESG_SIZE];
  int n;

  while (1) {
    // clear buffer
    bzero(buffer, sizeof(buffer));
    printf("Enter Character (1 - 5 or C, D, Q): \n\n");
    n = 0;
    while ((buffer[n++] = getchar()) != '\n')
      ;
    //(buffer[0] = tolower(getchar()) != '\n');
    // tolower(getchar())

    // write(sockfd, buffer, sizeof(buffer));
    // bzero(buffer, sizeof(buffer));

    // read(sockfd, buffer, sizeof(buffer));
    // printf("From Server : %s", buffer);

    if ((strncmp(buffer, "1", 1)) == 0) {
      printf("Your input: 1\n");
      readAndSendMessage(1);
      // sleep(100);
      // continue;
      break;
    }

    if ((strncmp(buffer, "2", 1)) == 0) {
      printf("Your input: 2\n");
      readAndSendMessage(2);
      break;
    }
    if ((strncmp(buffer, "3", 1)) == 0) {
      printf("Your input: 3\n");
      readAndSendMessage(3);
      break;
    }
    if ((strncmp(buffer, "4", 1)) == 0) {
      printf("Your input: 4\n");
      readAndSendMessage(4);
      break;
    }
    if ((strncmp(buffer, "5", 1)) == 0) {
      printf("Your input: 5\n");
      readAndSendMessage(5);
      break;
    }

    if ((strncmp(buffer, "C", 1)) == 0) {
      printf("Your input: C\n");
      establishConnection();
      break;
    }
    if ((strncmp(buffer, "D", 1)) == 0) {
      printf("Your input: D\n");
      stopConnection();
      break;
    }
    if ((strncmp(buffer, "Q", 1)) == 0) {
      printf("Your input: Q\n");
      quit();
      break;
    }
  }
}

int main(int argc, char** argv) {
  signal(SIGPIPE, SIG_IGN);  // Stopping Server crash after ctrl + c
  setbuf(stdout, 0);         // ==> deleting standard out puffer

  // starting thread for server
  pthread_t server;
  pthread_create(&server, NULL, &runServer, NULL);

  // pthread_join(server, NULL);

  ThreadData td[MAX_CLIENTS];
  for (int i = 0; i < MAX_CLIENTS; i++) {
    td[i].ID = i;
    td[i].mfc = &Message;
  }
  pthread_mutex_init(&mut, NULL);

  // initialising Client Threads
  for (int i = 0; i < 5; i++) {
    pthread_create(&clients[i], NULL, ThreadFunc, &(td[i]));
  }

  while (1) {
    waitForInput();
  }

  // pthread_join(thread, NULL);

  // // Closing all sockets when server ist not used anymore
  // // for (int i = connectionCount; i > 0; i--) {
  // //   printf("closing socket: %i\n", sockfds[i]);
  // //   close(sockfds[i]);
  // // }
  return 0;
}
