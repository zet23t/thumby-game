#ifndef PLATFORM_DESKTOP
#define PLATFORM_DESKTOP
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>
#include <pthread.h>
#include <setjmp.h>

#include <gen_assets.c>
#include <engine_main.h>
#include <raymath.h>
#include <math.h>

static jmp_buf panicJmpBuf;

void *getFunction(void *lib, const char *name);
void *loadLibrary(const char *libName);
void unloadLibrary(void *lib);

void* configureCOMPort(const char *portName);
void closeCOMPort(void* hComm);
int sendData(void* hComm, const char *data);
int receiveData(void* hComm, char *buffer, int bufferSize);

void *coreLib;
typedef void (*AudioUpdate)(AudioContext *);
typedef void (*UpdateFunc)(RuntimeContext *);
typedef void (*InitFunc)(RuntimeContext *);
UpdateFunc update;
AudioUpdate audioUpdate;
static const char *_isPanicked = NULL;

static char consoleBuffer[0x10000];
void consoleLog(const char *str)
{
    int blen = strlen(consoleBuffer);
    int slen = strlen(str);
    if (blen + slen >= sizeof(consoleBuffer) - 1)
    {
        // find first linebreak and move memory after it to the start
        char *linebreak = strchr(consoleBuffer + slen, '\n');
        if (linebreak)
        {
            int len = strlen(linebreak);
            memmove(consoleBuffer, linebreak, len);
            consoleBuffer[len] = '\0';
        }
        else
        {
            consoleBuffer[0] = '\0';
        }
    }

    strcat(consoleBuffer, str);
    strcat(consoleBuffer, "\n");
}

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

