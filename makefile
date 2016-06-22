testshell: parse.o shell.o debug.o MyReadLine.o
	gcc shell.o parse.o debug.o MyReadLine.o -o naiveshell -lreadline
shell.o: shell.c header.h
	gcc -c shell.c
parse.o: parse.c header.h
	gcc -c parse.c
debug.o: debug.c
	gcc -c debug.c
MyReadLine.o: MyReadLine.c
	gcc -c MyReadLine.c
