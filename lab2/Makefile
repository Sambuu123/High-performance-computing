CC = gcc
SRC = auto_vec_add.c
CFLAGS = -Wall -Wextra -std=c11

.PHONY: all run vec-info clean

all: scalar_add auto_vec

# Скаляр: ижил -O3, векторжуулалт хаалттай.
scalar_add: $(SRC)
	$(CC) $(CFLAGS) -O3 -fno-tree-vectorize -fno-tree-slp-vectorize -fno-unroll-loops $(SRC) -o scalar_add

# Вектор: лабораторийн заавар.
auto_vec: $(SRC)
	$(CC) $(CFLAGS) -O3 -march=native $(SRC) -o auto_vec

vec-info:
	$(CC) $(CFLAGS) -O3 -march=native -fopt-info-vec-optimized $(SRC) -o auto_vec

run: all
	@echo "=== SCALAR ==="
	./scalar_add
	@echo "=== VECTOR ==="
	./auto_vec

clean:
	rm -f scalar_add auto_vec
