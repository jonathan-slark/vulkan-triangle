.POSIX:

CC      = gcc
CFLAGS  = -I/mingw64/include -Wall -Wextra -g -DDEBUG
LDFLAGS = -L/mingw64/lib -municode -mwindows -lvulkan-1

BIN = vulkan_test.exe
SRC = vulkan_debug.c vulkan_device.c vulkan_instance.c win32.c
OBJ = ${SRC:.c=.o}

all: ${BIN}

${BIN}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

%.o: %.c Makefile
	${CC} -c ${CFLAGS} $<

vulkan_debug.o:    vulkan_debug.c vulkan_debug.h
vulkan_device.o:   vulkan_device.c vulkan_device.h vulkan_instance.h
vulkan_instance.o: vulkan_instance.c vulkan_debug.h vulkan_device.h \
                   vulkan_instance.h
win32.o:           win32.c vulkan_instance.h

clean:
	@rm -f ${BIN} ${OBJ}

run:	${BIN}
	@./${BIN}

.PHONY:	all clean run
