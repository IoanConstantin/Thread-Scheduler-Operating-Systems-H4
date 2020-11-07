all: build

build:
	gcc -Wall -g -fPIC -c scheduler.c -lpthread
	gcc -Wall -g -shared scheduler.o -o libscheduler.so

clean:
	rm -f *.o