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

#define MAXTOPIC  10000

struct server {
    int sockfd;
    char topic[MAXTOPIC];
    char port[5];
    char IP[15];
    int serverLength;
    int childPid;
    struct server * next;
    struct server * prev;
};

struct server* headptr = NULL;
char buffer[MAXTOPIC];
int nbytes;
char pt[MAXTOPIC+5];
char portG[5];
char topicG[MAXTOPIC];
char ipG[14];
char * allChats;
int shutDown;

/*
adds a new server to the lined list by using the port and topic provided as arguments.
Also checks to find duplicates, if found, the server will exit on its side.
Creates a new server struct and copies the port, ip and topic into that struct.
Appends it to the end of the linked list
*/
int addNewServer(char * ip, char * port, char * topic)
{
    if (headptr == NULL || strcmp(ip, "")==0 || strcmp(topic, "")==0)
    {
        return -1; /*invalid arguments*/
    }
    struct server * curr = headptr;
    int deDup;
    while (curr->next != NULL)
    {
        deDup = strcmp(curr->next->topic, topic);
        if(deDup == 0)
        {
            return -2; /*found a duplicate topic*/
        }
        curr= curr->next;
    }
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    struct server * newServer = (struct server *) malloc(sizeof(struct server));
    strcpy(newServer->IP, ip);
    strcpy(newServer->port, port);
    strcpy(newServer->topic, topic);
    newServer->next = NULL;
    /*strcpy(newServer->topic, serverTopic);*/
    curr->next = newServer;
    return 1;
}

/*
Sends the available chat rooms to the client.
Queries through the linked list, creates a string separated by commas to differentiate 
between the port, and topic.
An additional character is prefixed to the end of the string to let the client know that 
it has received all chat room. The character is 'd' if all are sent or 'g' if in progress
*/
void retrieveAllChatRooms(int sockfd, int cli_addr, int clilen)
{

    if (headptr == NULL)
    {
        return -1;
    }
    allChats = (char *)malloc(sizeof(char)*(MAXTOPIC+6)*150);
    struct server * list = headptr;

    int index=0;
    char* element;
    while (list->next != NULL)
    {
        element = malloc(sizeof(char)*MAXTOPIC+15);
        strcpy(element, list->next->topic);
        element = strcat(element, ",");
        element = strcat(element, list->next->port);
        allChats=strcat(allChats, element);
        index++;
        int sendingList = sendto(sockfd, (const char*)element, sizeof(element), 0, (struct sockaddr *) cli_addr, clilen);
        printf("errno is %s\n", strerror(errno));
        memset((char *)&element, 0, sizeof(element));
        free(element);
        list = list->next;
    }
    return;
}

/*
removes all cliients from the linked list that belong to the server that was shit down
readjusts links between cells
*/
void removeServer(char * port)
{
	struct server * curr = headptr;
    while (curr->next != NULL)
    {
        if(strcmp(port,curr->next->port) ==0)
		{
			strncpy(curr->topic, "\0", sizeof(char)*MAXTOPIC);
			if(curr->next->next != NULL);
			curr->next->next->prev = curr;
		}
        curr= curr->next;
    }
}

/*
checks to see if a 0 was received which shows the server quit*/
void checkEndMsg(char * m, char *port)
{
	if(strcmp(m, "0")==0)
	{
		removeServer(port);
	}
}

int main(int argc, char *argv)
{
    int sockfd;
    char * request;
    struct sockaddr_in cli_addr, serv_addr;
    int serverLength;
    int serverCounter = 0;
    fd_set active_fd_set;
    fd_set read_fd_set;
    int messageLength = 0;
    char * messageBuf = malloc(sizeof(MAXTOPIC));
    struct server * head = (struct server *) malloc((sizeof(struct server)));

    headptr = head;
    strncpy(head->topic, "\0", 1);
    head->prev = NULL;
    head->next = NULL;
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port = htons(SERV_UDP_PORT);

    int fdMax;
    int listener;
    int newFd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        
        perror("server: can't open stream socket");
        return(1);
    }
    /* Bind socket to local address */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("server: can't bind local address ds");
        return(1);
    }
    listen(sockfd, 10);
    printf("after the listen\n");
    serverLength = sizeof(serv_addr);
    int clilen= sizeof(cli_addr);
    for (; ; )
    {
        char identity[7];
        int recvCli = recvfrom(sockfd, identity, sizeof(identity), 0, (struct sockaddr *)&cli_addr, &clilen);
        printf("errno is %s\n", strerror(errno));
        if(strcmp(identity, "Client") ==0)
        {
            char * allChatRooms;
            struct server * list = headptr;
            int index=0;
            int someSize= (sizeof(list->next->topic)+3+sizeof(list->next->port));
            char * element;
            while (list->next != NULL)
            {
                element = malloc(sizeof(char)*someSize);
                if(list->next->next == NULL)
                {
                    strcpy(element, "d,");
                }
                else
                {
                    strcpy(element, "g,");
                }
                element = strcat(element, list->next->topic);
                element = strcat(element, ",");
                element = strcat(element, list->next->port);
                index++;
                int sendingList = sendto(sockfd, (const char*)element, someSize, 0, (struct sockaddr *) &cli_addr, clilen);

                memset((char *)&element, '\0', sizeof(element));
                list = list->next;
            }
        }
        else if(strcmp(identity, "Server")==0)
        {
            memset(pt, '\0', sizeof(MAXTOPIC+10));

            int revBytes = recvfrom(sockfd, topicG, MAXTOPIC, 0, (struct sockaddr *)&cli_addr, &clilen);

            printf("errno is %s\n", strerror(errno));
            int revBytes2 = recvfrom(sockfd, portG, 5, 0, (struct sockaddr *)&cli_addr, &clilen);
            printf("errno is %s\n", strerror(errno));
            int addResult = addNewServer("129.130.10.43", portG, topicG);
            if(addResult == -2)
            {
                char deDupMsg[42];
                strcpy(deDupMsg, "A chatroom with this topic already exists.");
                int sendDup = sendto(sockfd, deDupMsg, strlen(deDupMsg)+1, 0, (struct sockaddr *) &cli_addr, clilen);

            }
			struct timeval read_timeout;
    		
            char* shut=malloc(sizeof(char)*2);
            int recvShutDown = recvfrom(sockfd, shut, sizeof(shut), 0, (struct sockaddr *)&cli_addr, &clilen);
            if(recvShutDown>0 )
			checkEndMsg(shut, portG);
        }
    }
}