# Variables
CC = gcc
CFLAGS = -Wall -g
TARGET = main
SRC = tensor.c
OBJ = $(SRC:.c=.o)

# Default rule
all: $(TARGET)

# Build target from object files
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)
