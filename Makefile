# A very simple Makefile
all: build read_schar

build:
	mkdir -p build

build/%.o: src/%.cc 
	g++ -DUSE_DDU2004 -c -o $@ $< -I inc

read_schar : build/read_schar.o build/Base.o build/Spy.o build/Clock.o
	g++ -o $@ $^

.PHONY: clean

clean:
	rm -f build/*.o read_schar
	rmdir build
