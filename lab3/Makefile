CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -fopenmp
SRC = lab3.c timer.c

.PHONY: all run bench plot clean

all: lab3 lab3_O0

# Үндсэн хувилбар: лабораторийн шаардлагаар -O3.
lab3: $(SRC) timer.h
	$(CC) $(CFLAGS) -O3 $(SRC) -o lab3

# Асуулт 6 (нэмэлт): B2-ийг -O0 дээр харьцуулах.
lab3_O0: $(SRC) timer.h
	$(CC) $(CFLAGS) -O0 $(SRC) -o lab3_O0

run: lab3
	OMP_NUM_THREADS=4 ./lab3

# 1, 2, 4, 8 урсгалаар бүх хэмжилт → results/results.csv
bench: all
	./run_benchmarks.sh

# Хүснэгт ба 2 график → results/
plot:
	.venv/bin/python analyze.py

clean:
	rm -f lab3 lab3_O0
