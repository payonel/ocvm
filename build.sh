#!/bin/bash
mkdir -p bin &&
rm -rf bin/* &&
mkdir -p bin/components/ &&
rm -f ocpc &&
g++ -O0 -g --std=c++11 -lstdc++ -Wl,--no-as-needed -c main.cpp -o bin/main.o &&
for d in ./ components/; do
    echo building $d
    for h in $(ls ${d}*.h); do
        f=${h/.h}
        cpp=${f}.cpp
        if [ -e $cpp ]; then
            echo CPP $cpp
            g++ -g -I. -I/usr/include/lua5.2/ --std=c++11 -lstdc++ -Wl,--no-as-needed -c $cpp -o bin/${f}.o || exit
        fi
    done
done &&
echo linking
g++ -g --std=c++11 bin/*.o bin/components/*.o -llua5.2-c++ -lstdc++ -Wl,--no-as-needed -o ocpc &&
echo done || echo failed

