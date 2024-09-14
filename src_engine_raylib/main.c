#ifndef PLATFORM_DESKTOP
#define PLATFORM_DESKTOP
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>
#include <pthread.h>

#include <gen_assets.c>
#include <engine_main.h>
#include <raymath.h>
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
            sprintf(args->command, "gcc -DPLATFORM_DESKTOP -c -g -o %s %s -Isrc_engine -I_src_gen", coreObjFiles[i], coreSrcFiles[i]);
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

    sprintf(coreCommand, "gcc -DPLATFORM_DESKTOP -shared -g -o %s %s", coreDLL, objfiles);
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

uint32_t getUTime(void)
{
    return (uint32_t) (GetTime() * 1000000.0f);
}


#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
int main(void)
{
    // Initialization
    int screenWidth = 128 * 2;
    int screenHeight = 128 * 2;
    int isExtended = 0;
    
    InitWindow(screenWidth, screenHeight, "Thumby color engine simulator");
    printf("sizeof(RuntimeContext) = %d\n", sizeof(RuntimeContext));

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    buildCoreDLL(1);

    RuntimeContext ctx = {0};
    ctx.getUTime = getUTime;
    int isPaused = 0;
    int step = 0;

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
    Image imgOverdraw = {
        .width = 128,
        .height = 128,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .data = ctx.frameStats.overdrawCount,
        .mipmaps = 1,
    };
    Texture2D overdrawTexture = LoadTextureFromImage(imgOverdraw);

    int loadSkip = 0;
    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        if (IsKeyPressed(KEY_P)) {
            isPaused = !isPaused;
        }
        if (IsKeyPressed(KEY_LEFT_SHIFT) && isPaused)
        {
            step = 1;
        }

        if (IsKeyPressed(KEY_F))
        {
            // toggle window always on top
            static int topmost = 0;
            if (topmost)
                topmost = 0, ClearWindowState(FLAG_WINDOW_TOPMOST);
            else
                topmost = 1, SetWindowState(FLAG_WINDOW_TOPMOST);
        }
            

        if (IsKeyPressed(KEY_R))
        {
            float t = GetTime();
            buildCoreDLL(IsKeyDown(KEY_LEFT_CONTROL));
            printf("Rebuilt core DLL in %f seconds\n", GetTime() - t);
            loadSkip = 2;
            ctx.time = 0.0f;
            ctx.frameCount = 0;
        }
        int winW = 0, winH = 0;
        if (IsKeyPressed(KEY_ONE)) winW = 128, winH = 128;
        if (IsKeyPressed(KEY_TWO)) winW = 128 * 2, winH = 128 * 2;
        if (IsKeyPressed(KEY_THREE)) winW = 128 * 3, winH = 128 * 3;
        if (IsKeyPressed(KEY_FOUR)) winW = 128 * 4, winH = 128 * 4;
        if (IsKeyPressed(KEY_FIVE)) winW = 128 * 5, winH = 128 * 5;
        if (IsKeyPressed(KEY_SIX)) winW = 128 * 6, winH = 128 * 6;
        
        if (IsKeyPressed(KEY_X)) 
        {
            isExtended = !isExtended;
            winW = isExtended ? screenWidth * 2 : screenWidth / 2;
        }

        if (winW > 0)
        {
            Vector2 winPos = GetWindowPosition();
            int x = (int) winPos.x;
            int y = (int) winPos.y;
            winH = winW;
            SetWindowPosition(x + (screenWidth - winW) / 2, y + (screenHeight - winH) / 2);
            screenWidth = winW, screenHeight = winH;
            SetWindowSize(winW, winH);
        }
        // Update
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(GRAY);
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        float offsetWidth = screenWidth;
        float offsetHeight = screenHeight;
        if (isExtended)
        {
            offsetWidth = screenWidth / 2;
            offsetHeight = screenHeight / 2;
        }
        float scale = min(offsetWidth / 128.0f, offsetHeight / 128.0f);
        Vector2 offset = (Vector2){(offsetWidth - 128 * scale) / 2, (offsetHeight - 128 * scale) / 2};

        if ((!isPaused || step) && loadSkip-- <= 0)
        {
            step = 0;
            ctx.frameCount++;
            ctx.previousInputState = ctx.inputState;

            ctx.inputA = IsKeyDown(KEY_I);
            ctx.inputB = IsKeyDown(KEY_J);
            ctx.inputUp = IsKeyDown(KEY_W);
            ctx.inputDown = IsKeyDown(KEY_S);
            ctx.inputLeft = IsKeyDown(KEY_A);
            ctx.inputRight = IsKeyDown(KEY_D);
            ctx.inputShoulderLeft = IsKeyDown(KEY_Q);
            ctx.inputShoulderRight = IsKeyDown(KEY_E);
            ctx.inputMenu = IsKeyDown(KEY_M);
            ctx.time += GetFrameTime();
            ctx.deltaTime = GetFrameTime();

            memset(ctx.frameStats.overdrawCount, 0, sizeof(ctx.frameStats.overdrawCount));
            update(&ctx);
            uint32_t maxOverdraw = 0;
            for (int i = 0; i < 128 * 128; i++)
            {
                maxOverdraw = max(maxOverdraw, ctx.frameStats.overdrawCount[i]);
            }

            for (int i = 0; i < 128 * 128; i++)
            {
                float overdraw = (float)ctx.frameStats.overdrawCount[i] / maxOverdraw * 2.0f;
                Color c = overdraw <= 1.0f ?
                    (Color){(uint8_t)((1.0f - overdraw) * 255), (uint8_t)((overdraw) * 255), 0, 255} :
                    (Color){0, (uint8_t)(1.0f-(overdraw - 1.0f) * 255), (uint8_t)(((overdraw - 1.0f)) * 255), 255};
                ctx.frameStats.overdrawCount[i] = c.a << 24 | c.r << 16 | c.g << 8 | c.b;
            }
            uint32_t screenData[128 * 128];
            for (int i = 0; i < 128 * 128; i++)
            {
                uint32_t c = ctx.screenData[i];
                screenData[i] = c | 0xFF000000;
            }
            UpdateTexture(texture, screenData);
            UpdateTexture(overdrawTexture, ctx.frameStats.overdrawCount);
            if (isExtended)
                DrawText(TextFormat("FPS: %d, maxOverdraw=%d", GetFPS(), maxOverdraw), 10, 10 + screenHeight / 2, 20, BLACK);
        }
        
        rlDisableColorBlend();
        DrawTextureEx(texture, offset, 0.0f, scale, WHITE);
        DrawTextureEx(overdrawTexture, (Vector2){offset.x + 128 * scale, offset.y}, 0.0f, scale, WHITE);
        rlEnableColorBlend();

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