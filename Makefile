BUILD_DIR=build
include $(N64_INST)/include/n64.mk

N64_CXXFLAGS += -std=gnu++11

src = example.cpp gdb.cpp
obj = $(src:%.cpp=$(BUILD_DIR)/%.o)

$(info src: $(src))
$(info object files: $(obj))

all: example.z64

$(BUILD_DIR)/example.elf: $(obj)

example.z64: N64_ROM_TITLE="GDB File Watcher"
example.z64: $(BUILD_DIR)/example.elf Makefile

clean:
	rm -rf $(BUILD_DIR) *.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
