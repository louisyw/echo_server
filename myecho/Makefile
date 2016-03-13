object=main.o rio.o sbuf.o
echo:$(object)
	gcc -o echo $(object) -lpthread

main.o:rio.h sbuf.h
rio.o:rio.h
sbuf.o:sbuf.c sbuf.h

.PHONY:clean
clean:
	rm echo $(object)
