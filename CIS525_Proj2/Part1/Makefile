#
# Makefile for Example
#
LIBS	=
CFLAGS	= -g -ansi -pedantic

all:	tcp 

#
# Internet stream version (TCP protocol).
#

tcp:	chatServer1 chatClient1

chatClient1.o chatServer1.o: inet.h

chatServer1:	chatServer1.o 
	gcc $(CFLAGS) -o server chatServer1.o $(LIBS)

chatClient1:	chatClient1.o 
	gcc $(CFLAGS) -o client chatClient1.o $(LIBS)

#
# Clean up the mess we made
#
clean:
	rm *.o \
	server4 client4
 
