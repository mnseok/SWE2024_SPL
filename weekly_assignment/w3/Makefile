p1.out: main.o plus.o minus.o
	gcc -o p1.out plus.o minus.o main.o
	mv main.o bin
	mv plus.o bin
	mv minus.o bin
	mv p1.out bin

main.o: src/main.c
	gcc -c src/main.c

plus.o: src/plus.c
	gcc -c src/plus.c

minus.o: src/minus.c
	gcc -c src/minus.c

clean: 
	rm bin/*
