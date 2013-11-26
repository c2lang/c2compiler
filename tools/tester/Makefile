CFLAGS=-Wall -Wextra -Wno-unused -Wshadow -O2

tester: main.cpp
	g++ $(CFLAGS) main.cpp FileMap.cpp StringBuilder.cpp -lrt -o tester
	strip tester

clean:
	rm -f *.o tester


