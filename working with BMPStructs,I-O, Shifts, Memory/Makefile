main: main.c lab4bmp.h
	gcc -Wall main.c -o main
main.o: 
	gcc -Wall -c main.c
test: main.c lab4bmp.h test.c
	gcc -Wall -o test test.c
clear:
	rm -f *.o main test