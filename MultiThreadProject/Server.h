#pragma once
#include <thread>
#include <iostream>
#include <conio.h>
#include <ctime>
#include <stdlib.h> 
#include "Common.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std::literals::chrono_literals;

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;
DWORD recvTimeout = 20;

std::unordered_map<Game::GUID, SOCKET> clients;
std::mutex clientsLock;

void BroadcastMessage(Message* message)
{
    for (auto iterator = clients.begin(); iterator != clients.end(); ++iterator)
    {
        SendNetworkMessage(message, (*iterator).second);
    }
}

void OnMessageReceived(char* buffer, SOCKET socket)
{
    Message* pBaseMessage = (Message*)buffer;
    switch (pBaseMessage->Type)
    {
    case Message::MessageType::PickItem:
    {
        printf("Received PickItem message\n");
        PickItemMessage* pMessage = new PickItemMessage(*(PickItemMessage*)buffer);
        // Apply
        BroadcastMessage(pMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::DropItem:
    {
        printf("Received DropItem message\n");
        DropItemMessage* pMessage = (DropItemMessage*)buffer;
        // Validate first
        MultiThreadGame::Instance.GetClient(pMessage->ClientGUID);
        // Apply
        BroadcastMessage(pMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    case Message::MessageType::GiveItem:
    {
        printf("Received GiveItem message\n");
        GiveItemMessage* pMessage = (GiveItemMessage*)buffer;
        // Validate first
        // Apply
        BroadcastMessage(pMessage);
        MultiThreadGame::Instance.QueueMessage(pMessage);
        break;
    }
    default:
        break;
    }
}

void ServerProcessInput(char input)
{
    switch (input)
    {
    case 'x':
    {
        printf("Shutting down\n");
        running = false;
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

void RegisterClientThread()
{
    while (running)
    {
        SOCKET Socket = INVALID_SOCKET;
        // Accept a client socket (This waits for a connection to come)
        Socket = accept(ListenSocket, NULL, NULL);
        if (Socket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
        }
        else
        {
            MultiThreadClient* client = new MultiThreadClient();
            clientsLock.lock();
            setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));
            clients[client->ClientId] = Socket;
            clientsLock.unlock();
            MultiThreadGame::Instance.ApplyOnEachClient([Socket](MultiThreadClient* client) 
                {
                    RegisterPlayerMessage message;
                    client->FillRegisterPlayerMessageContent(&message);
                    SendNetworkMessage(&message, Socket);
                });

            MultiThreadGame::Instance.RegisterClient(client);
            RegisterPlayerMessage message;
            client->FillRegisterPlayerMessageContent(&message);
            BroadcastMessage(&message);
        }
    }

    // No longer need server socket
    closesocket(ListenSocket);
}

void ReceiveMessageThread()
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int index = 0;
    size_t count;
    SOCKET socket;
    auto it = clients.begin();
    int iResult;

    while (running)
    {
        clientsLock.lock();
        count = clients.size();

        // There is an issue here when I have more than one client, it will wait to receive a message from both socket. I might consider using only one socket and add an id to identify client to have multiple clients on same socket ? (maybe a bad idea)
        for (it = clients.begin(); it != clients.end(); ++it)
        {
            socket = (*it).second;
            iResult = recv(socket, recvbuf, recvbuflen, 0);
            if (iResult > 0)
            {
                printf("Bytes received: %d from %s\n", iResult, ((*it).first).ToString().c_str());
                OnMessageReceived(recvbuf, socket);
            }
            else if (iResult == 0 || (iResult == -1 && WSAGetLastError() == 10054)) // 10054 is a connection break from client
            {
                printf("Connection closing...\n");
                closesocket(socket);
                // unregister client
                MultiThreadGame::Instance.UnregisterClient((*it).first);
                // [TODO] broadcast unregister client message
                clients.erase((*it).first);
                break;
            }
            else
            {
                if (WSAGetLastError() != 10060)
                {
                    printf("recv failed with error: %d\n", WSAGetLastError());
                }
            }
        }

        clientsLock.unlock();
        std::this_thread::sleep_for(20ms);
    }
}

int ServerMain()
{
    std::thread inputThread(ServerInputThread);

    // setup random
    time_t timer;
    time(&timer);
    srand((unsigned int)timer);

    // Window Socket data
    WSADATA wsaData;
    int iResult;

    struct addrinfo* result = NULL;
    struct addrinfo hints; 

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

    std::thread receiveClientThread(RegisterClientThread);

    // wait for a client to connect
    while (running && clients.size() == 0)
    {
        std::this_thread::sleep_for(20ms);
    }

    std::thread receiveMessageThread(ReceiveMessageThread);

    // Receive until the peer shuts down the connection
    while (running)
    {
        MultiThreadGame::Instance.ProcessMessages();
    }

    receiveMessageThread.detach();

    clientsLock.lock();
    if (clients.size() > 0)
    {
        SOCKET socket;
        for (auto it = clients.begin(); it != clients.end(); ++it)
        {
            socket = (*it).second;
            // shutdown the connection since we're done
            iResult = shutdown(socket, SD_SEND);
            if (iResult == SOCKET_ERROR) {
                printf("shutdown failed with error: %d\n", WSAGetLastError());
                closesocket(socket);
                WSACleanup();
                return 1;
            }

            // cleanup
            closesocket(socket);
        }
    }
    clientsLock.unlock();

    receiveClientThread.detach();

    WSACleanup();
    inputThread.join();
    return 0;
}