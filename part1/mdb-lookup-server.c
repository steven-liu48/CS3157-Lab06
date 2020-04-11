/*
 * tcp-lookup-server.c
 */

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

#include "mdb.h"
#include "mylist.h"
#include <ctype.h>

struct Node *appendNode(struct List *list, struct MdbRec *data){
	struct Node *pointer = list -> head;
	if(!pointer){
        return addFront(list, data);
    }
	while(pointer -> next != NULL){
		pointer = pointer -> next;
	}
	return addAfter(list, pointer, data);
}

void setEnd(struct List *list){
	struct Node *pointer = list -> head;
	struct MdbRec *dataPointer;
	while(pointer -> next != NULL){
		dataPointer = pointer -> data;
		char *n = dataPointer -> name;
		int i = 0;
		while(isprint(n[i])){
			i++;
		}
		n[i] = '\0';
		i = 0;
		n = dataPointer -> msg;
		while(isprint(n[i])){
            i++;
        }
		n[i] = '\0';
        pointer = pointer -> next;
    }
}

void findData2(char *string, struct List *list, int sock){
    char *output = calloc(68, 1);
	struct Node *pointer = list -> head;
	int counter = 1;
	while (pointer != NULL){
		struct MdbRec *data = pointer -> data;
		//printf("%d%d ", strstr(data -> name, string) != NULL, strstr(data -> msg, string) != NULL);
		//printf("%s %s %s \n", data->name, data->msg, string);
		if (strstr(data -> name, string) != NULL||strstr(data -> msg, string) != NULL){
			if (counter < 10) {
                snprintf(output, 68, "   %d: {%s} said {%s} \n", counter, data -> name, data -> msg);
                send(sock, output, strlen(output), 0);
            }
            if (counter < 100 && counter >=10) {
                snprintf(output, 68, "  %d: {%s} said {%s} \n", counter, data -> name, data -> msg);
                send(sock, output, strlen(output), 0);
            }
			if (counter >= 100) {
                snprintf(output, 68, " %d: {%s} said {%s} \n", counter, data -> name, data -> msg);
                send(sock, output, strlen(output), 0);
            }
            //snprintf(*output, 64, "%d: {%s} said {%s} \n", counter, data -> name, data -> msg);
		}
		counter++;
		pointer = pointer -> next;
	}
    free(output);
}


void findData(char *string, struct List *list){
	struct Node *pointer = list -> head;
	int counter = 1;
	while (pointer != NULL){
		struct MdbRec *data = pointer -> data;
		//printf("%d%d ", strstr(data -> name, string) != NULL, strstr(data -> msg, string) != NULL);
		//printf("%s %s %s \n", data->name, data->msg, string);
		if (strstr(data -> name, string) != NULL||strstr(data -> msg, string) != NULL){
			if (counter < 10) printf("   ");
			if (counter < 100 && counter >=10) printf("  ");
			if (counter >= 100) printf(" ");
			printf("%d: {%s} said {%s} \n", counter, data -> name, data -> msg);
		}
		counter++;
		pointer = pointer -> next;
	}
}
void freeData(struct List *list){
	struct Node *pointer = list -> head;
	while (pointer != NULL){
        //struct Node *pointer = list -> head;
		free(pointer -> data);
		pointer = pointer -> next;
	}
}

static void die(const char *s) { perror(s); exit(1); }

int main(int argc, char **argv)
{
    // ignore SIGPIPE so that we don’t terminate when we call 
    // send() on a disconnected socket.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        die("signal() failed");

    // Process input

    if (argc != 3) {
        fprintf(stderr, "usage: %s <database> <server-port> \n", argv[0]);
        exit(1);
    }

    // Open file
    
    char *filename = argv[1];

    // Port number

    unsigned short port = atoi(argv[2]);

    

    // Create a listening socket (also called server socket) 

    int servsock;
    if ((servsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("socket failed");

    // Construct local address structure

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // any network interface
    servaddr.sin_port = htons(port);

    // Bind to the local address

    if (bind(servsock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("bind failed");

    // Start listening for incoming connections

    if (listen(servsock, 5 /* queue size for connection requests */ ) < 0)
        die("listen failed");

    int clntsock;
    socklen_t clntlen;
    struct sockaddr_in clntaddr;
    
    
    //int r;
    //char buf[4096];
    //uint32_t size, size_net, remaining, limit;
    //struct stat st;

    while (1) {

        // Accept an incoming connection

        clntlen = sizeof(clntaddr); // initialize the in-out parameter

        if ((clntsock = accept(servsock, (struct sockaddr *) &clntaddr, &clntlen)) < 0)
            die("accept failed");

        // Print starting message

        fprintf(stderr, "connection started from: %s\n", inet_ntoa(clntaddr.sin_addr));

        // Process socket descriptor

        FILE *input = fdopen(clntsock, "r");

        // Open file

        FILE *fp = fopen(filename, "rb");
        if (fp == NULL) 
            die(filename);

        //Create & Initialize a list

    	struct List list;
    	initList(&list);
    
    	//Read data into the list

        while (1){
    	    struct MdbRec *buffer = malloc(40);
	    	if (fread(buffer, 40, 1, fp) == 1){
	    		appendNode(&list, buffer);
	    	}else{
	    		free(buffer);
	    		break;
	    	}
        }
	
	    //Set the end of every string '\n'

	    setEnd(&list);
    
        //r = recv(clntsock, buf, 5, 0);
        char *in = calloc(1000, 1);

        while (fgets(in, 1000, input)){
        char *proc = calloc(6, 1);
            strncpy(proc, in, 5);
		    for(int i=0; i<6; i++){
		    	if(!isprint(proc[i])){
		    		proc[i] = '\0';
		    	}
		    }
            //char *out = calloc(1000, 1);
            //findData(proc, &list);
            findData2(proc, &list, clntsock);
            free(proc);
            send(clntsock, "\n", 1, 0);
        }  

        fclose(input);
        fclose(fp);
        free(in);
        //printf("%s", output);

        // Finally, close the client connection and go back to accept()

        close(clntsock);

        //Free everything
        freeData(&list);
        while ((popFront(&list)) != NULL) {
            ;
        }

        //Print termination message
        
        fprintf(stderr, "connection terminated from: %s\n", inet_ntoa(clntaddr.sin_addr));
    }

    
}
