all:	morse

morse:	morse.c morse.h
	gcc -o morse morse.c

clean:
	rm -f *.o morse
