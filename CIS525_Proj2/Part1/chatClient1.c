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

void main(void)
{
	int				   sockfd;
	struct sockaddr_in serv_addr;
	char			   s[MAXMSG];
	int				   response;
	int				   nread;
	int	client_inputFD = fileno(stdin);
	int				   i;
	fd_set			   active_fd_set;
	fd_set			   read_fd_set;
	struct timeval timeout;
	timeout.tv_sec=1200000;
	timeout.tv_usec=0;/*microsec*/

	/*memset(address to start filling memory at, 
			 value to fill it with, 
			 number of bytes to fill)*/
	memset((char *)&serv_addr, 0, sizeof(serv_addr)); /*reset the serv_addr, then reassign*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port = htons(SERV_TCP_PORT);

	/* Create the socket. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket (client)");
		return(1);
	}

	/* Connect to the server. */
	if (connect(sockfd, (struct sockfd *) &serv_addr, sizeof(serv_addr)) < 0)
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
		char clientName[MAXNAME];
		printf("what does this print, %s", s); /*prints "plz enter name or whatever is returned by the server"*/
		fflush(stdout);
		fgets(clientName, MAXNAME, stdin);
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
					/*msg*/
					memset(s, 0, MAXMSG);
					read(sockfd, &s, MAXMSG);
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