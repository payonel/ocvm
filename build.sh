#!/bin/bash -x
mkdir -p bin &&
rm -rf bin/* &&
rm -f ocpc &&
g++ -O0 -g --std=c++11 -lstdc++ -Wl,--no-as-needed -c main.cpp -o bin/main.o &&
for h in $(ls *.h); do f=${h/.h} && echo CPP ${f}.cpp && g++ -g -I/usr/include/lua5.2/ --std=c++11 -lstdc++ -Wl,--no-as-needed -c ${f}.cpp -o bin/${f}.o; done &&
g++ -g --std=c++11 bin/*.o -llua5.2-c++ -lstdc++ -Wl,--no-as-needed -o ocpc &&
echo done || echo failed

