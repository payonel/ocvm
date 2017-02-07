# ocvm
OpenComputer Emulator

This emulator is inspired by the outstanding work done by gamax with OCEmu ( https://github.com/gamax92/OCEmu ).

This project will not be nearly as impressive as OCEmu, nonetheless I embark.

The main goals of this projects are

1. Learn lua binding and environment emulation from a C++ application.
2. Provide memory and cpu profiling.
3. Create an emulator that runs fully in the terminal

There are some clear disadvantages to the approach I'm taking for this project, to name just a few

1. The terminal graphics layer depends on ncurses, not quite cross platform
2. I am building the C++ binding and (de)serialization for all lua callback and arguments by hand (error-prone)
3. My emulator will not natively support the font.hex from opencomputers (I'll probably have to emulator that)

But this should be a lot of fun!

Requirements to build:
1. (incomplete) At some point setup will require download opencomputer source into the right places.
2. g++ with c++11 support
3. ncurses
4. lua.hpp and lua5.2-c++ (todo: update for 5.3)

Then run `make` in the root dir of the project, and `./ocvm`

