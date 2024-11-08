.POSIX:

CC      = gcc
CFLAGS  = -ansi -Wall -Wextra -g
LDFLAGS = -municode
SRC     = win32.c
OBJ     = $(SRC:.c=.o)
BIN     = vulkan.exe

all:	$(BIN)

%.o:	%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	@$(CC) $(LDFLAGS) $(OBJ) -o $(BIN)

clean:
	@rm -f $(OBJ) $(BIN)

run:	$(BIN)
	@./$(BIN)

.PHONY:	all clean run
