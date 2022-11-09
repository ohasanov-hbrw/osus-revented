CC = g++ -DGRAPHICS_API_OPENGL_11 -DOPENGL_SPOOF
cflags = -std=c++17 -g -O2
cflags += -Ivendor/raylib/src -Iinclude -Ivendor/zip/src -Iinclude/sound 
ldflags += -lstdc++ -Lvendor/raylib/src -lraylib -Lvendor/zip/build -lzip -lpthread -ldl
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
	cd vendor/zip && mkdir -p build && cd build && cmake .. -DCMAKE_DISABLE_TESTING=1 && make -j12
	cd vendor/zip && mkdir -p build && cd build && cmake .. -DCMAKE_DISABLE_TESTING=1 && make -j12
run: $(name)
	mkdir -p beatmaps
	bin/$(name)

$(name): $(objects)
	$(CC) -o bin/$(name) $^ $(ldflags)

object/%.o: src/%.cpp
	$(CC) -MMD -o $@ -c $< $(cflags)

clean:
	rm -rf vendor/raylib/src/*.o vendor/raylib/src/libraylib.a bin/$(name) $(objects) object/*.d vendor/zip/build SDL_mixer/build vendor/raylib/src/external/glfw/build
