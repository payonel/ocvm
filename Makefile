flags=-g --std=c++11 -Wl,--no-as-needed -Wall

ifeq ($(prof),)
includes=-I. -I/usr/include/lua5.2/
libs=-llua5.2-c++ -lstdc++ -lncurses -lstdc++fs
else
$(info profile build)
libs=-L../gperftools-2.5/.libs -L../lua-5.3.4/src -llua -lstdc++ -lncurses -lstdc++fs -lprofiler
includes=-I. -I../lua-5.3.4/src
bin=-profiled
endif

files = $(wildcard *.cpp) $(wildcard apis/*.cpp) $(wildcard components/*.cpp) $(wildcard framing/*.cpp)
objs = $(files:%.cpp=bin/%$(bin).o)
deps = $(objs:%.o=%.d)

ocvm$(bin): $(objs)
	g++ $(flags) $(objs) $(libs) -o ocvm$(bin)

$(objs): | bin bin/components bin/apis bin/framing

bin:
bin/components:
bin/apis:
bin/framing:
	mkdir -p bin/apis
	mkdir -p bin/components
	mkdir -p bin/framing

-include $(deps)
bin/%$(bin).o : %.cpp
	g++ $(flags) $(includes) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -fr bin ocvm
