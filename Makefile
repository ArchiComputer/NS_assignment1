.PHONY: all clean

clean:
	rm -f udp_client/client udp_server/server

all: clean client server

client: udp_client/udp_client.c
	gcc udp_client/udp_client.c -o udp_client/client

server: udp_server/udp_server.c
	gcc udp_server/udp_server.c -o udp_server/server


