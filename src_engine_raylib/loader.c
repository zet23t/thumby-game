#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#define LOAD_LIBRARY(name) LoadLibrary(name)
#define GET_FUNCTION(lib, func) GetProcAddress(lib, func)
#define LIBRARY_HANDLE HMODULE
#define CLOSE_LIBRARY(lib) FreeLibrary(lib)
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(name) dlopen(name, RTLD_LAZY)
#define GET_FUNCTION(lib, func) dlsym(lib, func)
#define LIBRARY_HANDLE void*
#define CLOSE_LIBRARY(lib) dlclose(lib)
#endif



void* loadLibrary(const char *libName)
{
    LIBRARY_HANDLE lib = LOAD_LIBRARY(libName);
    if (!lib)
    {
        fprintf(stderr, "Error loading library: %s\n", libName);
        exit(1);
    }
    return lib;
}

void unloadLibrary(void* lib)
{
    CLOSE_LIBRARY((LIBRARY_HANDLE)lib);
}

void* getUpdateFunction(void* lib)
{
    void* update = (void*)GET_FUNCTION((LIBRARY_HANDLE)lib, "update");
    if (!update)
    {
        fprintf(stderr, "Error getting function: update\n");
        CLOSE_LIBRARY((LIBRARY_HANDLE)lib);
        exit(1);
    }
    return update;
}

void* getInitFunction(void *lib)
{
    void* init = (void*)GET_FUNCTION((LIBRARY_HANDLE)lib, "init");
    if (!init)
    {
        fprintf(stderr, "Error getting function: init\n");
        CLOSE_LIBRARY((LIBRARY_HANDLE)lib);
        exit(1);
    }
    return init;
}
