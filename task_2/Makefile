CXX = g++
CPPFLAGS += -I.
CXXFLAGS += -std=c++17 -Wall -Wextra -Werror -pedantic -fno-exceptions -fno-rtti -nostdlib

CLASSES = unique_pointer_object unique_pointer_array flexible_pointer \
	shared_pointer copiable_unique_pointer elapsed_warning
OBJECTS = $(addprefix build/,$(addsuffix .o,some_object $(CLASSES)))
TESTS = $(addprefix build/test_,$(CLASSES))

.PHONY: all test zip clean

all: $(OBJECTS)

build/%.o: %.cpp %.h some_object.h Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/test_%: test_%.cpp build/%.o build/some_object.o Makefile | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o build/test_$*.o
	$(CXX) $(LDFLAGS) build/$*.o build/some_object.o build/test_$*.o $(LDLIBS) -o $@

build:
	mkdir -p build

test: all $(TESTS)
	./build/test_unique_pointer_object
	./build/test_unique_pointer_array
	./build/test_flexible_pointer
	./build/test_shared_pointer
	./build/test_copiable_unique_pointer
	./build/test_elapsed_warning

zip: $(addsuffix .h,$(CLASSES)) $(addsuffix .cpp,$(CLASSES))
	zip Korenev_ID.zip $^

clean:
	rm -rf build
