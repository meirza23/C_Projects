main:                                                    
	gcc -c main.c reader.c                                 
	gcc main.o reader.o -o myreader

clean:
	rm *.o
	rm myreader