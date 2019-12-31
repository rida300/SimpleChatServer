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
#include <signal.h>

#define MAXCLIENTNAME 100
#define MAXMSG  10000
#define MAXTOPIC 256

struct client* headptr = NULL;
char buffer[MAXMSG];
int nbytes;
int portSelected;
int sockFdDirectory;
struct sockaddr_in  cliu_addr, servu_addr;
int sigResult;
struct client {
    int sockfd;
    char name[MAXCLIENTNAME];
    int clientLength;
    int childPid;
    struct client * next;
    struct client * prev;
};

void sigintHandler(int);

/*
broadcasts to all clients in the linked list
separate if and lese for msg and client addition*/
void broadcast(char * msg, int fd)
{
    if (headptr == NULL || msg == NULL || strcmp(msg, "") == 0)
        return;
    struct client * currentNode = headptr;
    struct client * bdMsg = headptr;
    char name[MAXCLIENTNAME];
    char *bufferb = malloc(sizeof(char) *(MAXMSG+MAXCLIENTNAME+5));
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
    sprintf(bufferb, "%s: %s\n", name, msg);
    strcpy(bufferb, name);
    bufferb = strcat(bufferb, ": ");
    bufferb = strcat(bufferb, msg);
    /*concatenate na,e and msg, separate by colon*/
    while (bdMsg->next != NULL)
        {
            write(bdMsg->sockfd, bufferb, MAXMSG+MAXCLIENTNAME+2);
            bdMsg = bdMsg->next;
        }
    }
    /*Broadcasting the addition of a client*/
    else
    {
        char * msgFirst = malloc(sizeof(char)*39);
        char * msgJoined = malloc(sizeof(char) *(MAXCLIENTNAME));
        strcpy(msgJoined, " has joined the chat\n");
        char * concat= strcat(msg, msgJoined);
        strcpy(bufferb, concat);
    /*  printf("%s\n", msg);
        fflush(stdout);*/
    }
    currentNode=headptr;
    while (currentNode->next != NULL)
    {
        write(currentNode->next->sockfd, bufferb, MAXMSG+MAXCLIENTNAME+2);
        currentNode = currentNode->next;
    }
}
/*adds client to the linked list, calls refuse)dups to ensure no duplicates. Asks the user to re-enter 
name if empty or duplicate*/
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

    while ((e = refuse_duplicates(clientName)) != 0|| strcmp(clientName, "")==0)
    {
        if (e < 0) {
            write(newClient->sockfd, "This name is taken. Choose another one: ", 40);
        }
        else if(e == 1){
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
    /*calls broadcast to announce addition of client*/
    broadcast(clientName, -1);
    return 1;
}
/*to read user input on client side*/
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
        return 0;
    }
}
/*queries linked list to see if new client's name is already present*/
/*returns 1 if given name is empty, -1 if duplicate found*/
int refuse_duplicates(char * name)
{
    if (headptr == NULL)
    {
        exit(-1);
    }
    if (strcmp(name, "") == 0 || strcmp(name, "\0") == 0 || strcmp(name, " ") == 0 || name == NULL) {
     
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
    return (0);
}

int main(int argc, char * argv[3])
{
    signal (SIGINT, sigintHandler);
    if(argc!= 3)
    {
        fprintf(stderr, "Please provide the topic and port number\n");
        exit(0);
    }
    registerServer(argv[1], argv[2]);
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
    fprintf(stderr, "1. sockfd is : %d\n", sockfd);
    /* Bind socket to local address */
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portSelected);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("server: can't bind local address");
        return(1);
    }
    
    fprintf(stderr, "2. sockfd is : %d\n", sockfd);
	signal (SIGINT, sigintHandler);
    FD_ZERO(&active_fd_set);
    FD_ZERO(&read_fd_set);
    FD_SET(sockfd, &active_fd_set);
    listen(sockfd, 10);
    clientLength = sizeof(cli_addr);
    
    fprintf(stderr, "3. sockfd is : %d\n", sockfd);
    for (; ; )
    {
        read_fd_set = active_fd_set;
        int socketCount = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);
        if (socketCount <= 0)
        {
            perror("select");
            exit(1);
        }
        
        fprintf(stderr, "4. sockfd is : %d\n", sockfd);
        int i;
        char * messageBuffer=  malloc(sizeof(char) * (MAXMSG+2+MAXCLIENTNAME));;
        /* Service all the sockets with input pending. */
        for (i = 1; i < FD_SETSIZE; i++)
        {
            fprintf(stderr, "isset returns %d\n", (FD_ISSET(i, &read_fd_set)));
            if (FD_ISSET(i, &read_fd_set))
            {
                fprintf(stderr, "i is %d\n", i);
                fprintf(stderr, "5. sockfd is : %d\n", sockfd);
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
                    memset(messageBuffer, '\0', sizeof(messageBuffer));
                    messageLength = recv(i, messageBuffer, MAXMSG, 0);
                    broadcast(messageBuffer, i) ; /*new*/
                    free(messageBuffer);
                }
            }
        }
    }
}

