TARGET  = plexer
CC      = gcc
# COMMIT  = $(shell git rev-parse HEAD)
COMMIT  = $(shell echo test)
CFLAGS  = -Wpedantic -Wextra -Wunreachable-code -Wunused -Wall -I. -Wno-initializer-overrides -Wno-missing-field-initializers -Wno-unused-parameter -Wno-missing-braces -Wno-gnu -D PLXR_COMMIT_HASH=$(COMMIT)
OBJ     = main.o http_state.o http_request.o connection.o connection_list.o connection_loop.o socket.o
HEADERS =        http_state.h http_request.h connection.h connection_list.h connection_loop.h socket.h log.h version.h
MAIN    = http_parse.o

# where plexer will be installed
INSTALL_DEST = /usr/local/bin

.PHONY: default all clean test install uninstall rebuild

default: rebuild
all: $(TARGET) $(OBJ) test

install: $(TARGET)
	chmod 755 ./$(TARGET)
	cp ./$(TARGET) $(INSTALL_DEST)/$(TARGET)

uninstall:
	rm $(INSTALL_DEST)/$(TARGET)

test: $(HEADERS) $(OBJ)
	$(MAKE) $(OBJ)
	cd test && $(MAKE) test
	./test/test

run: $(TARGET)
	./$(TARGET)

rebuild: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $@

debug: $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -g -o $(TARGET)-debug

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) $(TARGET)-debug
	$(RM) -r $(TARGET)-debug.dSYM
	$(RM) $(TARGET)