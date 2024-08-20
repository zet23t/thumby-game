gcc src_build/build.c -g -Iraylib/src -Lraylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -o build.exe
REM Path: build.exe
build.exe raylib

raylib_backend.exe