void buildCoreDLL(int forceRebuild, RuntimeContext *ctx)
{
    audioUpdate = NULL;
    update = NULL;
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
    printf("Checking for header changes, forceRebuild=%d\n", forceRebuild);
    for (int i = 0; i < list.count; i++)
    {
        if (strcmp(GetFileExtension(list.paths[i]), ".h") == 0)
        {
            struct stat srcStat, objStat;
            int srcExists = stat(list.paths[i], &srcStat);
            char objFile[256];
            char cFile[256];
            sprintf(cFile, "src_engine/%s.c", GetFileNameWithoutExt(list.paths[i]));
            sprintf(objFile, "_obj/%s.o", GetFileNameWithoutExt(list.paths[i]));
            strReplaceDirSeps(objFile + 5);
            strReplaceDirSeps(cFile);
            int cExists = stat(cFile, &srcStat);
            if (cExists) continue;
            int objExists = stat(objFile, &objStat);

            // Compile if the object file doesn't exist or the source file is newer
            if (objExists != 0 || srcStat.st_mtime > objStat.st_mtime)
            {
                printf("Header change detected, rebuilding all: %s (%d %d)\n", list.paths[i], cExists, objExists);
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
            sprintf(args->command, "gcc -Wall -Wextra -Wincompatible-pointer-types -Wno-unused-parameter -DPLATFORM_DESKTOP -c -g -o %s %s -Isrc_engine -I_src_gen", coreObjFiles[i], coreSrcFiles[i]);
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

    sprintf(coreCommand, "gcc -Wall -Wextra -Wincompatible-pointer-types -Wno-unused-parameter -DPLATFORM_DESKTOP -shared -g -o %s %s", coreDLL, objfiles);
    printf("Building core DLL: %s\n", coreCommand);
    system(coreCommand);

    // Load the DLL and get the update function
    coreLib = loadLibrary("core.dll");
    update = getFunction(coreLib, "update");

    audioUpdate = getFunction(coreLib, "audioUpdate");
    
    _isPanicked = NULL;
    
    InitFunc init = getFunction(coreLib, "init");
    if (init != NULL && setjmp(panicJmpBuf) == 0)
        init(ctx);
    else
    {
        printf("Panic in init function\n");
    }
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

void copyFilesToDir(const char *srcDir, const char *dstDir)
{
    char command[256];
    sprintf(command, "xcopy /Y /E /I %s %s", srcDir, dstDir);
    system(command);
}

void _DLLPanic(const char *msg)
{
    printf("PANIC: %s\n", msg);
    _isPanicked = msg;
    longjmp(panicJmpBuf, 1);
    printf("This should not print\n");
}

uint32_t max_uint32(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

void updateEngine(RuntimeContext *ctx, AudioContext *audioCtx, Texture2D texture, Texture2D overdrawTexture, Font myMonoFont, int isExtended, int screenWidth, int screenHeight)
{
    int step = 0;
    ctx->frameCount++;
    ctx->previousInputState = ctx->inputState;

    ctx->inputA = IsKeyDown(KEY_I);
    ctx->inputB = IsKeyDown(KEY_J);
    ctx->inputUp = IsKeyDown(KEY_W);
    ctx->inputDown = IsKeyDown(KEY_S);
    ctx->inputLeft = IsKeyDown(KEY_A);
    ctx->inputRight = IsKeyDown(KEY_D);
    ctx->inputShoulderLeft = IsKeyDown(KEY_Q);
    ctx->inputShoulderRight = IsKeyDown(KEY_E);
    ctx->inputMenu = IsKeyDown(KEY_M);
    ctx->time += GetFrameTime();
    ctx->deltaTime = GetFrameTime();

    memset(ctx->frameStats.overdrawCount, 0, sizeof(ctx->frameStats.overdrawCount));

    if (update != NULL && setjmp(panicJmpBuf) == 0)
    {
        for (int i = 0; i < 5; i++)
        {
            ctx->sfxChannelStatus[i] = audioCtx->outSfxChannelStatus[i];
        }
        update(ctx);
        for (int i=0;i<5;i++)
        {
            audioCtx->inSfxInstructions[i] = ctx->outSfxInstructions[i];
        }

    }
    else
    {
        printf("Panic in update function\n");
        // do nothing
    }
    uint32_t maxOverdraw = 0;
    for (int i = 0; i < 128 * 128; i++)
    {
        maxOverdraw = max_uint32(maxOverdraw, ctx->frameStats.overdrawCount[i]);
    }

    for (int i = 0; i < 128 * 128; i++)
    {
        float overdraw = (float)ctx->frameStats.overdrawCount[i] / maxOverdraw * 2.0f;
        Color c = overdraw <= 1.0f ?
            (Color){(uint8_t)((1.0f - overdraw) * 255), (uint8_t)((overdraw) * 255), 0, 255} :
            (Color){0, (uint8_t)(1.0f-(overdraw - 1.0f) * 255), (uint8_t)(((overdraw - 1.0f)) * 255), 255};
        ctx->frameStats.overdrawCount[i] = c.a << 24 | c.r << 16 | c.g << 8 | c.b;
    }
    uint32_t screenData[128 * 128];
    for (int i = 0; i < 128 * 128; i++)
    {
        uint32_t c = ctx->screenData[i];
        screenData[i] = c | 0xFF000000;
    }
    UpdateTexture(texture, screenData);
    UpdateTexture(overdrawTexture, ctx->frameStats.overdrawCount);
    if (isExtended) {
        SetTextLineSpacing(-2);
        const float spacing = -1.0f;
        DrawTextEx(myMonoFont, TextFormat("FPS: %d, maxOverdraw=%d", GetFPS(), maxOverdraw), 
            (Vector2){ 10, 10 + screenHeight / 2}, myMonoFont.baseSize, spacing, WHITE);
        // print last 10 log lines
        char *endOfLog = &consoleBuffer[strlen(consoleBuffer)];
        char *lastLine = endOfLog;
        int line = 10;
        while (line >= 0 && lastLine > consoleBuffer)
        {
            lastLine--;
            if (*lastLine == '\n')
            {
                line--;
            }
        }
        if (*lastLine == '\n')
            lastLine++;
        DrawTextEx(myMonoFont, lastLine, (Vector2){ 10, 28 + screenHeight / 2}, myMonoFont.baseSize, spacing, WHITE);
    }
}

typedef struct ModMusicPlayState
{
    pthread_t thread;
    pthread_mutex_t mutex;
    Music music;
    int rewind;
    int isPlaying;
    int exit;
} ModMusicPlayState;

void* ModMusicPlayState_thread(void *arg)
{
    ModMusicPlayState *state = (ModMusicPlayState *)arg;
    printf("Music thread started\n");
    while (!state->exit)
    {
        pthread_mutex_lock(&state->mutex);
        if (state->rewind)
        {
            StopMusicStream(state->music);
            PlayMusicStream(state->music);
            state->rewind = 0;
        }
        if (state->isPlaying)
        {
            UpdateMusicStream(state->music);
        }
        pthread_mutex_unlock(&state->mutex);
    }

    return NULL;
}

void ModMusicPlayState_init(ModMusicPlayState *state)
{
    FilePathList list = LoadDirectoryFiles("dev_mod_files");
    for (int i = 0; i < list.count; i++)
    {
        printf("Found mod file: %s\n", list.paths[i]);
    }
    state->music = LoadMusicStream(list.paths[0]);
    UnloadDirectoryFiles(list);
    
    state->isPlaying = 0;
    pthread_mutex_init(&state->mutex, NULL);
    pthread_create(&state->thread, NULL, (void *(*)(void *))ModMusicPlayState_thread, state);
}

void ModMusicPlayState_update(ModMusicPlayState *state)
{
    if (IsKeyPressed(KEY_F7))
    {
        pthread_mutex_lock(&state->mutex);
        if (state->isPlaying)
        {
            PauseMusicStream(state->music);
            state->isPlaying = 0;
        }
        else
        {
            PlayMusicStream(state->music);
            state->isPlaying = 1;
        }
        printf("Music playing: %d\n", state->isPlaying);
        pthread_mutex_unlock(&state->mutex);
    }
    
    if (IsKeyPressed(KEY_F8))
    {
        pthread_mutex_lock(&state->mutex);
        printf("Rewind music\n");
        state->rewind = 1;
        pthread_mutex_unlock(&state->mutex);
    }
}

typedef struct InputState
{
    uint8_t inputRight, inputLeft, inputUp, inputDown, inputA, inputB, inputShoulderLeft, inputShoulderRight, inputMenu;
} InputState;

typedef struct SerialRemote
{
    int running;
    void *hComm;
    pthread_t thread;
    pthread_mutex_t mutex;

    InputState currentInput;
    InputState remoteInput;
} SerialRemote;


int SerialRemote_updateInput(SerialRemote *remote, void *hComm, uint8_t *remoteInput, uint8_t *input, const char *onChar, const char *offChar)
{
    if (*remoteInput != *input)
    {
        if (sendData(hComm, *input ? onChar : offChar))
        {
            printf("Error sending data\n");
            return 0;
        }
        else 
        {
            printf("Sent data: %s\n", *input ? onChar : offChar);
            *remoteInput = *input;
        }
    }
    return 1;
}

void* SerialRemote_thread(void *arg)
{
    SerialRemote *remote = (SerialRemote *)arg;
    printf("Serial remote thread started\n");
    const char *comports[] = {
        "COM3",
        "COM4",
        "COM5",
        "COM6",
        "COM7",
        "COM8",
        "COM9",
    };
    void *hComm = NULL;

    while (1)
    {
        nanosleep((struct timespec[]){{0, 
            hComm ? 100000 : 1000000
        }}, NULL);

        for (int i = 0; i < sizeof(comports) / sizeof(comports[0]) && hComm == NULL; i++)
        {
            hComm = configureCOMPort(comports[i]);
            if (hComm)
            {
                printf("Connected to COM port: %s\n", comports[i]);
                // test if this com port has the device we are looking for
                char buffer[256];
                if (sendData(hComm, "?"))
                {
                    printf("Error sending data\n");
                    closeCOMPort(hComm);
                    hComm = NULL;
                }
                else
                {
                    printf("Sent data: ?\n");
                    int bytesRead = receiveData(hComm, buffer, sizeof(buffer));
                    if (bytesRead > 0)
                    {
                        buffer[bytesRead] = '\0';
                        if (strncmp(buffer, "Input: ?", 8) == 0)
                        {
                            printf("Handshake successful\n");
                        }
                        else
                        {
                            printf("Received unexpected data: %s\n", buffer);
                            closeCOMPort(hComm);
                            hComm = NULL;
                        }
                    }
                    else if (bytesRead < 0)
                    {
                        printf("Error receiving data\n");
                        closeCOMPort(hComm);
                        hComm = NULL;
                    }
                }
            }
        }
    

        pthread_mutex_lock(&remote->mutex);
        if (!remote->running)
        {
            pthread_mutex_unlock(&remote->mutex);
            break;
        }

        if (hComm)
        {
            if (!SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputRight, &remote->currentInput.inputRight, "D", "d") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputLeft, &remote->currentInput.inputLeft, "A", "a") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputUp, &remote->currentInput.inputUp, "W", "w") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputDown, &remote->currentInput.inputDown, "S", "s") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputA, &remote->currentInput.inputA, "I", "i") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputB, &remote->currentInput.inputB, "J", "j") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputShoulderLeft, &remote->currentInput.inputShoulderLeft, "Q", "q") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputShoulderRight, &remote->currentInput.inputShoulderRight, "E", "e") ||
                !SerialRemote_updateInput(remote, hComm, &remote->remoteInput.inputMenu, &remote->currentInput.inputMenu, "M", "m")
                )
            {
                closeCOMPort(hComm);
                hComm = NULL;
            }
        }

        pthread_mutex_unlock(&remote->mutex);
    }
    if (hComm)
    {
        closeCOMPort(hComm);
    }
    return NULL;
}

