flags=-g --std=c++11 -Wl,--no-as-needed -Wall
includes=-I. -I/usr/include/lua5.2/
libs=-llua5.2-c++ -lstdc++ -lncurses -lstdc++fs
files = $(wildcard *.cpp) $(wildcard apis/*.cpp) $(wildcard components/*.cpp) $(wildcard framing/*.cpp)
objs = $(files:%.cpp=bin/%.o)
deps = $(objs:%.o=%.d)

ocvm: $(objs)
	g++ $(flags) $(objs) $(libs) -o ocvm

$(objs): | bin bin/components bin/apis bin/framing

bin:
bin/components:
bin/apis:
bin/framing:
	mkdir -p bin/apis
	mkdir -p bin/components
	mkdir -p bin/framing

-include $(deps)
bin/%.o : %.cpp
	g++ $(flags) $(includes) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -fr bin ocvm

