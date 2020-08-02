MK_DIR:=$(shell pwd)
BUILD_DIR := $(MK_DIR)/build
default_target: all

.PHONY : default_target

make_build:
	@mkdir -p $(BUILD_DIR)

# The main all target
all: make_build
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release $(MK_DIR) && make

.PHONY : all

# The main clean target
clean:
	@rm -rf $(BUILD_DIR)
