#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CLIENTS  5
#define MAX_MSG_SIZE 128

typedef struct {
	int clientID;  // ID von der verschicken soll
	char mesg[MAX_MSG_SIZE];  // Nachricht die verschickt werden
} MessageForClient; 

typedef struct {   
	int ID;
	MessageForClient* mfc;
} ThreadData;

MessageForClient Message;  // Globale Nachricht zum austausch für alle
pthread_t clients[MAX_CLIENTS];  // Alle Thread/Clients
pthread_mutex_t mut;      

int setMessageForClient(int _id, char* _str) {
	if(_id >= MAX_CLIENTS) {
		fprintf(stderr, "Error: no such client\n");
		return -1;
	}
	pthread_mutex_lock(&mut);
	if(Message.clientID != -1) {
		pthread_mutex_unlock(&mut);
		return 0;
	}
	Message.clientID = _id;
	strncpy(Message.mesg, _str, MAX_MSG_SIZE);
	pthread_mutex_unlock(&mut);
	return 1;
}

void* ThreadFunc(void* _data) {
	ThreadData data = *((ThreadData*)_data);
	int myID = data.ID;
	MessageForClient* mfc = data.mfc;
	char buffer[MAX_MSG_SIZE];

	while(1) {
		pthread_mutex_lock(&mut);
		if(mfc->clientID == myID) {
			strncpy(buffer, mfc->mesg, MAX_MSG_SIZE);
			printf("%d: got message: %s\n", myID, buffer);
			mfc->clientID = -1;
			pthread_mutex_unlock(&mut);
			if(strncmp(buffer, "quit", 4) == 0)
				break;
		} else {
			pthread_mutex_unlock(&mut);
			usleep(100);
		}
	}
	return NULL;
}

int main() {
	ThreadData td[MAX_CLIENTS];
	for(int i = 0; i < MAX_CLIENTS; i++) {
		td[i].ID = i;
		td[i].mfc = &Message;
	}
	pthread_mutex_init(&mut, NULL);

	for(int i = 0; i < 5; i++) {
		pthread_create(&clients[i], NULL, ThreadFunc, &(td[i]));
	}

	char buffer[MAX_MSG_SIZE];
	int target;
	Message.clientID = -1;
	while(1) {
		while(Message.clientID != -1) {
			usleep(100);
		}
		printf("Ziel: ");
		scanf("%d", &target);
		printf("Nachricht: ");
		scanf("%s", buffer);

		while(!setMessageForClient(target, buffer))
			usleep(100);
	}

	return 0;
}
