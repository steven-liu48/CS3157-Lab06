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
#include <netdb.h>
#include <ctype.h>

static void die(const char *s) { perror(s); exit(1); }

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s <host name> <port number> <file path>\n   ex) http-client www.example.com 80 /index.html\n",
                argv[0]);
        exit(1);
    }

	// Format the IP
	
	struct hostent *he;
    char *serverName = argv[1];
    // get server ip from server name
    if ((he = gethostbyname(serverName)) == NULL) {
        die("gethostbyname failed");
    }
	char *ip = inet_ntoa(*(struct in_addr *)he->h_addr);

	// Take in port number and file path
	
    unsigned short port = atoi(argv[2]); // ARRAY TO INTEGER
    const char *filepath = argv[3];
	char *filename = strrchr(filepath, '/') + 1;
	//printf("File name: %s\n", filename);
	if (filename[0] == '\0'){
		printf("can't open output file: No such file or directory\n");
		exit(1);
	}
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

    // Send stuff
	
	char request[1000];
	sprintf(request, "GET %s HTTP/1.0\r\nHost: %s:%d\r\n\r\n", filepath, serverName, port);
	
	//printf("%s--------\n", request);
    send(sock, request, strlen(request), 0);

	// Receive stuff
    //char buff[4096];
	
	FILE *response = fdopen(sock, "rb");

	// Receive first line
	char c;
	char firstLine[20];
	int counter = 0;
	while ((c = fgetc(response))!='\r'){
		firstLine[counter] = c;
		counter++;
	}
	firstLine[counter] = 0;

	if (firstLine[9] == '2' && firstLine[10] == '0' && firstLine[11] == '0'){
			//printf("Success");
	}else{
			printf("%s\n", firstLine);
			die("");
	}
	
	// Look for the blank line
	
	char last = '\r';
	while(1){
		c = fgetc(response);
		if ((last == '\n') && (c == '\r')){
			//printf("Found");
			break;
		}
		last = c;
	}
	c = fgetc(response);

	// Read the content into the file
	
	FILE *fp = fopen(filename, "wb");
	
	int i = 0;
	char buffer[4096];
	while (1){
		//printf("%d", i);
		i = fread(buffer, 1, 4096, response);
		fwrite(buffer, 1, i, fp);
		if (i < 4096) {break;}
	}
	
	/*
	while ((c = fgetc(response)) != EOF){
		//printf("%c", c);
		i++;
		fputc(c, fp);
	}
	*/	
	printf("%d", i);

	fclose(fp);
	fclose(response);
	
	// Clean-up

    close(sock);
    return 0;
}
