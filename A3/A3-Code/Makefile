mysh: shell.c interpreter.c shellmemory.c PCB.c
	gcc -D FSIZE=$(framesize) -D VSIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c PCB.c
	gcc -g -o mysh shell.o interpreter.o shellmemory.o PCB.o

clean: 
	rm mysh; rm *.o
