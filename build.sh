#!/bin/bash -x
mkdir -p bin &&
rm -f bin/ocpc &&
rm -f bin/*.o &&
g++ -O0 -g --std=c++11 -lstdc++ -Wl,--no-as-needed -c main.cpp -o bin/main.o &&
g++ -g --std=c++11 bin/*.o -lstdc++ -Wl,--no-as-needed -o bin/ocpc &&
echo done || echo failed

