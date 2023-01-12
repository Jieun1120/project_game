CC = gcc
SOURCES = game_server.c ControlDB.c
OBJECTS = ${SOURCES:.c=.o}
CFLAGS=-I /usr/include/mysql -Wall -g
LDFLAGS=-lmysqlclient -g
RM = rm -f
OUT = GAME_SERVER
$(OUT): $(OBJECTS)
	$(CC) -o $(OUT) $(OBJECTS) $(LIBS) $(CFLAGS) $(LDFLAGS)
clean:
	$(RM) $(OUT) *.o
