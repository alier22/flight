CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lrt

SERVER_SRC = flight_server.c
CLIENT_SRC = flight_client.c
HELPER_SRC = helper.c

SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
HELPER_OBJ = $(HELPER_SRC:.c=.o)

SERVER_BIN = flight_server
CLIENT_BIN = flight_client
HELPER_BIN = helper

all: $(SERVER_BIN) $(CLIENT_BIN) $(HELPER_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
    $(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_OBJ)
    $(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(HELPER_BIN): $(HELPER_OBJ)
    $(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(HELPER_OBJ) $(SERVER_BIN) $(CLIENT_BIN) $(HELPER_BIN)
