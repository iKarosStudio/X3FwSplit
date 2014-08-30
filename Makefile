


OBJS:= Main.o X3FwSplit.o

X3Split : $(OBJS)
	gcc $(OBJS) -o X3Split

Main.o : Main.c
	gcc -c Main.c

X3FwSplit.o : X3FwSplit.c
	gcc -c X3FwSplit.c
