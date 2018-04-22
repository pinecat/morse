all:	morse

morse:	morse.c morse.h
	gcc -ggdb -o morse morse.c -Wall

clean:
	rm -f *.o morse
