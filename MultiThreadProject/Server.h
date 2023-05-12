#pragma once
#include <thread>
#include <iostream>
#include <conio.h>
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"
#include "Defines.h"
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std::literals::chrono_literals;

void ServerProcessInput(char input)
{
    switch (input)
    {
    case 'x':
    {
        std::cout << "Shutting down" << std::endl;
        running = false;
        break;
    }
    case '+':
    {
        std::cout << "New Client" << std::endl;
        MultiThreadClient* client = new MultiThreadClient();
        MultiThreadGame::Instance.RegisterClient(client);
        break;
    }
    case '1':
    {
        try
        {
            MultiThreadClient* client = MultiThreadGame::Instance.GetRandomClient();
            PickItemOrder* order = new PickItemOrder(client->ClientId, MONEY, 20);
            MultiThreadGame::Instance.QueueOrder(order);
        }
        catch (std::exception e)
        {
            std::cerr << "Cannot do pick item because of exception : " << e.what() << std::endl;
        }

        break;
    }
    case '2':
    {
        try
        {
            MultiThreadClient* client = MultiThreadGame::Instance.GetRandomClient();
            DropItemOrder* order = new DropItemOrder(client->ClientId, MONEY, 20);
            MultiThreadGame::Instance.QueueOrder(order);
        }
        catch (std::exception e)
        {
            std::cerr << "Cannot do drop item because of exception : " << e.what() << std::endl;
        }

        break;
    }
    case '3':
    {
        try
        {
            MultiThreadClient* fromClient = MultiThreadGame::Instance.GetRandomClient();
            MultiThreadClient* toClient = MultiThreadGame::Instance.GetRandomClient();
            while (fromClient == toClient)
            {
                toClient = MultiThreadGame::Instance.GetRandomClient();
            }

            GiveItemOrder* order = new GiveItemOrder(fromClient->ClientId, toClient->ClientId, MONEY, 20);
            MultiThreadGame::Instance.QueueOrder(order);
        }
        catch (std::exception e)
        {
            std::cerr << "Cannot do give item because of exception : " << e.what() << std::endl;
        }
        break;
    }

    default:
        std::cout << input << std::endl;
    }
}

void ServerInputThread()
{
    while (running)
    {
        char input = 0;
        if (_kbhit() != 0)
        {
            char input = _getch();
            ServerProcessInput(input);
        }
    }
}

int ServerMain()
{
    // Window Socket data
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints; 
    
    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket (This waits for a connection to come)
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    
    // No longer need server socket
    closesocket(ListenSocket);
    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);

    WSACleanup();
    return 0;
}