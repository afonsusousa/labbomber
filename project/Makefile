PROG = proj
CC = cc
PROJECT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CFLAGS = -Wall -Wextra -pedantic -D__LCOM_OPTIMIZED__ -I$(PROJECT_DIR)lib

BUILD_DIR = $(PROJECT_DIR)build
SRC_DIR = $(PROJECT_DIR)src

# Game source files
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/game.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/src/%.o, $(SRCS))

# Sub-library configurations
LIB_DIR = $(BUILD_DIR)/lib
# Add libraries that exist in the lib folder
LIBS = -L$(LIB_DIR) -lutils -ltimer -lkeyboard -lmouse -lvbe -lrtc
LCOM_LIBS = -llm

all: libs $(BUILD_DIR)/$(PROG)

# Compile all modules inside lib/ to .a files
libs:
	$(MAKE) -C $(PROJECT_DIR)lib

# Link the final executable using the static .a modules
$(BUILD_DIR)/$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LCOM_LIBS)

# Compile .c files into the build directory
$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)/src
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(MAKE) -C $(PROJECT_DIR)lib clean
	rm -rf $(BUILD_DIR)/src/* $(BUILD_DIR)/$(PROG)

.PHONY: all libs clean
