CC = g++
#test : test.o kfifol.o
#	$(CC) -g -o test test.o kfifol.o -lpthread 
#test.o : test.cpp kfifol.h
#	$(CC) -c test.cpp
#kfifo.o : kfifol.cpp kfifol.h
#	$(CC) -c kfifol.cpp

Target = test
all: $(Target)
rc=$(wildcard *.cpp )
bj=$(patsubst %.cpp,%.o,$(rc))
$(Target) : $(bj)
	$(CC)  -o $@ $^ -lpthread
#$(bj) : $(rc)
%.o : %.cpp
	$(CC) -g -c $< -o $@
clean :
	@echo $(bj)
	@echo $(rc)
	rm test.o kfifol.o $(Target)
