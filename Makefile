all:
	 gcc -g -nostdinc++ -std=gnu17 -o main tag_type.c main.c -lfort
run:
	./main image.CR2
