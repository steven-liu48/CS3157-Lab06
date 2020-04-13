http-client: http-client.o
	gcc -g -Wall http-client.o -o http-client

http-client.o: http-client.c
	gcc -c -g -Wall http-client.c

.PHONY: vtest
vtest: http-client
	valgrind --leak-check=yes ./http-client www.gnu.org 80 /software/make/manual/make.html

.PHONY: clean
clean:
	rm -f *.o *.a *.html *.jpg http-client
