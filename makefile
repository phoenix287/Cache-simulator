
cacheSim: *.cpp
	g++ -std=c++11 -o cacheSim *.cpp

.PHONY: clean
clean:
	rm -f *.o
	rm -f cacheSim
