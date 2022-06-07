CC = g++
cflags = -std=c++17 -O2 -g -Wall -Wextra -Werror -Wstrict-aliasing
cflags += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
cflags += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
cflags += -Ivendor/raylib/src -Iinclude
ldflags += -lraylib
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

run: $(name)
	bin/$(name)

$(name): $(objects)
	$(CC) -o bin/$(name) $^ $(ldflags)

object/%.o: src/%.cpp
	$(CC) -MMD -o $@ -c $< $(cflags)

clean:
	rm -rf vendor/raylib/src/*.o vendor/raylib/src/libraylib.a bin/$(name) $(objects) object/*.d
