hello:hello.o print.o
	gcc -o hello hello.o print.o

hello.o: hello.c print.h
	gcc -o hello.o -c hello.c

print.o: print.c print.h
	gcc -o print.o -c print.c

hello.i : hello.c print.h
	gcc -o hello.i -E hello.c

hello.asm : hello.c print.h
	gcc -o hello.asm -S -masm=intel hello.c

clean:
	rm hello *.i *.o *.asm