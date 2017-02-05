flags=-g --std=c++11
includes=-I. -I/usr/include/lua5.2/
deps=$(wildcard *.h)
deps+=$(wildcard apis/*.h)
deps+=$(wildcard components/*.h)
files_0=$(wildcard *.cpp)
files_1=$(wildcard apis/*.cpp)
files_2=$(wildcard components/*.cpp)
objs_0=$(addprefix bin/,$(notdir $(files_0:.cpp=.o)))
objs_1=$(addprefix bin/apis/,$(notdir $(files_1:.cpp=.o)))
objs_2=$(addprefix bin/components/,$(notdir $(files_2:.cpp=.o)))
libs=-llua5.2-c++ -lstdc++ -Wl,--no-as-needed

ocpc: $(objs_0) $(objs_1) $(objs_2)
	g++ $(flags) $(libs) $(objs_0) $(objs_1) $(objs_2) $(libs) -o ocpc

$(objs_0): | bin
bin:
	mkdir -p bin

$(objs_1): | bin/apis
bin/apis:
	mkdir -p bin/apis

$(objs_2): | bin/components
bin/components:
	mkdir -p bin/components

bin/%.o: %.cpp $(deps)
	g++ -o $@ -c $< $(flags) $(includes) $(libs)

bin/apis/%.o: apis/%.cpp $(deps)
	g++ -o $@ -c $< $(flags) $(includes) $(libs)

bin/components/%.o: components/%.cpp $(deps)
	g++ -o $@ -c $< $(flags) $(includes) $(libs)

.PHONY: clean

clean:
	rm -fr bin ocpc


flags=-Wfatal-errors -Wall -Wextra -Wpedantic -Wconversion -Wshadow -g --std=c++11
files = $(wildcard *.cpp) $(wildcard apis/*.cpp) $(wildcard components/*.cpp)
objs = $(files:%.cpp=$(bin)/%.o)
deps = $(objs:%.o=%.d)

ocpc: $(objs)
    mkdir -p $(@D)
    g++ -c $< $(flags) $(includes) $(libs) -o $@

-include $(deps)

bin/%.o : %.cpp
    mkdir -p $(@D)
    g++ $(flags) $(includes) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -fr bin ocpc

