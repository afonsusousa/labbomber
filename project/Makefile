CC = cc
PROG = proj
CFLAGS = -Wall -Wextra -pedantic -D__LCOM_OPTIMIZED__ -I./lib

BUILD_DIR = build
SRC_DIR = src

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/game.c
OBJS = $(BUILD_DIR)/src/main.o $(BUILD_DIR)/src/game.o

LIB_DIR = $(BUILD_DIR)/lib
LIBS = -L$(LIB_DIR) -lutils -ltimer -lkeyboard -lmouse -lvbe -lrtc
LCOM_LIBS = -llm -llcf

all: libs $(BUILD_DIR)/$(PROG)

libs:
	$(MAKE) -C $(PROJECT_DIR)lib

$(BUILD_DIR)/$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LCOM_LIBS)

$(BUILD_DIR)/src/main.o: src/main.c
	@mkdir -p $(BUILD_DIR)/src
	$(CC) $(CFLAGS) -c src/main.c -o $(BUILD_DIR)/src/main.o

$(BUILD_DIR)/src/game.o: src/game.c
	@mkdir -p $(BUILD_DIR)/src
	$(CC) $(CFLAGS) -c src/game.c -o $(BUILD_DIR)/src/game.o

clean:
	$(MAKE) -C $(PROJECT_DIR)lib clean
	rm -rf $(BUILD_DIR)/src/* $(BUILD_DIR)/$(PROG)

.PHONY: all libs clean
