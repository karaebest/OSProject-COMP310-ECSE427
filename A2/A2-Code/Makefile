mysh: shell.c interpreter.c shellmemory.c PCB.c
	gcc -c shell.c interpreter.c shellmemory.c PCB.c
	gcc -o mysh shell.o interpreter.o shellmemory.o PCB.o

clean: 
	rm mysh; rm *.o
