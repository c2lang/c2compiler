CFLAGS=-Wall -Wextra -Wno-unused -Wno-unused-parameter -Wshadow -O2

tester: main.cpp
	g++ $(CFLAGS) main.cpp FileMap.cpp StringBuilder.cpp -o tester
	strip tester

clean:
	rm -f *.o tester


