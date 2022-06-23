CC = g++
cflags = -std=c++17 -O2 -g
cflags += -Ivendor/raylib/src -Iinclude -Ivendor/zip/src
ldflags += -Lvendor/raylib/src -lraylib -Lvendor/zip/build -lzip
name = osus

sources = $(wildcard src/*.cpp)
objects = $(patsubst src/%, object/%,$(sources:.cpp=.o))
deps = $(objects:.o=.d)

-include $(deps)
.PHONY: all clean

all: files deps $(name)

files:
	mkdir -p bin object

deps:
	cd vendor/raylib/src/ && $(MAKE)
	cd vendor/zip && mkdir -p build && cd build && cmake .. && make

run: $(name)
	mkdir -p beatmaps
	bin/$(name)

$(name): $(objects)
	$(CC) -o bin/$(name) $^ $(ldflags)

object/%.o: src/%.cpp
	$(CC) -MMD -o $@ -c $< $(cflags)

clean:
	rm -rf vendor/raylib/src/*.o vendor/raylib/src/libraylib.a bin/$(name) $(objects) object/*.d
