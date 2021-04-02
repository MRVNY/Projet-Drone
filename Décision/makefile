all: decision


decision.o : decision.c
	gcc -o decision.o -c decision.c 


decision : decision.o
	gcc -o decision decision.o 

clean:
	rm -rf *.o