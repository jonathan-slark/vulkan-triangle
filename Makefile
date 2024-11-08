.POSIX:

CC      = gcc
CFLAGS  = -I/mingw64/include -Wall -Wextra -g
LDFLAGS = -L/mingw64/lib -municode -mwindows -lvulkan-1.dll
SRC     = win32.c vulkan_instance.c
OBJ     = $(SRC:.c=.o)
BIN     = vulkan_test.exe

all:	$(BIN)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

clean:
	@rm -f $(OBJ) $(BIN)

run:	$(BIN)
	@./$(BIN)

.PHONY:	all clean run