void SerialRemote_init(SerialRemote *remote)
{
    remote->running = 1;
    pthread_mutex_init(&remote->mutex, NULL);
    pthread_create(&remote->thread, NULL, (void *(*)(void *))SerialRemote_thread, remote);
}

void SerialRemote_update(SerialRemote *remote)
{
    pthread_mutex_lock(&remote->mutex);
    remote->currentInput.inputRight = IsKeyDown(KEY_D);
    remote->currentInput.inputLeft = IsKeyDown(KEY_A);
    remote->currentInput.inputUp = IsKeyDown(KEY_W);
    remote->currentInput.inputDown = IsKeyDown(KEY_S);
    remote->currentInput.inputA = IsKeyDown(KEY_I);
    remote->currentInput.inputB = IsKeyDown(KEY_J);
    remote->currentInput.inputShoulderLeft = IsKeyDown(KEY_Q);
    remote->currentInput.inputShoulderRight = IsKeyDown(KEY_E);
    remote->currentInput.inputMenu = IsKeyDown(KEY_M);
    pthread_mutex_unlock(&remote->mutex);
}

void SerialRemote_exit(SerialRemote *remote)
{
    pthread_mutex_lock(&remote->mutex);
    remote->running = 0;
    pthread_mutex_unlock(&remote->mutex);
    pthread_join(remote->thread, NULL);
}

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define SAMPLE_RATE 20050
#define SAMPLE_SIZE 16
#define AUDIO_BUFFER_SIZE 4096
AudioStream audioStream;


