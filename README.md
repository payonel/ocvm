# ocvm
OpenComputer Emulator

This emulator is inspired by the outstanding work done by gamax with OCEmu ( https://github.com/gamax92/OCEmu ).

This project will not be nearly as impressive as OCEmu, nonetheless I embark.

**Project Goals**

1. Learn lua binding and environment emulation from a C++ application.
2. Provide memory and cpu profiling for debugging operating systems for OpenComputers
3. A purely command line emulator (e.g. over-ssh support)

**Expected Disadvantages**

1. The terminal graphics layer depends on ansi escape codes, not quite cross platform
2. I am building the C++ binding and (de)serialization for all lua callback and arguments by hand (error-prone)
3. Displayed font will be host machine fonts using utf8 encoding, I will not be rendering OC fonts
4. pty applications do not get all key events, such as no key releases. I can emulate key releases some of the time, but not all of the time.

But this should be a lot of fun!

**How to Build**

Run `make deps` at least once to download OpenComputers machine system files. These files are cached and not deleted by `make clean`. Delete the temporary `system` directory manually if you want `make deps` to redownload them

Run `make` to build

Run `./ocvm`

1. tools required: c++ compiler and lua sdk files
2. c++17 support
3. pkg-config defaults to `lua`. If your system has a different lua pkg name, you can override the default with `make lua=lua5.3`. If you are unsure, see your systems lua packages with `pkg-config --list-all`.
4. A vt100 compatible terminal

**Custom Build Overrides**

* lua=lua
  - You can override the lua pkg config with 'lua' environment variable: e.g. `make lua=lua5.2`

* CXX=g++
  - You can override the lua pkg config with 'lua' environment variable: e.g. `make CXX=g++-12`

**Future Scope**

I plan to add support for building ocvm on Mac using boost filesystem
I do not plan to add support for non-ansi terminals nor windows. If this works in cygwin it wasn't on purpose

