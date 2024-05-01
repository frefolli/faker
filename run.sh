#!/bin/bash
for i in $(seq 0 99);
do
    ./builddir/main.exe $i 100;
done
