SYSTEM := $(shell uname -s)

TARGET := build/task_1
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,build/obj/%.o,$(SOURCES))
DEPFILES := $(OBJECTS:.o=.d)

CPPFLAGS += -I.
LINUX_CXXFLAGS := \
	-mfpmath=sse -fstack-protector-all \
	-W -Wall -Wextra -Wunused -Wcast-align -Werror \
	-pedantic -pedantic-errors -Wfloat-equal -Wpointer-arith \
	-Wformat-security -Wmissing-format-attribute -Wformat=1 \
	-Wwrite-strings -Wcast-align -Wno-long-long -Wcast-qual \
	-Wno-suggest-attribute=format

# Apple Clang uses -Wformat instead of -Wformat=1 and does not support
# -Wno-suggest-attribute=format. SSE floating-point math is unavailable on ARM.
MACOS_CXXFLAGS := $(filter-out -mfpmath=sse -Wformat=1 -Wno-suggest-attribute=format,$(LINUX_CXXFLAGS)) -Wformat

ifeq ($(SYSTEM),Darwin)
CXX = clang++
CXXFLAGS += -std=c++17 $(MACOS_CXXFLAGS)
else
CXX = g++
CXXFLAGS += -std=c++17 $(LINUX_CXXFLAGS)
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

build/obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build

-include $(DEPFILES)
