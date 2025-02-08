#!/bin/bash

# 定义样本编号
samples=("C1" "C2" "C3")
g++ -std=c++14 3036128004-C.cpp -o 3.o
g++ -std=c++14 3035973799-C.cpp -o 4.o
g++ -std=c++14 3036128846-C.cpp -o 5.o
g++ -std=c++14 3036128690-C.cpp -o 6.o

# 循环遍历每个样本
for sample in "${samples[@]}"
do
    echo "Testing sample $sample..."
    ./5.o< sample$sample.in > myoutput$sample.txt
    if diff -cw myoutput$sample.txt sample$sample.ans > differences$sample.txt; then
        echo "Sample $sample: PASS"
    else
        echo "Sample $sample: FAIL"
    fi
done