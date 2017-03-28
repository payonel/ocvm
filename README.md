# ocvm
OpenComputer Emulator

This emulator is inspired by the outstanding work done by gamax with OCEmu ( https://github.com/gamax92/OCEmu ).

This project will not be nearly as impressive as OCEmu, nonetheless I embark.

The main goals of this projects are

1. Learn lua binding and environment emulation from a C++ application.
2. Provide memory and cpu profiling for debugging operating systems for OpenComputers
3. Create an emulator that runs fully in the terminal

There are some clear disadvantages to the approach I'm taking for this project, to name just a few

1. The terminal graphics layer depends on ansi escape codes, not quite cross platform
2. I am building the C++ binding and (de)serialization for all lua callback and arguments by hand (error-prone)
3. Displayed font will be host machine fonts using utf8 encoding, I will not be rendering OC fonts

But this should be a lot of fun!

How to Build:

Just run `make` in the source root. If successful, you should have a `ocvm` binary. Run it using `./ocvm`

There are some requirements for building:
1. svn: The Makefile will download OpenComputers source files
    If you do have svn available, or prefer to prepare the system/ dir by hand, from the source root:
    1. mkdir system
    2. `wget https://raw.githubusercontent.com/MightyPirates/OpenComputers/master-MC1.7.10/src/main/resources/assets/opencomputers/lua/machine.lua -O system/machine.lua`
    3. `wget https://raw.githubusercontent.com/MightyPirates/OpenComputers/master-MC1.7.10/src/main/resources/assets/opencomputers/lua/bios.lua -O system/bios.lua`
    4. `wget https://raw.githubusercontent.com/MightyPirates/OpenComputers/master-MC1.7.10/src/main/resources/assets/opencomputers/font.hex -O system/font.hex`

2. g++ 5.4 with c++14 support (experimental/filesystem is used)
3. lua5.2 (`make lua=5.3` to use lua5.3)
4. A vt100 compatible terminal

Future Scope:

I plan to add support for building ocvm on Mac using boost filesystem and clang+llvm
I do not plan to add support for non-ansi terminals nor windows. If this works in cygwin it wasn't on purpose

