make clean
make deps CC="x86_64-w64-mingw32-gcc --static -DGRAPHICS_API_OPENGL_33" -j12
make run CC="x86_64-w64-mingw32-g++ --static -DGRAPHICS_API_OPENGL_33" name="osus.exe" ldflags="-Lvendor/raylib/src -lraylib -Lvendor/zip/build -lzip -lpthread -lwinmm -lgdi32 -lopengl32" -j12