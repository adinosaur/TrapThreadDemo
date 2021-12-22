
a.out: main.cpp
	g++ main.cpp --std=c++11 -g -lpthread

clean:
	rm a.out