// MultiThreadProject.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <thread>
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"

bool running = false;

void SendOrderThread()
{
    while (running)
    {

    }
}

int main()
{
    MultiThreadGame game;
    MultiThreadClient* client = new MultiThreadClient();
    MultiThreadGame::Instance.RegisterClient(client);

    client->PickItem("test", 1);

    if (client->GetItemCount("test") != 1)
    {
        std::cerr << "BAD NUMBER OF ITEM, expected 1" << std::endl;
    }

    std::cin.get();
    return 0;
}