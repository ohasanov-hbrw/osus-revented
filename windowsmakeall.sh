make clean
make deps CC="i686-w64-mingw32-gcc -m32 --static" -j12
make run CC="i686-w64-mingw32-g++ -m32 --static" name="osus.exe" ldflags="-Lvendor/raylib/src -lraylib -Lvendor/zip/build -lzip -lpthread -lwinmm -lgdi32 -lopengl32" -j12