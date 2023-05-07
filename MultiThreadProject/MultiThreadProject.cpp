// MultiThreadProject.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <thread>
#include <conio.h>
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"

#define MONEY "Money"

using namespace std::literals::chrono_literals;
bool running = true;

void GameLoop()
{
    while (running)
    {
        MultiThreadGame::Instance.ApplyOnEachClient([](MultiThreadClient* pClient) {
            MultiThreadGame::Instance.PickItem(pClient, MONEY, 1);
        });

        std::this_thread::sleep_for(20ms);
    }
}

void ProcessInput(char input)
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
            std::cout << "Pick Item" << std::endl;
            MultiThreadGame::Instance.PickItem(client, MONEY, 20);
            std::cout << "Client " << client->ClientId.ToString() << " now have " << client->GetItemCount(MONEY) << std::endl;
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
            std::cout << "Drop Item" << std::endl;
            MultiThreadGame::Instance.DropItem(client, MONEY, 20);
            std::cout << "Client " << client->ClientId.ToString() << " now have " << client->GetItemCount(MONEY) << std::endl;
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

            std::cout << "Give Item" << std::endl;
            MultiThreadGame::Instance.GiveItem(fromClient, toClient, MONEY, 20);
            std::cout << "Client " << fromClient->ClientId.ToString() << " now have " << fromClient->GetItemCount(MONEY) << " and Client " << toClient->ClientId.ToString() << " now have " << toClient->GetItemCount(MONEY) << std::endl;
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

void InputThread()
{
    while (running)
    {
        char input = 0;
        if (_kbhit() != 0)
        {
            char input = _getch();
            ProcessInput(input);
        }
    }
}

int main()
{
    std::thread gameloop = std::thread(GameLoop);
    std::thread inputThread = std::thread(InputThread);

    gameloop.join();
    inputThread.join();
    return 0;
}