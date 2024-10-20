#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

#include "gen_assets.c"

static void collectCFiles(char *buffer, const char *dir)
{
    FilePathList files = LoadDirectoryFiles(dir);
    for (int i = 0; i < files.count; i++)
    {
        if (IsFileExtension(files.paths[i], ".c"))
        {
            strcat(buffer, files.paths[i]);
            strcat(buffer, " ");
        }
    }
    UnloadDirectoryFiles(files);
}

void buildWebBackend(int optimize)
{
    system("mkdir _web_build");
    char inputFiles[2048] = {0};
    collectCFiles(inputFiles, "src_engine");
    collectCFiles(inputFiles, "_src_gen");
    char command[2048] = {0};
    const char* exportedFunctions = 
        "_malloc,_free,_init,_update,_RuntimeContext_create,"
        "_RuntimeContext_setUTimeCallback,_RuntimeContext_getScreen,"
        "_RuntimeContext_getRGBLed,_RuntimeContext_getRumble,"
        "_RuntimeContext_updateInputs,"
        "_AudioContext_create,_AudioContext_beforeRuntimeUpdate,"
        "_AudioContext_afterRuntimeUpdate,_AudioContext_audioUpdate"
        ",_AudioContext_getChannelStatus"
        ",_AudioContext_setSfxInstructions"
        ",_RuntimeContext_getSfxInstructions"
        ",_RuntimeContext_setSfxChannelStatus"
        ",_RuntimeContext_clearSfxInstructions"
        ;
    sprintf(command, "emcc %s -o _web_build/game.js %s -s EXPORTED_FUNCTIONS=%s "
        "-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,addFunction,getValue -s WASM=1 -s MODULARIZE=1 "
        "-s EXPORT_NAME=createModule -s ALLOW_TABLE_GROWTH=1 -I _src_gen", 
        inputFiles, optimize ? "-O3" : "", exportedFunctions);
    printf("Building web backend, command: \n%s\n", command);
    system(command);
    const char *runtimeJSLib = "src_runtime_web/*.js";
    const char *runtimeJSOut = "_web_build";
    char copyCommand[512];
    sprintf(copyCommand, "cp %s %s", runtimeJSLib, runtimeJSOut);
    printf("Copying runtime JS, command: \n%s\n", copyCommand);
    system(copyCommand);
}

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
            return 0;
        }
        if (strcmp(argv[1], "web") == 0)
        {
            int optimize = argc > 2 && strcmp(argv[2], "release") == 0;
            buildWebBackend(optimize);
            return 0;
        }

        printf("Unknown target: %s\n", argv[1]);
    }
    return 0;
}
