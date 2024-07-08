CC=gcc
CFLAGS=-Wall -Wextra -pthread

barrier_pthreads: barrier_pthreads.c
	$(CC) $(CFLAGS) -o barrier_pthreads barrier_pthreads.c

run: barrier_pthreads
	./barrier_pthreads | tee ReadMe.txt

clean:
	rm -f barrier_pthreads
