CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm
SRC = main.c network.c
OBJ = $(SRC:.c=.o)
EXEC = network

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(EXEC)
