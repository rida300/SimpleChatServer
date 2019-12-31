#include <time.h>
#include <string.h>
#include "inet.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXCLIENTNAME 100
#define MAXMSG  10000

struct client* headptr = NULL;
char buffer[MAXMSG];
int nbytes;


struct client {
	int sockfd;
	char name[MAXCLIENTNAME];
	int clientLength;
	int childPid;
	struct client * next;
	struct client * prev;
};


void broadcast(char * msg, int fd)
{					
	if (headptr == NULL || msg == NULL || strcmp(msg, "") == 0)
		return;
	
	struct client * currentNode = headptr;
	char name[MAXCLIENTNAME];
	char *buffer = malloc(sizeof(char) *(MAXMSG+MAXCLIENTNAME+3));
	/* Broadcasting the message*/
	if (fd != -1)
	{							
		while (currentNode->next != NULL)
		{
			if(currentNode->next->sockfd == fd)
			{
				strcpy(name, currentNode->next->name);
			}
			currentNode = currentNode->next;
		}
	sprintf(buffer, "%s: %s\n", name, msg);
	}
	/*Broadcasting the addition of a client*/
	else 
	{
		char * msgFirst = malloc(sizeof(char)*39);
		char * msgJoined = malloc(sizeof(char) *(MAXCLIENTNAME));
		strcpy(msgJoined, " has joined the chat.");
		char * concat= strcat(msg, msgJoined);
		strcpy(buffer, concat);
	/*	printf("%s\n", msg);
		fflush(stdout);*/
	}
	currentNode=headptr;
	while (currentNode->next != NULL)
	{
		write(currentNode->next->sockfd, buffer, MAXMSG+MAXCLIENTNAME+2);
		currentNode = currentNode->next;
	}
}

/*adds new client to the linked list, checks for duplicates, calls broadcast to announce addition*/
int addNewClient(int sockfd)
{
	if (headptr == NULL)
	{
		return -1;
	}
	struct client * curr = headptr;
	while (curr->next != NULL)
	{
		curr = curr->next;
	}

	struct client * newClient = (struct client *) malloc(sizeof(struct client));
	newClient->sockfd = sockfd;
	newClient->next = NULL;
	char * clientName= malloc(sizeof(char) * MAXCLIENTNAME);

	write(newClient->sockfd, "Please enter your name: ", MAXMSG);

	read_from_client(newClient->sockfd);
	strcpy(clientName, buffer);

	int e;

	while ((e = refuse_duplicates(clientName)) != 0 || strcmp(clientName, "")==0)
	{
		if (e < 0) {
			write(newClient->sockfd, "This name is taken. Choose another one: ", 40);	
		}
		else {
			write(newClient->sockfd, "The name is empty. Please choose another one: ", 46);
		}
		/*read(newClient->sockfd, clientName, MAXCLIENTNAME);*/
		read_from_client(newClient->sockfd);
		strcpy(clientName, buffer);
	}
	
	strcpy(newClient->name, clientName);
	curr->next = newClient;

	if (strlen(curr->name) == 0)
	{
		fprintf(stderr,"You are the first user to join the chat\n");
	}

	/*fprintf(stderr,"Calling broadcast from addNewClient\n");*/
	broadcast(clientName, -1);
	return 1;
}
/*read user input*/
int read_from_client(int filedes)
{
	memset(buffer, '\0', MAXMSG);
    nbytes = read(filedes, buffer, MAXMSG);
    if (nbytes < 0)
    {
        /* Read error. */
        perror("read");
        exit(EXIT_FAILURE);
    }
    else if (nbytes == 0)
        /* End-of-file. */
        return -1;
    else
    {
        /* Data read. */
        fprintf(stderr, "Server: got message: `%s'\n", buffer);
        return 0;
    }
}
/*queries linked list to return -1 if new name already exists*/
int refuse_duplicates(char * name)
{
	if (headptr == NULL)
	{
		exit(-1);
	}
	if (strcmp(name, "") == 0) {
		return 1;
	}
	struct client * currentNode = headptr;
	
	while (currentNode->next != NULL)
	{
		int i;
		/*name[strlen(name)-1] = 0;*/
		if (strcmp(currentNode->next->name, name) == 0 || strcmp(name, "")==0)
		{
			return -1;
		}
		currentNode = currentNode->next;
	}
	return(0);
}

int main(int argc, char *argv)
{
	int sockfd;
	char * request;
	struct sockaddr_in  cli_addr, serv_addr;
	int clientLength;
	int clientCounter = 0;
	fd_set active_fd_set;
	fd_set read_fd_set;
	int socketCount;
	int messageLength = 0;
	char * messageBuf = malloc(sizeof(MAXMSG));
	struct client * head = (struct client *) malloc((sizeof(struct client)));
	headptr = head;
	strncpy(head->name, "\0", 1);
	head->prev = NULL;
	head->next = NULL;

	fd_set master;
	fd_set read_fds;
	int fdMax;
	int listener;
	int newFd;
	int yes=1;
	int i,j,rv;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("server: can't open stream socket");
		return(1);
	}

	/* Bind socket to local address */
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_TCP_PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("server: can't bind local address");
		
		return(1);
	}
	
	FD_ZERO(&active_fd_set);
	FD_ZERO(&read_fd_set);
	FD_SET(sockfd, &active_fd_set);
	listen(sockfd, 10);
	clientLength = sizeof(cli_addr);
	for (; ; )
	{
		read_fd_set = active_fd_set;
		int socketCount = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);
		if (socketCount <= 0)
		{
			perror("select");
			exit(1);
		}
		int i;
		/* Service all the sockets with input pending. */
		for (i = 1; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &read_fd_set))
			{
				if (i == sockfd)
				{
					int acceptedClient = accept(sockfd, &(cli_addr), (unsigned*)&clientLength);
					if (acceptedClient < 0)
					{
						perror("Connection was not accepted");
						exit(1);
					}
					
					int addResult = addNewClient(acceptedClient);
					if(addResult != -2)
					FD_SET(acceptedClient, &active_fd_set);
				}
				else {
					char * messageBuffer = malloc(sizeof(char) * (MAXMSG+2));
					memset(messageBuffer, '\0', sizeof(messageBuffer));
					messageLength = recv(i, messageBuffer, MAXMSG, 0);
					
					broadcast(messageBuffer, i) ; /*new*/
					free(messageBuffer);
				}
			}
		}
	}
}


