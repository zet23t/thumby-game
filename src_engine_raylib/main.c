#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <rlgl.h>

#include <gen_assets.c>
#include <engine_main.h>

void *getUpdateFunction(void *lib);
void *getInitFunction(void *lib);
void *loadLibrary(const char *libName);
void unloadLibrary(void *lib);

void *coreLib;
typedef void (*UpdateFunc)(void *);
typedef void (*InitFunc)(void);
UpdateFunc update;

void buildCoreDLL()
{
    if (coreLib)
        unloadLibrary(coreLib);
    // Build core DLL
    const char *coreDLL = "core.dll";
    const char *coreSrc = "src_engine/engine_main.c";
    const char *coreOutput = "core.dll";
    char coreCommand[256];
    generateAssets();
    sprintf(coreCommand, "gcc -shared -g -o %s %s -Isrc_engine -I_src_gen", coreDLL, coreSrc);
    printf("Building core DLL: %s\n", coreCommand);
    system(coreCommand);

    // Load the DLL and get the update function
    coreLib = loadLibrary("core.dll");
    update = getUpdateFunction(coreLib);
    InitFunc init = getInitFunction(coreLib);
    if (init != NULL)
        init();
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

    buildCoreDLL();

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
            buildCoreDLL();
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
    }

    // De-Initialization
    CloseWindow(); // Close window and OpenGL context
}