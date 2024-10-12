// TODO: Make this useful;
// The purpose of this server would be to serve the WASM builds of the project, triggering
// recompiles when the source files change, specifically the JS files that are used to display
// the game and handle user input.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!"

typedef struct TinyWebServer {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in server, client;
} TinyWebServer;

static TinyWebServer _server;

int serverInit()
{
    printf("\nInitializing Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &_server.wsa) != 0) {
        printf("Failed. Error Code: %d", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    if ((_server.serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created.\n");
    // Prepare the sockaddr_in structure
    _server.server.sin_family = AF_INET;
    _server.server.sin_addr.s_addr = inet_addr("127.0.0.1");
    _server.server.sin_port = htons(PORT);

    // Bind
    if (bind(_server.serverSocket, (struct sockaddr *)&_server.server, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d", WSAGetLastError());
        closesocket(_server.serverSocket);
        WSACleanup();
        return 1;
    }
    printf("Bind done.\n");

    // Listen to incoming connections
    listen(_server.serverSocket, 3);
    return 0;
}

void serverCleanup() {
    closesocket(_server.serverSocket);
    WSACleanup();
}

int main() {
    if (serverInit() != 0) {
        return 1;
    }

    // Accept and incoming connection
    printf("Waiting for incoming connections...\n");
    int c = sizeof(struct sockaddr_in);
    while ((_server.clientSocket = accept(_server.serverSocket, (struct sockaddr *)&_server.client, &c)) != INVALID_SOCKET) {
        printf("Connection accepted.\n");

        // Send the response
        send(_server.clientSocket, RESPONSE, strlen(RESPONSE), 0);
        closesocket(_server.clientSocket);
    }

    if (_server.clientSocket == INVALID_SOCKET) {
        printf("Accept failed with error code: %d", WSAGetLastError());
        closesocket(_server.serverSocket);
        WSACleanup();
        return 1;
    }

    serverCleanup();
    return 0;
}