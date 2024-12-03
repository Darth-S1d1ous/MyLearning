#!/bin/bash

# 定义样本编号
samples=("C1" "C2" "C3")
g++ -std=c++14 3036128004-C.cpp -o 3.o

# 循环遍历每个样本
for sample in "${samples[@]}"
do
    echo "Testing sample $sample..."
    ./3.o< sample$sample.in > myoutput$sample.txt
    if diff -w myoutput$sample.txt sample$sample.ans > /dev/null; then
        echo "Sample $sample: PASS"
    else
        echo "Sample $sample: FAIL"
    fi
done