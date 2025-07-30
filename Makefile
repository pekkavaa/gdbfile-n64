BUILD_DIR=build
include $(N64_INST)/include/n64.mk

src = example.c gdbfile.c
obj = $(src:%.c=$(BUILD_DIR)/%.o)

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
