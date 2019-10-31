all: startClient startServer

startClient:
	gcc -pthread -o startClient client.c

startServer:
	gcc -o startServer server.c

clean:
	rm -rf startClient startServer

