CXX      := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -I.. -O2
LDFLAGS  := 
TARGET   := main

SRC_DIR   := ../src
OBJ_DIR   := obj

SRCS := $(shell find $(SRC_DIR) -name "*.cc")

OBJS := $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean rebuild

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

rebuild: clean all