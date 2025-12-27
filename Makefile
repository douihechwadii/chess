# Compiler
CC = gcc

# Project name
TARGET = chess

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = builds

# Source files (all .c inside src directory)
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files (convert src/*.c → builds/*.o)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Compiler flags (common for both platforms)
CFLAGS = -O1 -Wall -std=c99 -Wno-missing-braces -I $(INC_DIR)

# Detect operating system
ifeq ($(OS),Windows_NT)
    # Windows-specific settings
    TARGET := $(TARGET).exe
    LDFLAGS = -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
    MKDIR = mkdir
    RM = rmdir /S /Q
    OUT = $(BUILD_DIR)/$(TARGET)
    CLEAN_CMD = @if exist $(BUILD_DIR) ( $(RM) $(BUILD_DIR) )
else
    # Linux-specific settings
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
        MKDIR = mkdir -p
        RM = rm -rf
        OUT = $(BUILD_DIR)/$(TARGET)
        CLEAN_CMD = $(RM) $(BUILD_DIR)
    endif
endif

# Default rule
all: $(BUILD_DIR) $(OUT)

# Create build directory
$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

# Link all object files into final executable
$(OUT): $(OBJS)
	$(CC) $(OBJS) -o $(OUT) $(LDFLAGS)

# Compile each .c file into .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Run the program
run: all
	./$(OUT)

# Clean build files
clean:
	$(CLEAN_CMD)

.PHONY: all run clean