#pragma once
#include <thread>
#include <iostream>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"
#include "Defines.h"

using namespace std::literals::chrono_literals;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET ConnectSocket = INVALID_SOCKET;

void SendMessage(Message* order)
{
    const int length = order->size();
    char* sendbuf = new char[length];
    memcpy(sendbuf, order, length);
    int iResult = send(ConnectSocket, sendbuf, length, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
    }
}

void OnMessageReceived(char* buffer)
{
    Message* pBaseMessage = (Message*)buffer;
    switch (pBaseMessage->Type)
    {
    case Message::MessageType::RegisterPlayerRequest:
    {
        RegisterPlayerRequestMessage* pMessage = new RegisterPlayerRequestMessage(*(RegisterPlayerRequestMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::RegisterPlayer:
    {
        RegisterPlayerMessage* pMessage = new RegisterPlayerMessage(*(RegisterPlayerMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::PickItem:
    {
        PickItemMessage* pMessage = new PickItemMessage(*(PickItemMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::DropItem:
    {
        DropItemMessage* pMessage = new DropItemMessage(*(DropItemMessage*)pBaseMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break; 
    }
    case Message::MessageType::GiveItem:
    {
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
        }

        break;
    }
    case '1':
    {
        try
        {
            MultiThreadClient* client = MultiThreadGame::Instance.GetRandomClient();
            PickItemMessage* message = new PickItemMessage(client->ClientId, MONEY, 20);
            SendMessage(message);
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
            SendMessage(message);
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
            SendMessage(message);
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

int ClientMain()
{
    std::thread inputThread(ClientInputThread);

    WSADATA wsaData;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
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
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0 || running);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    inputThread.join();
    return 0;
}