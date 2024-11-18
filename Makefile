CC       = gcc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DDEBUG
CFLAGS   = -I/mingw64/include -std=c99 -pedantic -Wall -Wextra -g -O0
LDFLAGS  = -L/mingw64/lib -municode -mwindows -lvulkan-1
DEPFLAGS = -MMD -MP -MT $@ -MF $(DEPDIR)/$*.d
DEPDIR   = .dep

BIN = vulkan_triangle.exe
SRC = vulkan_debug.c vulkan_device.c vulkan_imageview.c vulkan_instance.c \
      vulkan_physicaldevice.c vulkan_surface.c vulkan_swapchain.c win32.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:%.c=$(DEPDIR)/%.d)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c Makefile $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) $<

$(DEPDIR):
	@mkdir -p $@

$(DEP):

include $(wildcard $(DEP))

clean:
	@rm -f $(BIN) $(OBJ)
	@rm -rf $(DEPDIR)

run:	$(BIN)
	@./$(BIN)

.PHONY:	all clean run