// Cycles per second (hz)
float frequency = 440.0f;

// Audio frequency, for smoothing
float audioFrequency = 440.0f;

// Previous value, used to test if sine needs to be rewritten, and to smoothly modulate frequency
float oldFrequency = 1.0f;

// Index for audio rendering
float sineIdx = 0.0f;

// Audio input processing callback
void AudioInputCallback(void *buffer, unsigned int frames)
{
    audioFrequency = frequency + (audioFrequency - frequency)*0.95f;

    float incr = audioFrequency/44100.0f;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        d[i] = (short)(32000.0f*sinf(2*PI*sineIdx));
        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}

static int isPaused = 0;
static int step = 0;
static int stepAudio = 0;
static RuntimeContext ctx = {0};
static AudioContext audioCtx = {0};

void AudioSystemCallback(void *buffer, unsigned int frames)
{
    for (unsigned int i = 0; i < frames; i++)
    {
        ((short *)buffer)[i] = 0;
    }
    if (audioUpdate != NULL && (!isPaused || stepAudio))
    {
        stepAudio = 0;
        for (int i=0;i<5;i++)
        {
            audioCtx.inSfxInstructions[i] = ctx.outSfxInstructions[i];
            ctx.outSfxInstructions[i] = (SFXInstruction){0};
        }
        audioCtx.frames = frames;
        audioCtx.sampleRate = SAMPLE_RATE;
        audioCtx.sampleSize = SAMPLE_SIZE;
        audioCtx.outBuffer = buffer;
        audioUpdate(&audioCtx);
        for (int i=0;i<5;i++)
        {
            ctx.sfxChannelStatus[i] = audioCtx.outSfxChannelStatus[i];
            audioCtx.inSfxInstructions[i] = (SFXInstruction){0};
        }
    }
}

