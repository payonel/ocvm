lua ?= lua

TARGET_EXEC ?= ocvm
OPENCOMPUTERS=https://github.com/MightyPirates/OpenComputers/

LUA_CFLAGS  := $(shell pkg-config --cflags $(lua) --silence-errors)
LUA_LDFLAGS := $(shell pkg-config --libs $(lua) --silence-errors)

ifeq ($(strip $(LUA_CFLAGS) $(LUA_LDFLAGS)),)
    $(error "ERROR: pkg-config failed to find lua package: '$(lua)'")
endif

HOST=$(shell uname -s)

ldflags.Haiku = -lnetwork
dirs.Haiku    = haiku

LDFLAGS+=-lstdc++
ifeq ($(shell uname -s 2>/dev/null),Haiku)
	LDFLAGS+=-lnetwork
else
	LDFLAGS+=-lstdc++fs -pthread -ldl
endif

SRC_DIRS = . apis color components drivers io model util $(dirs.$(HOST))
SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))

CXXFLAGS += $(LUA_CFLAGS)
CXXFLAGS += -MMD -MP -Wall -g --std=c++17 -O0 -Wl,--no-as-needed
CXXFLAGS += $(addprefix -I,$(SRC_DIRS))
LDFLAGS  += $(LUA_LDFLAGS) $(ldflags.$(HOST))

SYSTEM_DIR := system
SYSTEM_FILES := $(SYSTEM_DIR)/

BUILD_DIR ?= ./bin
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

-include $(DEPS)
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

deps:
	@if ! command -v git >/dev/null; then \
		echo deps cancelled: 'make deps' requires 'git' command. Aborting \
		false; \
	fi
	@if [ -d $(SYSTEM_DIR) ]; then \
		echo deps skipped: ./$(SYSTEM_DIR) already exists. To redownload system files, remove ./$(SYSTEM_DIR) and run 'make deps' again; \
	else \
		set -e; \
		mkdir -p $(SYSTEM_DIR); \
		cd $(SYSTEM_DIR); \
		git clone -n --depth=1 --filter=tree:0 $(OPENCOMPUTERS); \
		cd OpenComputers; \
		git sparse-checkout set --no-cone /src/main/resources/assets/opencomputers/ && git checkout; \
		set -x; \
		mv src/main/resources/assets/opencomputers/loot ../; \
		mv src/main/resources/assets/opencomputers/lua/machine.lua ../; \
		mv src/main/resources/assets/opencomputers/lua/bios.lua ../; \
		mv src/main/resources/assets/opencomputers/font.hex ../; \
		cd ../..; \
		rm -rf $(SYSTEM_DIR)/OpenComputers; \
	fi

help:
	@echo See README.md for build instructions
	@echo Likely all you need is: make deps \$$\$$ make

clean:
	$(RM) -r $(BUILD_DIR) $(TARGET_EXEC)
