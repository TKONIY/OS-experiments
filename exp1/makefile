CC = gcc
STD = -std=gnu11
S = src/
B = build/

all: $(B)fork.out $(B)clone.out $(B)mergesort.out $(B)reader_fist_rw.out $(B)sleep_barber.out

$(B)fork.out: $(S)fork.c
	$(CC) $(STD) $(S)fork.c -o $(B)fork.out
$(B)clone.out: $(S)clone.c
	$(CC) $(STD) $(S)clone.c -o $(B)clone.out
$(B)mergesort.out: $(S)mergesort.c
	$(CC) $(STD) $(S)mergesort.c -o $(B)mergesort.out -lm -lpthread
$(B)reader_fist_rw.out: $(S)reader_fist_rw.c
	$(CC) $(STD) $(S)reader_fist_rw.c -o $(B)reader_fist_rw.out -lm -lpthread
$(B)sleep_barber.out: $(S)sleep_barber.cpp
	g++ -std=c++11 $(S)sleep_barber.cpp -o $(B)sleep_barber.out -lpthread
.PHONY:
clean:
	rm -r $(B)*