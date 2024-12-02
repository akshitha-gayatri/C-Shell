# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Linker flags
LDFLAGS =

# Executable name
TARGET = a.out

# Source files
SRCS = main.c bgfg.c pipe_redir.c signal.c hop.c reveal.c list.c log.c seek.c proc.c iMan.c neonate.c activities.c bg.c fg.c my.c

# Header files
HEADERS = bgfg.h initialise.h pipe_redir.h seek.h hop.h reveal.h list.h log.h proc.h signal.h iMan.h neonate.h bg.h fg.h my.h

# Object files
OBJS = $(SRCS:.c=.o)

# Default rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Rule to build object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: clean
