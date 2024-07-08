CC = mpicc
CFLAGS = -Wall -std=c99
LDFLAGS = -lm

.PHONY: all clean

all: pi_mpi

pi_mpi: pi_mpi.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: pi_mpi
	@echo "Running with 1 to 100 processes..."
	@echo "Results for running pi_mpi with 1 to 100 processes:" > ReadMe.txt
	@for i in $$(seq 1 100); do \
		echo "Processes: $$i" >> ReadMe.txt; \
		mpiexec --oversubscribe -n $$i ./pi_mpi 100 $$i >> ReadMe.txt; \
		echo >> ReadMe.txt; \
		echo >> ReadMe.txt; \
	done

clean:
	rm -f pi_mpi ReadMe.txt
