#
# Makefile for Example
#
LIBS	=
CFLAGS	= -g -ansi -pedantic

all:	tcp 

#
# Internet stream version (TCP protocol).
#

tcp:	chatServer2 chatClient2 
udp: 	directoryServer2
chatClient2.o chatServer2.o directoryServer2.o: inet.h

chatServer2:	chatServer2.o 
	gcc $(CFLAGS) -o server chatServer2.o $(LIBS)

chatClient2:	chatClient2.o 
	gcc $(CFLAGS) -o client chatClient2.o $(LIBS)

directoryServer2:	directoryServer2.o 
	gcc $(CFLAGS) -o directoryServer directoryServer2.o $(LIBS)

#
# Clean up the mess we made
#
clean:
	rm *.o \
	server4 client4
 
