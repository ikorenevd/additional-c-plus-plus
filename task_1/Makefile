SYSTEM := $(shell uname -s)

TARGET := build/task_1
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,build/obj/%.o,$(SOURCES))
DEPFILES := $(OBJECTS:.o=.d)

CPPFLAGS += -I.
LINUX_CXXFLAGS := \
	-O3 -mfpmath=sse -fstack-protector-all -g -W -Wall -Wextra -Wunused -Wcast-align -Werror -pedantic -pedantic-errors -Wfloat-equal -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Wformat=1 -Wwrite-strings -Wcast-align -Wno-long-long -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-qual -Wno-suggest-attribute=format

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

# -nostdlib disables implicit libraries and startup files. The classes and
# process-isolated tests still use C/POSIX; link those dependencies explicitly.
# Header-only C++ utilities are allowed, but no C++ runtime is linked.
CXXFLAGS += -fno-exceptions -fno-rtti
ifeq ($(SYSTEM),Darwin)
TEST_LIBS := -lSystem
else
TEST_START := $(shell $(CXX) -print-file-name=crt1.o) $(shell $(CXX) -print-file-name=crti.o) $(shell $(CXX) -print-file-name=crtbegin.o)
TEST_END := $(shell $(CXX) -print-file-name=crtend.o) $(shell $(CXX) -print-file-name=crtn.o)
TEST_LIBS := -lm -lc -lgcc
# Use the non-PIE startup objects consistently, including on PIE-default GCC.
CXXFLAGS += -fno-pie
LDFLAGS += -no-pie
endif

.PHONY: all test test-sanitize clean

all: $(TARGET)

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(TEST_START) $(OBJECTS) $(LDLIBS) $(TEST_LIBS) $(TEST_END) -o $@

build/obj/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build

-include $(DEPFILES)

# Separate binary: sanitizer flags must not reuse normal object files.
# Sanitizers require their hosted runtime and the standard allocation operators.
SANITIZER_TARGET := build/task_1_sanitized
SANITIZER_SOURCES := $(filter-out test_runtime.cpp,$(SOURCES))

$(SANITIZER_TARGET): $(SOURCES) $(wildcard *.h) Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(filter-out -nostdlib,$(CXXFLAGS)) -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all $(LDFLAGS) $(SANITIZER_SOURCES) $(LDLIBS) -o $@

test-sanitize: $(SANITIZER_TARGET)
	./$(SANITIZER_TARGET)
