gcc src_build/build.c -g -Iraylib/src -Lraylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -o build.exe
REM Path: build.exe
set arg=%1
if "%arg%"=="" set arg=raylib
build.exe %arg% %2 %3 %4 %5 %6 %7 %8 %9

if "%arg%"=="raylib" raylib_backend.exe
if "%arg%"=="web" emrun --no_browser --port 8080 _web_build