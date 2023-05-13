#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Message.h"
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"

#define MONEY "Money"

volatile bool running = true;

void SendNetworkMessage(Message* order, SOCKET socket)
{
    const size_t length = order->size();
    char* sendbuf = new char[length];
    memcpy(sendbuf, order, length);
    int iResult = send(socket, sendbuf, (int)length, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
    }
}