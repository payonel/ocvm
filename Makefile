MAKEFLAGS+="-j 4"
flags=-g --std=c++14 -Wall

ifeq ($(prof),)
libs=-llua5.2
includes=-I/usr/local/include/lua5.2/
else
$(info profile build)
libs=-L../gperftools-2.5/.libs/ -ldl -lprofiler ../lua-5.3.4/src/liblua.a
includes=-I../lua-5.3.4/src
bin=-profiled
flags+=-Wl,--no-as-needed
endif

ifeq ($(lua),5.3)
libs=-llua5.3
includes=-I/usr/local/include/lua5.3/
endif

includes+=-I.
libs+=-lstdc++ -lstdc++fs -pthread

files=$(wildcard *.cpp)
files+=$(wildcard apis/*.cpp)
files+=$(wildcard components/*.cpp)
files+=$(wildcard io/*.cpp)
files+=$(wildcard drivers/*.cpp)
files+=$(wildcard color/*.cpp)
objs = $(files:%.cpp=bin/%$(bin).o)
deps = $(objs:%.o=%.d)

ocvm$(bin): $(objs) system
	$(CXX) $(flags) $(objs) $(libs) -o ocvm$(bin)
	@echo done

-include $(deps)
bin/%$(bin).o : %.cpp
	@mkdir -p $@D
	$(CXX) $(flags) $(includes) -MMD -c $< -o $@

system:
	@echo downloading OpenComputers system files
	mkdir system
	svn checkout https://github.com/MightyPirates/OpenComputers/trunk/src/main/resources/assets/opencomputers/loot system/loot
	wget https://raw.githubusercontent.com/MightyPirates/OpenComputers/master-MC1.7.10/src/main/resources/assets/opencomputers/lua/machine.lua -O system/machine.lua
	wget https://raw.githubusercontent.com/MightyPirates/OpenComputers/master-MC1.7.10/src/main/resources/assets/opencomputers/lua/bios.lua -O system/bios.lua
	wget https://raw.githubusercontent.com/MightyPirates/OpenComputers/master-MC1.7.10/src/main/resources/assets/opencomputers/font.hex -O system/font.hex

.PHONY : clean
clean:
	@echo rm -rf bin ocvm ocvm-profiled

ifneq ($(filter clean,$(MAKECMDGOALS)),)
	$(shell rm -rf bin ocvm ocvm-profiled)
endif