/*to re register server, sigint handler is used, it sends the indication to 
directory server to remove it from the list of available chat room*/
void sigintHandler(int sig_num)
{   
    signal(sig_num, SIG_IGN);
	fprintf(stderr,"Goodbye! \n");
    char request='0';
    int servlen=sizeof(servu_addr);
    sigResult=sig_num;
	removeClients();
    sendto(sockFdDirectory, (char *)&request, sizeof(request), 0,(struct sockadr *) &servu_addr, servlen);
    fflush(stdout);
    exit(0);
}

/*remove all clients if the sigint was caught, also close the clients*/
void removeClients()
{
	struct client * cList = headptr;
	char * closeClient= malloc(sizeof(char)*4);
	strcpy(closeClient, "End");
	while (cList->next != NULL)
        {
            write(cList->sockfd, closeClient, sizeof(char)*4);
			strncpy(cList->name, "\0", sizeof(char)*MAXCLIENTNAME);
            cList = cList->next;
        }
}

/*registers the server with the directory server using UDP links*/
void registerServer(char * topic, char * port)
{
	signal (SIGINT, sigintHandler);
    portSelected = atoi(port);
    char * request;
    /*struct sockaddr_in  cliu_addr, servu_addr;*/

    memset((char *) &servu_addr, 0, sizeof(servu_addr));
    servu_addr.sin_family      = AF_INET;
    servu_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    servu_addr.sin_port        = htons(SERV_UDP_PORT);

    /* Set up the address of the client. */
    memset((char *) &cliu_addr, 0, sizeof(cliu_addr));
    cliu_addr.sin_family      = AF_INET;
    cliu_addr.sin_addr.s_addr = htonl(0);
    cliu_addr.sin_port        = htons(0);

    struct client * head = (struct client *) malloc((sizeof(struct client)));
    headptr = head;
    strncpy(head->name, "\0", 1);
    head->prev = NULL;
    head->next = NULL;
    char sending [MAXTOPIC+5];
    /*popen hostnet -1*/
    int sockfdU;
    if ((sockfdU = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server: can't open stream socket directory server");
        return(1);
    }
    sockFdDirectory=sockfdU;
    /*
    adding non blocking stuff
    */
    struct timeval read_timeout;
    read_timeout.tv_sec = 2;
    read_timeout.tv_usec = 50;
    setsockopt(sockfdU, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    /*end*/
     if (bind(sockfdU, (struct sockaddr *) &cliu_addr, sizeof(cliu_addr)) < 0) {
        perror("client: can't bind local address");
        exit(1);
    }
    else
    {
        int servlen = sizeof(servu_addr);
        char * identity = malloc(sizeof(char)*7);
        strcpy(identity, "Server");
        int sentIdentity=sendto(sockfdU, identity, sizeof(identity), 0, (struct sockaddr *) &servu_addr, servlen);
  
        int sentTopic=sendto(sockfdU, topic, MAXTOPIC, 0, (struct sockaddr *) &servu_addr, servlen);
   
        int sentPort=sendto(sockfdU, port, 5, 0, (struct sockaddr *) &servu_addr, servlen);
        
        char duped[43];
        int recvDup = recvfrom(sockfdU, duped , sizeof(duped), 0, (struct sockaddr *) &servu_addr, &servlen);
     
        if(recvDup >= 0 )
        {
            
            fprintf(stderr, duped);
            fprintf(stderr, "exiting...\n");
            exit(1);
        }
		char req='1';
        int sendPreShut=sendto(sockfdU, (char *)&req, sizeof(req), 0, (struct sockaddr *) &servu_addr, servlen);
    }
    return;
}
