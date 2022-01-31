#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MESG_SIZE 80
#define SA struct sockaddr

void clientFunction(int sockfd) {
  char buffer[MESG_SIZE];
  int n;
  while (1) {
    // clear buffer
    bzero(buffer, sizeof(buffer));
    printf("Enter string: ");
    n = 0;
    while ((buffer[n++] = getchar()) != '\n')
      ;
    write(sockfd, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));

    read(sockfd, buffer, sizeof(buffer));
    printf("From Server : %s", buffer);

    if ((strncmp(buffer, "QUIT", 4)) == 0) {
      printf("Client exit.\n");
      break;
    }
  }
}

int setupConnection(char* _serverAddress, int _port) {
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

int firstTouch(char* _serverAddress, int _port) {
  int sockfd = setupConnection(_serverAddress, _port);
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

void permanentConnection(char* _serverAddress, int _port) {
  int sockfd = setupConnection(_serverAddress, _port);
  int retries = 5;
  printf("Permanent port: %d\n", _port);
  while (sockfd < 0 && retries > 0) {
    sleep(1);
    sockfd = setupConnection(_serverAddress, _port);
    retries--;
  }
  if (retries == 0 && sockfd < 0) {
    fprintf(stderr, "Error: Cannot reconnect server on port %d --- exit\n",
            _port);
    exit(-1 * sockfd);
  }
  clientFunction(sockfd);
  close(sockfd);
}

int main(int argc, char** argv) {
  char* serverAddress = "127.0.0.1";
  int port = 4567;

  if (argc > 3) {
    fprintf(stderr, "usage: %s [server [port]] --- exit\n", argv[1]);
    return 1;
  }
  if (argc == 3) {
    char* tmp = argv[2];
    while (tmp[0] != 0) {
      if (!isdigit((int)tmp[0])) {
        fprintf(stderr, "Error: %s is no valid port --- exit\n", argv[1]);
        return 2;
      }
      tmp++;
    }
    port = atoi(argv[2]);
  }

  port = firstTouch(serverAddress, port);
  permanentConnection(serverAddress, port);

  return 0;
}
