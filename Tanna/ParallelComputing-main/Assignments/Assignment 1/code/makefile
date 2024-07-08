run:
	@gcc -pthread $(codefile) -lm -o algo.o -fsanitize=address
	@gcc -o generate.o generate.c
	@gcc -o test.o test.c
	@./generate.o $(count) input.txt
	@./algo.o input.txt output.txt $(threads)
	@./test.o output.txt
	@rm input.txt
	@rm output.txt
	@rm algo.o
	@rm generate.o
	@rm test.o


clean:
	@rm input.txt || true
	@rm output.txt || true
	@rm algo.o || true
	@rm generate.o || true
	@rm test.o || true
