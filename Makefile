CFLAGS=-Wall -Wextra -Werror -Wpedantic
CC=clang $(CFLAGS)

main   :	main.o	
	clang -o main main.o trie.o word.o io.o -lm
main.o :	main.c
	$(CC) -c main.c 
	$(CC) -c trie.c 
	$(CC) -c word.c
	$(CC) -c io.c
all	:
	$(CC) -c main.c 
	$(CC) -c trie.c 
	$(CC) -c word.c
	$(CC) -c io.c
	clang -o main main.o trie.o word.o io.o -lm
clean   :
	rm main main.o trie.o word.o io.o infer-out
infer   :
	make clean; infer-capture -- make; infer-analyze -- make
format  :
	clang-format -i -style=file *.c

