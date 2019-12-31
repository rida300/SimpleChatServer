Sources:
1. https://stackoverflow.com/questions/28244082/proper-use-of-getsockopt-and-setsockopt-for-so-rcvtimeo-and-so-sndtimeo
2. https://stackoverflow.com/questions/7140438/error-transport-endpoint-is-already-connected
3. https://beej.us/guide/bgnet/html/multi/recvman.html
4. https://www.geeksforgeeks.org/udp-client-server-using-connect-c-implementation/
5. http://man7.org/linux/man-pages/man2/write.2.html
6. https://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
7. https://www.qnx.com/developers/docs/6.4.0/neutrino/lib_ref/f/fileno.html
8. https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/rtfil.htm
9. https://rosettacode.org/wiki/Chat_server#C
10.https://www.gnu.org/software/libc/manual/html_node/Server-Example.html
11. https://rosettacode.org/wiki/Chat_server
12. https://www.tenouk.com/Module41.html
13. http://man7.org/linux/man-pages/man2/write.2.html
14. https://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
15. http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
16. https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
17. https://stackoverflow.com/questions/36868273/c-fifos-how-to-read-servers-stdin-while-waiting-for-client-requests

Part1:

Compile using 'make' in the corresponding directory.
To run the server in the background: ./server &
To run the client: ./client

A linked list was used to keep track of all clients. Separate functions were added for broadcast, adding a new client, checking for duplicates.If the user enetered a name that already exists, the refuse_duplicates function would allow the user to re-enter a name. The broadcast function was called from addNewClient to announce them to the chat. TCP server was used along with select to choose ready file descriptors. The files can be compiled using the make file which uses gcc commands. This can be done by typing make in the terminal. 
 
Part2:

Compile using 'make' in the corresponding directory. Then, execute 'make directoryServer1.c'
To run the directory server in the background: ./directoryServer &
To run the chat server in the background: ./server "Topic" 8787 &
where the first command line argument is the chatroom topic and second is the port associated with it.
To run the client: ./client

The directory server was implemented using UDP. The server and client sides have functions to register them with the directory. The makefile can be used to compile the char and server side but the directory server needs to be compiled separately using make directoryServer. The de-registering was handled using the SIGINT signal handler. Messages were sent to the directory server by the chat server letting it know that the chat server should be removed from its list of chatrooms. On the chatServer side, there is a function to query all clients that are connected to the server, and de-registering this server will close the connections to the clients as well. 