int main(void)
{
    // Initialization
    int screenWidth = 128 * 2;
    int screenHeight = 128 * 2;
    int isExtended = 0;
    ModMusicPlayState musicState = {0};
    // SerialRemote remote = {0};
    
    InitWindow(screenWidth, screenHeight, "Thumby color engine simulator");
    InitAudioDevice();
    audioStream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE, 1);
    SetAudioStreamCallback(audioStream, AudioSystemCallback);
    PlayAudioStream(audioStream);

    printf("sizeof(RuntimeContext) = %d\n", sizeof(RuntimeContext));

    // ModMusicPlayState_init(&musicState);
    // SerialRemote_init(&remote);

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    ctx.log = consoleLog;
    ctx.panic = _DLLPanic;
    buildCoreDLL(1, &ctx);

    ctx.getUTime = getUTime;

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

    Font myMonoFont = LoadFont("assets/fnt_mymono.png");

    int loadSkip = 0;
    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // ModMusicPlayState_update(&musicState);
        if (IsKeyPressed(KEY_F10))
        {
            // copy source to pico2 build dir
            copyFilesToDir("src_engine", "C:\\devel\\rapico2\\test\\blink\\game");
            copyFilesToDir("_src_gen", "C:\\devel\\rapico2\\test\\blink\\_src_gen");
        }
        if (IsKeyPressed(KEY_P)) {
            isPaused = !isPaused;
            printf("Paused: %d\n", isPaused);
        }
        if (IsKeyPressed(KEY_LEFT_SHIFT) && isPaused)
        {
            step = 1;
            stepAudio = 1;
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
            buildCoreDLL(IsKeyDown(KEY_LEFT_CONTROL), &ctx);
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

        if (_isPanicked)
        {
            DrawText(_isPanicked, 10, 10, 20, RED);
        }
        else if ((!isPaused || step) && loadSkip-- <= 0)
        {
            updateEngine(&ctx, &audioCtx, texture, overdrawTexture, myMonoFont, isExtended, screenWidth, screenHeight);
            step = 0;
        }
        
        rlDisableColorBlend();
        DrawTextureEx(texture, offset, 0.0f, scale, WHITE);
        DrawTextureEx(overdrawTexture, (Vector2){offset.x + 128 * scale, offset.y}, 0.0f, scale, WHITE);

        // draw audio buffer
        DrawRectangle(0, screenHeight - 100, screenWidth, 100, BLACK);
        uint16_t *audioBuffer = (uint16_t *)audioCtx.outBuffer;
        for (int i = 0; i < 256; i++)
        {
            uint16_t sample = audioBuffer[i] >> 10;
            DrawRectangle(i, screenHeight - 100 + sample, 1, sample, RED);
        }
        
        rlEnableColorBlend();

        EndDrawing();

        // copy screen data to clipboard
        if (IsKeyPressed(KEY_C))
        {
            // compress screenshot data in memory to PNG format
            copyScreenShot();
        }

        // remote input updates
        // SerialRemote_update(&remote);
    }
    musicState.exit = 1;
    pthread_join(musicState.thread, NULL);
    CloseAudioDevice();
    // SerialRemote_exit(&remote);
    // De-Initialization
    CloseWindow(); // Close window and OpenGL context
}