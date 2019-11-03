all:
	 gcc -g -std=gnu17 -o main tag_type.c main.c -lfort
	 valgrind -s ./main image.CR2
run:
	./main image.CR2


