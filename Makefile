
fronton2 : fronton2.c winsuport.o winsuport.h
	gcc -Wall fronton2.c winsuport.o -o fronton2 -lcurses -lpthread


winsuport.o : winsuport.c winsuport.h
	gcc -c -Wall winsuport.c -o winsuport.o 

run:
	./fronton2 prova1.txt 2 50
	
run2:
	./fronton2 prova2.txt 4 50

run3:
	./fronton2 prova3.txt 10 50

clean:
	rm *.o fronton2
	