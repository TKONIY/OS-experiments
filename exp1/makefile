CC = gcc
STD = -std=gnu11
S = src/
B = build/

all: $(B)fork.out $(B)clone.out $(B)mergesort.out

$(B)fork.out: $(S)fork.c
	$(CC) $(STD) $(S)fork.c -o $(B)fork.out
$(B)clone.out: $(S)clone.c
	$(CC) $(STD) $(S)clone.c -o $(B)clone.out
$(B)mergesort.out: $(S)mergesort.c
	$(CC) $(STD) $(S)mergesort.c -o $(B)mergesort.out -lm -lpthread
.PHONY:
clean:
	rm -r $(B)*