CXX      := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Isrc/include -O2
LDFLAGS  := 
TARGET   := main

SRC_DIR    := src
BUILD_DIR  := build
TARGET_DIR := target

SRCS := $(shell find $(SRC_DIR) -name "*.cc")
OBJS := $(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean rebuild

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(TARGET_DIR)
	$(CXX) $(OBJS) -o $(TARGET_DIR)/$@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET_DIR)

rebuild: clean all