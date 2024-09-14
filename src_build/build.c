#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

#include "gen_assets.c"

void buildRaylibBackend()
{
    system("gcc -DPLATFORM_DESKTOP -g -o raylib_backend src_engine_raylib/main.c src_engine_raylib/loader.c -Isrc_build -Isrc_engine -I_src_gen -Iraylib/src -Lraylib/src -lraylib -lopengl32 -lgdi32 -lwinmm");
}

int main(int argc, char const *argv[])
{
    generateAssets();
    if (argc > 1)
    {
        if (strcmp(argv[1], "raylib") == 0)
        {
            buildRaylibBackend();
        }
    }
    return 0;
}
