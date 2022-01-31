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

#define MESG_SIZE 80
#define SA struct sockaddr  // ==> (SA*)&servaddr
// #define STD_PORT 4567

int connectionCount = 0;
int sockfds[50];

int clientThreads[5];

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
  printf("Verbindung aufbauen\n");
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
  fprintf(stderr, "sending Port %d\n", _port + 1);
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

  printf("Thread started for new client!\n");

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

void readAndSendMessage(int clientNumber) {
  char buffer[MESG_SIZE];
  int n;

  // clear buffer
  bzero(buffer, sizeof(buffer));
  printf("Enter string: ");
  n = 0;
  while ((buffer[n++] = getchar()) != '\n')
    ;
  // write(sockfd, buffer, sizeof(buffer));
  // bzero(buffer, sizeof(buffer));

  // read(sockfd, buffer, sizeof(buffer));
  // printf("From Server : %s", buffer);
}

int main(int argc, char** argv) {
  signal(SIGPIPE, SIG_IGN);  // Stopping Server crash after ctrl + c
  setbuf(stdout, 0);         // ==> deleting standard out puffer

  char buffer[MESG_SIZE];
  int n;

  while (1) {
    // clear buffer
    bzero(buffer, sizeof(buffer));
    printf("Enter Character (1 - 5 or C, D, Q): ");
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
      break;
    }
    if ((strncmp(buffer, "D", 1)) == 0) {
      printf("Your input: C\n");
      break;
    }
    if ((strncmp(buffer, "Q", 1)) == 0) {
      printf("Your input: Q\n");
      break;
    }

    // switch (buffer) {
    //   case '1':
    //     printf("Your input: 1\n");
    //     break;
    //   case '2':
    //     printf("Your input: 2\n");
    //     break;
    //   case '3':
    //     printf("Your input: 3\n");
    //     break;
    //   case '4':
    //     printf("Your input: 4\n");
    //     break;
    //   case '5':
    //     printf("Your input: 5\n");
    //     break;
    //   case 'c':
    //     printf("Your input: C\n");
    //     break;
    //   case 'd':
    //     printf("Your input: D\n");
    //     break;
    //   case 'q':
    //     printf("Your input: Q\n");
    //     break;

    //   default:
    //     break;
    // }
  }

  // char tmp = argv[1];

  // int port = 4567;
  // int stdPort = 4567;

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

  // pthread_t thread;

  // int sockfd = createFeed(stdPort);  // for first touch ==> creating socket
  // fprintf(stderr, "Port: %d\n", port);

  // while (1) {
  //   while (connectionCount > 50) {
  //     usleep(100);
  //   }

  //   for (int i = 0; i < 50; i++) {
  //     printf("array[%d] = %d\n\n", i + 1, sockfds[i]);
  //   }
  //   printf("connectionCount: %d\n", connectionCount);

  //   int connfd, len;
  //   struct sockaddr_in client;

  //   // fprintf(stderr, "sockfd: %d\n", sockfd);

  //   connfd = setupConnection(sockfd);

  //   if (connfd < 0) {
  //     fprintf(stderr, "Error: Server accept failed --- exit\n");
  //     exit(6);
  //   } else {
  //     printf("Server accept client.\n");
  //   }

  //   port = firstTouch(connfd, port);  // ==> now got to firstTouch

  //   // fprintf(stderr, "PORT: %d\n", port);
  //   // fprintf(stderr, "CONNFD: %d\n", connfd);

  //   int permanentSockfd = createFeed(port);

  //   int nextFreeIndex = 0;

  //   for (int i = 0; i < 50; i++) {
  //     if (sockfds[i] == 0) {
  //       nextFreeIndex = i;
  //       break;
  //     }
  //   }

  //   sockfds[nextFreeIndex] = setupConnection(permanentSockfd);
  //   int* arg = malloc(sizeof(*arg));
  //   if (arg == NULL) {
  //     exit(EXIT_FAILURE);
  //   }
  //   *arg = sockfds[nextFreeIndex];
  //   pthread_create(&thread, NULL, serverFunction, arg);
  // }

  // pthread_join(thread, NULL);

  // // Closing all sockets when server ist not used anymore
  // // for (int i = connectionCount; i > 0; i--) {
  // //   printf("closing socket: %i\n", sockfds[i]);
  // //   close(sockfds[i]);
  // // }

  return 0;
}
