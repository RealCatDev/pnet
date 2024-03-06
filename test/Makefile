CC := gcc
CFLAGS :=
LDFLAGS := -L../bin/ -lpnet -lws2_32

.PHONY: clean all

all: server client

clean:
	@rm -rf server.exe client.exe

server: server.c
	@$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -I../include/

client: client.c
	@$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -I../include/