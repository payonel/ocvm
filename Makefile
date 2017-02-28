MAKEFLAGS+="-j 4"
flags=-g --std=c++14 -Wl,--no-as-needed -Wall

ifeq ($(prof),)
libs=-llua5.2-c++
includes=-I/usr/include/lua5.2/
else
$(info profile build)
libs=-L../gperftools-2.5/.libs/ -ldl -lprofiler ../lua-5.3.4/src/liblua.a
includes=-I../lua-5.3.4/src
bin=-profiled
endif

includes+=-I.
libs+=-lstdc++ -lncurses -lstdc++fs -lX11 -pthread

files = $(wildcard *.cpp) $(wildcard apis/*.cpp) $(wildcard components/*.cpp) $(wildcard framing/*.cpp)
objs = $(files:%.cpp=bin/%$(bin).o)
deps = $(objs:%.o=%.d)

ocvm$(bin): $(objs)
	g++ $(flags) $(objs) $(libs) -o ocvm$(bin)

-include $(deps)
bin/%$(bin).o : %.cpp
	@mkdir -p $@D
	g++ $(flags) $(includes) -MMD -c $< -o $@

.PHONY : clean
clean:
	@echo rm -rf bin ocvm ocvm-profiled

ifneq ($(filter clean,$(MAKECMDGOALS)),)
	$(shell rm -rf bin ocvm ocvm-profiled)
endif

