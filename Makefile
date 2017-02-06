flags=-g --std=c++11 -Wl,--no-as-needed
includes=-I. -I/usr/include/lua5.2/
libs=-llua5.2-c++ -lstdc++ -lncurses
files = $(wildcard *.cpp) $(wildcard apis/*.cpp) $(wildcard components/*.cpp)
objs = $(files:%.cpp=bin/%.o)
deps = $(objs:%.o=%.d)

ocpc: $(objs)
	g++ $(flags) $(libs) $(objs) -o ocpc

$(objs): | bin bin/components bin/apis

bin:
bin/components:
bin/apis:
	mkdir -p bin/apis
	mkdir -p bin/components

-include $(deps)
bin/%.o : %.cpp
	g++ $(flags) $(includes) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -fr bin ocpc

