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

void* getFunction(void* lib, const char *name)
{
    void* fn = (void*)GET_FUNCTION((LIBRARY_HANDLE)lib, name);
    if (!fn)
    {
        fprintf(stderr, "Error getting function: %s\n", name);
        CLOSE_LIBRARY((LIBRARY_HANDLE)lib);
        exit(1);
    }
    return fn;
}

// COM port serial communication functions

// Function to configure the COM port
void* configureCOMPort(const char *portName) {
    HANDLE hComm = CreateFile(portName,                // Port name
                              GENERIC_READ | GENERIC_WRITE, // Read/Write access
                              0,                          // No sharing
                              NULL,                       // Default security attributes
                              OPEN_EXISTING,              // Opens existing port
                              0,                          // Non-overlapped I/O
                              NULL);                      // Null for COM devices

    if (hComm == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening COM port: %s\n", portName);
        return NULL;
    }

    // Configure the COM port
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hComm, &dcbSerialParams)) {
        fprintf(stderr, "Error getting COM state\n");
        CloseHandle(hComm);
        return NULL;
    }

    dcbSerialParams.BaudRate = CBR_115200; // Set baud rate to 9600
    dcbSerialParams.ByteSize = 8;        // Data bits = 8
    dcbSerialParams.StopBits = ONESTOPBIT; // One stop bit
    dcbSerialParams.Parity = NOPARITY;   // No parity

    if (!SetCommState(hComm, &dcbSerialParams)) {
        fprintf(stderr, "Error setting COM state\n");
        CloseHandle(hComm);
        return NULL;
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hComm, &timeouts)) {
        fprintf(stderr, "Error setting COM timeouts\n");
        CloseHandle(hComm);
        return NULL;
    }

    return hComm;
}

// Function to send data to the COM port
int sendData(void* hComm, const char *data) {
    DWORD bytesWritten = 0;
    int dataLength = strlen(data);
    printf("Sending data: %s\n", data);
    if (!WriteFile((HANDLE)hComm, data, dataLength, &bytesWritten, NULL)) {
        fprintf(stderr, "Error writing to COM port\n");
        return 1;
    }
    printf("Sent %d bytes\n", bytesWritten);
    return bytesWritten == dataLength ? 0 : 2;
}

// Function to receive data from the COM port
int receiveData(void* hComm, char *buffer, int bufferSize) {
    DWORD bytesRead = 0;
    if (!ReadFile((HANDLE)hComm, buffer, bufferSize, &bytesRead, NULL)) {
        fprintf(stderr, "Error reading from COM port\n");
        return -1;
    }
    buffer[bytesRead] = '\0'; // Null-terminate the received data
    return bytesRead;
}

void closeCOMPort(void* hComm)
{
    CloseHandle((HANDLE)hComm);
}