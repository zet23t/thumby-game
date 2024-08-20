#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>
#include <pthread.h>

#include <gen_assets.c>
#include <engine_main.h>

void *getUpdateFunction(void *lib);
void *getInitFunction(void *lib);
void *loadLibrary(const char *libName);
void unloadLibrary(void *lib);
int copyBitmapIntoClipboard(void* hwnd, const uint8_t *bitmapData, size_t sizeT);

void *coreLib;
typedef void (*UpdateFunc)(void *);
typedef void (*InitFunc)(void);
UpdateFunc update;

char* strReplaceDirSeps(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\\' || str[i] == '/')
        {
            str[i] = '_';
        }
    }
    return str;
}


typedef struct {
    char command[256];
} ThreadArgs;

void *compileFile(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    system(threadArgs->command);
    free(threadArgs);
    return NULL;
}

void buildCoreDLL(int forceRebuild)
{
    if (coreLib)
        unloadLibrary(coreLib);
    
    // Build core DLL
    createDirectory("_obj");
    const char *coreDLL = "core.dll";
    char *coreSrcFiles[128] = {
        0
    };
    char *coreObjFiles[128] = {
        0
    };

    FilePathList list = LoadDirectoryFiles("src_engine");
    int cFileCount = 0;
    int buildAll = forceRebuild;
    for (int i = 0; i < list.count; i++)
    {
        if (strcmp(GetFileExtension(list.paths[i]), ".h") == 0)
        {
            struct stat srcStat, objStat;
            int srcExists = stat(list.paths[i], &srcStat);
            char objFile[256];
            sprintf(objFile, "_obj/%s.o", GetFileNameWithoutExt(list.paths[i]));
            strReplaceDirSeps(objFile + 5);
            int objExists = stat(objFile, &objStat);

            // Compile if the object file doesn't exist or the source file is newer
            if (objExists != 0 || srcStat.st_mtime > objStat.st_mtime)
            {
                printf("Header change detected, rebuilding all: %s\n", list.paths[i]);
                buildAll = 1;
                break;
            }
        }
    }
    for (int i = 0; i < list.count; i++)
    {
        if (strcmp(GetFileExtension(list.paths[i]), ".c") == 0)
        {
            printf("Found core source file: %s\n", list.paths[i]);
            coreSrcFiles[cFileCount] = strdup(list.paths[i]);
            char objFile[256];
            sprintf(objFile, "_obj/%s.o", GetFileNameWithoutExt(list.paths[i]));
            coreObjFiles[cFileCount] = strdup(objFile);
            strReplaceDirSeps(coreObjFiles[cFileCount] + 5);
            cFileCount++;
        }
    }
    UnloadDirectoryFiles(list);

    list = LoadDirectoryFiles("_src_gen");
    for (int i = 0; i < list.count; i++)
    {
        if (strcmp(GetFileExtension(list.paths[i]), ".c") == 0)
        {
            printf("Found core source file: %s\n", list.paths[i]);
            coreSrcFiles[cFileCount] = strdup(list.paths[i]);
            char objFile[256];
            sprintf(objFile, "_obj/%s.o", list.paths[i]);
            coreObjFiles[cFileCount] = strdup(objFile);
            strReplaceDirSeps(coreObjFiles[cFileCount] + 5);
            cFileCount++;
        }
    }
    UnloadDirectoryFiles(list);

    printf("Found %d core source files\n", cFileCount);

    const char *coreOutput = "core.dll";
    char coreCommand[256];
    char objfiles[1024] = {0};
    generateAssets();

    
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * cFileCount);
    int usedThreads = 0;
    for (int i = 0; i < cFileCount; i++)
    {
        strcat(objfiles, coreObjFiles[i]);
        strcat(objfiles, " ");
        struct stat srcStat, objStat;
        int srcExists = stat(coreSrcFiles[i], &srcStat);
        int objExists = stat(coreObjFiles[i], &objStat);

        // Compile if the object file doesn't exist or the source file is newer
        if (buildAll || objExists != 0 || srcStat.st_mtime > objStat.st_mtime) {
            ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
            sprintf(args->command, "gcc -c -g -o %s %s -Isrc_engine -I_src_gen", coreObjFiles[i], coreSrcFiles[i]);
            printf("Scheduled building core object: %s\n", args->command);
            pthread_create(&threads[usedThreads++], NULL, compileFile, args);
            // printf("Building core object: %s\n", coreCommand);
            // system(coreCommand);
        }

        free((void *)coreSrcFiles[i]);
        free((void *)coreObjFiles[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < usedThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);

    sprintf(coreCommand, "gcc -shared -g -o %s %s", coreDLL, objfiles);
    printf("Building core DLL: %s\n", coreCommand);
    system(coreCommand);

    // Load the DLL and get the update function
    coreLib = loadLibrary("core.dll");
    update = getUpdateFunction(coreLib);
    InitFunc init = getInitFunction(coreLib);
    if (init != NULL)
        init();
}


void copyScreenShot()
{
    // Capture the screenshot data
    TakeScreenshot("screenshot.png");
    system("powershell.exe -windowstyle hidden -Command \"Add-Type -AssemblyName System.Windows.Forms; [Windows.Forms.Clipboard]::SetImage($([System.Drawing.Image]::Fromfile(\\\"screenshot.png\\\")))\"");
    remove("screenshot.png");
}

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
int main(void)
{
    // Initialization
    const int screenWidth = 128 * 6;
    const int screenHeight = 128 * 6;

    InitWindow(screenWidth, screenHeight, "Thumby color engine simulator");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    buildCoreDLL(1);

    RuntimeContext ctx;

    for (int i = 0; i < 128 * 128; i++)
    {
        ctx.screenData[i] = 0xFF8800FF;
    }

    Image img = {
        .width = 128,
        .height = 128,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .data = ctx.screenData,
        .mipmaps = 1,
    };

    Texture2D texture = LoadTextureFromImage(img);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        if (IsKeyPressed(KEY_R))
        {
            float t = GetTime();
            buildCoreDLL(IsKeyDown(KEY_LEFT_CONTROL));
            printf("Rebuilt core DLL in %f seconds\n", GetTime() - t);
        }
        // Update
        //----------------------------------------------------------------------------------
        BeginDrawing();
        rlDisableColorBlend();

        ClearBackground(GRAY);
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        float scale = min(screenWidth / 128.0f, screenHeight / 128.0f);
        Vector2 offset = (Vector2){(screenWidth - 128 * scale) / 2, (screenHeight - 128 * scale) / 2};

        ctx.frameCount++;
        ctx.inputA = IsKeyDown(KEY_I);
        ctx.inputB = IsKeyDown(KEY_J);
        ctx.inputUp = IsKeyDown(KEY_W);
        ctx.inputDown = IsKeyDown(KEY_S);
        ctx.inputLeft = IsKeyDown(KEY_A);
        ctx.inputRight = IsKeyDown(KEY_D);
        ctx.inputShoulderLeft = IsKeyDown(KEY_Q);
        ctx.inputShoulderRight = IsKeyDown(KEY_E);
        ctx.inputMenu = IsKeyDown(KEY_M);
        ctx.time = GetTime();
        ctx.deltaTime = GetFrameTime();

        update(&ctx);
        UpdateTexture(texture, ctx.screenData);
        DrawTextureEx(texture, offset, 0.0f, scale, WHITE);

        EndDrawing();

        // copy screen data to clipboard
        if (IsKeyPressed(KEY_C))
        {
            // compress screenshot data in memory to PNG format
            copyScreenShot();
        }
    }

    // De-Initialization
    CloseWindow(); // Close window and OpenGL context
}