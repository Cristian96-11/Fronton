
fronton0 : fronton0.c winsuport.o winsuport.h
	gcc -Wall fronton0.c winsuport.o -o fronton0 -lcurses

fronton1 : fronton1.c winsuport.o winsuport.h
	gcc -Wall fronton1.c winsuport.o -o fronton1 -lcurses -lpthread

winsuport.o : winsuport.c winsuport.h
	gcc -c -Wall winsuport.c -o winsuport.o 

run:
	./fronton0 prova1.txt 50
	
run2:
	./fronton0 prova2.txt 50

run3:
	./fronton0 prova3.txt 50

clean:
	rm *.o fronton0
	
run_hilo_f1:
	./fronton1 prova1.txt 50