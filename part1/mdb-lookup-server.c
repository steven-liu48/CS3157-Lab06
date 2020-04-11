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
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) 
        die(filename);

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
    
    
    int r;
    char buf[4096];
    //uint32_t size, size_net, remaining, limit;
    //struct stat st;

    //while (1) {

        // Accept an incoming connection

        clntlen = sizeof(clntaddr); // initialize the in-out parameter

        if ((clntsock = accept(servsock, (struct sockaddr *) &clntaddr, &clntlen)) < 0)
            die("accept failed");

        // Print ip address

        fprintf(stderr, "client ip: %s\n", inet_ntoa(clntaddr.sin_addr));

        // Process socket descriptor

        FILE *input = fdopen(clntsock, "r");

        //Create & Initialize a list

    	struct List list;
    	initList(&list);
    
    	//Read data into the list


       while (1){
    	    struct MdbRec *buffer = malloc(40);
	    	if (fread(buffer, 40, 1, fp) == 1){
	    		//printf("%s ", buffer -> name);
	            //printf("%s \n", buffer -> msg);
	    		appendNode(&list, buffer);
	    	}else{
	    		free(buffer);
                printf("finished reading\n");
	    		break;
	    	}
	    	//free(buffer);
        }
	
	    //Set the end of every string '\n'

	    setEnd(&list);
    
        //r = recv(clntsock, buf, 5, 0);
        char *proc = calloc(6, 1);
        fgets(proc, 6, input);
        fclose(input);
        
		//strncpy(proc, buf, 5);
        
		for(int i=0; i<6; i++){
			if(!isprint(proc[i])){
				proc[i] = '\0';
			}
		}
		
        //printf("print: %s", proc);
        
		findData(proc, &list);
        free(proc);
        

        // Finally, close the client connection and go back to accept()

        close(clntsock);
        printf("Loop Ended");

        //Free everything
        freeData(&list);
        while ((popFront(&list)) != NULL) {
            ;
        }
    //}

    
}
