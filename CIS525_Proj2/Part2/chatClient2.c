#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "inet.h"

#define MAXMSG   10000
#define MAXNAME  100
#define MAXTOPIC 256
char * portC;

void main()
{
    registerWithDirectory();
    int portInt = atoi(portC);
    int                sockfd;
    struct sockaddr_in serv_addr;
    char               s[MAXMSG];
    int                response;
    int                nread;
    int client_inputFD = fileno(stdin);
    int                i;
    fd_set             active_fd_set;
    fd_set             read_fd_set;
    struct timeval timeout;
    timeout.tv_sec=1200000;
    timeout.tv_usec=0;/*microsec*/

    /*memset(address to start filling memory at,
             value to fill it with,
             number of bytes to fill)*/
    memset((char *)&serv_addr, 0, sizeof(serv_addr)); /*reset the
serv_addr, then reassign*/
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port = htons(portInt);
    /* Create the socket. */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket (client)");
        return(1);
    }
    /* Connect to the server. */
    int connectResult = connect(sockfd, (struct sockfd *) &serv_addr,sizeof(serv_addr));
    fprintf(stderr, "connect result is %d\n", connectResult);
    if (connectResult < 0)
    {
        perror("Client did not connect to server");
        return(1);
    }
    /* connection was successfully established by this point*/
    else
    {
        printf("we here now, in the main of client, after connecting to server\n");
        fflush(stdout);
        memset(s, 0, MAXMSG); /*reset s*/
        read(sockfd, &s, MAXMSG); /* reads "plz enter name from server"*/
        EndMessageCheck(s);
        char clientName[MAXNAME];

        printf("what does this print, %s\n", s); /*prints "plz enter name or whatever is returned by the server"*/
        fflush(stdout);
        memset(clientName, '\0', MAXNAME);
        /*fgets(clientName, MAXNAME, stdin);*/
        scanf("%s", clientName);
        int len = strlen(clientName);
        if (clientName[len - 1] == '\n')
            clientName[len - 1] = 0;
        /*if(strcmp(clientName, "")!=0)*/
        write(sockfd, &clientName, MAXMSG);
        memset(s, 0, MAXMSG);
        FD_ZERO(&active_fd_set);
        FD_SET(sockfd, &active_fd_set);
        FD_SET(client_inputFD, &active_fd_set);
    }

    /*FD_ZERO(&read_fd_set);*/
    for (;;)
    {
        read_fd_set = active_fd_set;
        /*printf("sockfd: %d", sockfd);
        fflush(stdout);
        */
        int socketCount = select(sockfd+1, &read_fd_set, NULL, NULL, &timeout);
        if (socketCount < 0)
        {
            perror("Select in client failed as no sockfd is ready");
            exit(1);
        }
        if(socketCount ==0 )
        {
            printf("Timeout!\n");
            fflush(stdout);
            exit(1);
        }
        for (i = 0; i < sockfd+1; i++)
        {
            if (FD_ISSET(i, &read_fd_set)) /*read or active*/
            {
                socketCount--;
                /*trying to broadcast to all clients*/
                if (i == client_inputFD)
                {
                    /* enters on messages */
                    fgets(s, MAXMSG, stdin);
                    write(sockfd, s, MAXMSG);
                }
                else
                {
                    /* enters on addition of client, think? */
                    memset(s, 0, MAXMSG);/*dk if needed, might destroyeverythng*/
                    read(sockfd, &s, MAXMSG);
                    EndMessageCheck(s);
                    /*fprintf(stderr, "read on clientside: %s\n", s);*/

                    printf(s);
                    fflush(stdout);
                    /*
                    printf("What is s: %s \n", s);
                    fflush(stdout);
                    */
                    break;
                }
            }
        }

    }
}
/*checks if server has shut down, then exits*/
int EndMessageCheck(char * c1)
{
    if(strcmp(c1, "End\0")==0)
    {
        exit(0);
    }
    return 0;
}
/*registering with directory server to receive list of chatrooms, uses UDP*/
void registerWithDirectory()
{
    int                sockfd;
    struct sockaddr_in cli_addr, serv_addr;
    char               s[MAXMSG];
    int                response;
    int                nread;
    int client_inputFD = fileno(stdin);
    int                i;
    fd_set             active_fd_set;
    fd_set             read_fd_set;
    struct timeval timeout;
    timeout.tv_sec=1200000;
    timeout.tv_usec=0;/*microsec*/

    /*memset(address to start filling memory at,
             value to fill it with,
             number of bytes to fill)*/
    memset((char *)&serv_addr, 0, sizeof(serv_addr)); /*reset the serv_addr, then reassign*/
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port        = htons(SERV_UDP_PORT);

    /* Set up the address of the client. */
    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family      = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(0);
    cli_addr.sin_port        = htons(0);
    /*create the socket*/
    int sockfdU;
    if ((sockfdU = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("client: can't open stream socket directory server");
        return(1);
    }
    if (bind(sockfdU, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        perror("client: can't bind local address");
        exit(1);
    }
    int clilen = sizeof(cli_addr);
    int servlen = sizeof(serv_addr);
    char * identity = malloc(sizeof(char)*7);
    strcpy(identity, "Client");
    /*int =sendto(sockfdU, identity, sizeof(identity), 0, (struct sockaddr *) &serv_addr, servlen);*/
    int sentIdentity=sendto(sockfdU, identity, sizeof(identity), 0, (struct sockaddr *) &serv_addr, servlen);
    fprintf(stderr, "Here is a list of all available chat rooms, please enter an address from the following options:\n");
    /*If the user enters anything longer than the port number or
something that cant be parsed to a port number, the client will be
disconnected.*/
    char * allChats[150];
    char * topicsReceived[150];
    char * portsReceived[150];
    char singleElement[MAXTOPIC+15];
    char stillReceiving;

    do{
    int receivedList=recvfrom(sockfdU, singleElement, sizeof(singleElement), 0, (struct sockaddr *) &serv_addr, &servlen);
    EndMessageCheck(singleElement);
    printf("errno is %s\n", strerror(errno));
    stillReceiving=singleElement[0];
    int serverIndex;
    /*
    Tokenizing
    */
    char * first= malloc(sizeof(char));
    char * secondT = malloc(sizeof(char)* MAXTOPIC);
    char * thirdP = malloc(sizeof(char) * 6);

    char* token = strtok(singleElement, ",");
    int counterT=0;
    int counterP=0;
    int counterTokenize=0;
    while (token != NULL) {
        if(counterTokenize==0)
        {
          strcpy(first, token);
        }
        else if(counterTokenize == 1)
        {
         fprintf(stderr,"Chat room topic is: %s\t", token);
         topicsReceived[counterT] = token;
         counterT++;
        }
        else
        {
         fprintf(stderr,"Corresponding Address: %s\n", token);
         portsReceived[counterP]= token;
         counterP++;
        }
        counterTokenize++;
        token = strtok(NULL, ",");
    }
    memset((char *)&singleElement, '\0', (MAXTOPIC+15));
    } while(stillReceiving == 'g');
    char * portRead = malloc(sizeof(char)*5);
    printf("Please choose a chat room to join by entering the address\n");
    scanf("%s", portRead);
    portC = malloc(sizeof(char)*5);
    strcpy(portC, portRead);
    return;
}