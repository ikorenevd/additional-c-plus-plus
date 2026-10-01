CXX = g++
CPPFLAGS += -I.
CXXFLAGS += -std=c++17 -O3 -mfpmath=sse -fstack-protector-all -g \
	-W -Wall -Wextra -Wunused -Wcast-align -Werror -pedantic -pedantic-errors \
	-Wfloat-equal -Wpointer-arith -Wformat-security -Wmissing-format-attribute \
	-Wformat=1 -Wwrite-strings -Wno-long-long -Woverloaded-virtual \
	-Wnon-virtual-dtor -Wcast-qual -Wno-suggest-attribute=format \
	-nostdlib -fno-exceptions -fno-rtti -fno-pie
LDFLAGS += -no-pie
LDLIBS += -lm -lc -lgcc

# -nostdlib requires explicit startup objects and libraries (Linux/GCC).
STARTUP := $(shell $(CXX) -print-file-name=crt1.o) \
	$(shell $(CXX) -print-file-name=crti.o) \
	$(shell $(CXX) -print-file-name=crtbegin.o)
FINISH := $(shell $(CXX) -print-file-name=crtend.o) \
	$(shell $(CXX) -print-file-name=crtn.o)
OBJECTS := build/main.o build/block.o build/date_type.o build/rational.o build/test_runtime.o

.PHONY: all test clean

all: build/task_1

build/task_1: $(OBJECTS) $(STARTUP) $(FINISH) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(STARTUP) $(OBJECTS) $(LDLIBS) $(FINISH) -o $@

build/main.o: main.cpp block.h date_type.h rational.h Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c main.cpp -o $@

build/block.o: block.cpp block.h Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c block.cpp -o $@

build/date_type.o: date_type.cpp date_type.h Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c date_type.cpp -o $@

build/rational.o: rational.cpp rational.h Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c rational.cpp -o $@

build/test_runtime.o: test_runtime.cpp Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c test_runtime.cpp -o $@

build:
	mkdir -p build

test: build/task_1
	./build/task_1

clean:
	rm -rf build
