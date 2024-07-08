CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -fopenmp
LIBS=-lm

SRC=matrix_inverse_openmp.c
EXEC=matrix_inverse_openmp

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

run: $(EXEC)
	./$(EXEC)

clean:
	rm -f $(EXEC)

.PHONY: all run clean
