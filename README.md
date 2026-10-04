# Parallel & High Performance Computing (HPC) - Labs

Энэхүү репозитори нь **Зэрэгцээ ба Өндөр Гүйцэтгэлтэй Тооцоолол (Parallel & High Performance Computing)** хичээлийн хүрээнд хийгдсэн 7 лабораторийн ажлын код, судалгаа болон туршилтын үр дүнгүүдийг багтаасан болно.

---

## 📁 Төслийн бүтэц

```text
.
├── Lab-01/          # Lab 1: Процессор ба Кэш санах ойн судалгаа
├── Lab-02/          # Lab 2: Векторжуулалт (Auto-Vectorization / SIMD)
│   ├── auto_vec_add.c
│   └── README.md
├── Lab-03/          # Lab 3: OpenMP Зэрэгцээ боловсруулалт
├── Lab-04/          # Lab 4: OpenMP Ачаалал хуваарилалт ба Оновчлол
├── Lab-05/          # Lab 5: MPI Тархмал санах ойн програмчлал
├── Lab-06/          # Lab 6: Hybrid Programming (MPI + OpenMP)
└── Lab-07/          # Lab 7: GPU Acceleration (CUDA / OpenCL)

## Environment 

sudo apt update
sudo apt install build-essential gcc g++ make cmake
sudo apt install openmpi-bin libopenmpi-dev


# 1. Репозиторийг клон хийж авах
git clone [https://github.com/YOUR_USERNAME/YOUR_REPOSITORY_NAME.git](https://github.com/YOUR_USERNAME/YOUR_REPOSITORY_NAME.git)
cd YOUR_REPOSITORY_NAME

# 2. Lab-02 хавтас руу шижих
cd Lab-02

# 3. Скаляр хувилбарыг хөрвүүлэх (Векторжуулалтгүй)
gcc -O3 -fno-tree-vectorize -fno-tree-slp-vectorize -fno-unroll-loops auto_vec_add.c -o scalar_add

# 4. Вектор хувилбарыг хөрвүүлэх (SIMD идэвхжүүлсэн)
gcc -O3 -march=native auto_vec_add.c -o auto_vec

# 5. Туршилтыг ажиллуулах
./scalar_add
./auto_vec
