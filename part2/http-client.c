#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

static void die(const char *s) { perror(s); exit(1); }

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s <host name> <port number> <file path>\n   ex) http-client www.example.com 80 /index.html\n",
                argv[0]);
        exit(1);
    }

    const char *ip = argv[1];
    unsigned short port = atoi(argv[2]); // ARRAY TO INTEGER
    const char *filepath = argv[3];

    // Create a socket for TCP connection

    int sock; // socket descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) //SAME THROUGHOUT THE SEMESTER
        die("socket failed");

    // Construct a server address structure

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); // must zero out the structure
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port        = htons(port); // must be in network byte order

    // Establish a TCP connection to the server

    if (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("connect failed");
	//else printf("success\n");

    //Send stuff
	
	char request[1000];
	sprintf(request, "GET %s HTTP/1.0\r\nHost: %s:<%d>\r\n\r\n", filepath, ip, port);
	printf("--------\n%s--------\n", request);
    send(sock, request, strlen(request), 0);
    char buff[4096];
	recv(sock, buff, 2000, MSG_WAITALL);
	printf("%s", buff);
	
	
	

	// Clean-up

    close(sock);
    return 0;
}
