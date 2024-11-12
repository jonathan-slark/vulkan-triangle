CC       = gcc
CFLAGS   = -I/mingw64/include -Wall -Wextra -g -DDEBUG
LDFLAGS  = -L/mingw64/lib -municode -mwindows -lvulkan-1
DEPFLAGS = -MM
DEPFILE  = .dep

BIN = vulkan_test.exe
SRC = vulkan_debug.c vulkan_device.c vulkan_instance.c win32.c
OBJ = ${SRC:.c=.o}

all: ${BIN}

${BIN}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

%.o: %.c Makefile ${DEPFILE}
	${CC} -c ${CFLAGS} $<

${DEPFILE}: ${SRC}
	${CC} ${DEPFLAGS} ${SRC} > $@

include ${DEPFILE}

clean:
	@rm -f ${BIN} ${OBJ} ${DEPFILE}

run:	${BIN}
	@./${BIN}

.PHONY:	all clean run
