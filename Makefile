
ifneq ($(j),0)
	ifeq ($(j),)
		j=2
	endif
	MAKEFLAGS+="-j $(j)"
endif

flags=-g --std=c++17 -Wall

ifeq ($(lua),)
	lua=5.2
endif

ifeq ($(prof),)
	libs=$(shell pkg-config lua$(lua) --libs 2>/dev/null || echo -llua5.2)
	includes=$(shell pkg-config lua$(lua) --cflags)
else
	$(info profile build)
	libs=-L../gperftools-2.5/.libs/ -lprofiler ../lua-5.3.4/src/liblua.a
	includes=-I../lua-5.3.4/src
	bin=-profiled
	flags+=-Wl,--no-as-needed
endif

ifeq ($(lua),local)
	libs=../lua-5.3.4/src/liblua.a
	includes=-I../lua-5.3.4/src
	flags+=-Wl,--no-as-needed
endif

includes+=-I.
libs+=-lstdc++ -lstdc++fs -pthread -ldl

files=$(wildcard *.cpp)
files+=$(wildcard apis/*.cpp)
files+=$(wildcard components/*.cpp)
files+=$(wildcard io/*.cpp)
files+=$(wildcard drivers/*.cpp)
files+=$(wildcard color/*.cpp)
files+=$(wildcard model/*.cpp)
objs = $(files:%.cpp=bin/%$(bin).o)
deps = $(objs:%.o=%.d)

#ifeq (, $(shell which curl-config))
ifeq (, $(shell which wget))
	files := $(filter-out drivers/internet_http.cpp,$(files))
else
#includes+=$(shell curl-config --cflags)
#libs+=$(shell curl-config --libs)
endif

ocvm$(bin): system $(objs)
	$(CXX) $(flags) $(objs) $(libs) -o ocvm$(bin)
	@echo done

-include $(deps)
bin/%$(bin).o : %.cpp
	@mkdir -p $@D
	$(CXX) $(flags) $(includes) -MMD -c $< -o $@

system:
	@echo downloading OpenComputers system files
	mkdir -p system
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

