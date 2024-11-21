.POSIX:

CC       = gcc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DDEBUG -DUNICODE \
	   -DVK_USE_PLATFORM_WIN32_KHR
CFLAGS   = -I/mingw64/include -std=c99 -pedantic -Wall -Wextra -g -O0
LDFLAGS  = -L/mingw64/lib -municode -mwindows -lvulkan-1

BIN = triangle.exe
SRC = vulkan.c win32.c
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

vulkan.c: config.h win32.h
win32.c: config.h util.h vulkan.h win32.h

clean:
	@rm -f $(BIN) $(OBJ)

run:	$(BIN)
	@./$(BIN)

.PHONY:	all clean run
