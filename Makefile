.POSIX:

CC       = gcc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DDEBUG -DVK_USE_PLATFORM_WIN32_KHR
CFLAGS   = -I/mingw64/include -std=c99 -pedantic -Wall -Wextra -g -O0
LDFLAGS  = -L/mingw64/lib -mwindows -lvulkan-1
GLSLC    = glslc

BIN = triangle.exe
SRC = util.c vulkan.c win32.c
OBJ = $(SRC:.c=.o)

GLSL = shaders/vertex.glsl shaders/fragment.glsl
SPV  = $(GLSL:.glsl=.spv)

all: $(BIN) $(SPV)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

%.spv: %.glsl
	$(GLSLC) $< -o $@

vulkan.c win32.c: config.h util.h vulkan.h win32.h

clean:
	@rm -f $(BIN) $(OBJ) $(SPV)

run:	$(BIN)
	@./$(BIN)

.PHONY:	all clean run
