#pragma once
#include <thread>
#include <iostream>
#include <conio.h>
#include <ctime>
#include <stdlib.h> 
#include "Common.h"

using namespace std::literals::chrono_literals;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET ConnectSocket = INVALID_SOCKET;

void OnMessageReceived(char* buffer)
{
    Message* pBaseMessage = (Message*)buffer;
    switch (pBaseMessage->Type)
    {
    case Message::MessageType::RegisterPlayer:
    {
        printf("Received RegisterPlayer message\n");
        RegisterPlayerMessage* pMessage = new RegisterPlayerMessage(*(RegisterPlayerMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::PickItem:
    {
        printf("Received PickItem message\n");
        PickItemMessage* pMessage = new PickItemMessage(*(PickItemMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::DropItem:
    {
        printf("Received DropItem message\n");
        DropItemMessage* pMessage = new DropItemMessage(*(DropItemMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::GiveItem:
    {
        printf("Received GiveItem message\n");
        GiveItemMessage* pMessage = new GiveItemMessage(*(GiveItemMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    default:
        break;
    }
}

void ClientProcessInput(char input)
{
    switch (input)
    {
    case 'x':
    {
        std::cout << "Shutting down" << std::endl;
        running = false;

        // shutdown the connection since no more data will be sent
        int iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            running = false;
        }

        break;
    }
    case '1':
    {
        try
        {
            MultiThreadClient* client = MultiThreadGame::Instance.GetRandomClient();
            PickItemMessage* message = new PickItemMessage(client->ClientId, MONEY, 20);
            SendNetworkMessage(message, ConnectSocket);
            printf("Send PickItemMessage\n");
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
            DropItemMessage* message = new DropItemMessage(client->ClientId, MONEY, 20);
            SendNetworkMessage(message, ConnectSocket);
            printf("Send DropItemMessage\n");
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

            GiveItemMessage* message = new GiveItemMessage(fromClient->ClientId, toClient->ClientId, MONEY, 20);
            SendNetworkMessage(message, ConnectSocket);
            printf("Send GiveItemMessage\n");
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

void ClientInputThread()
{
    while (running)
    {
        char input = 0;
        if (_kbhit() != 0)
        {
            char input = _getch();
            ClientProcessInput(input);
        }
    }
}

void ReceiveMessageThread()
{
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    // Receive until the peer closes the connection
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            OnMessageReceived(recvbuf);
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            running = false;
        }

    } while (iResult > 0 || running);
}

int ClientMain()
{
    std::thread inputThread(ClientInputThread);
    // setup random
    time_t timer;
    time(&timer);
    srand(timer);

    WSADATA wsaData;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    
    std::thread receiveThread(ReceiveMessageThread);

    while (running)
    {
        MultiThreadGame::Instance.ProcessMessages();
    }

    receiveThread.detach();

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    inputThread.join();
    return 0;
}