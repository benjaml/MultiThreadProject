#pragma once
#include <thread>
#include <iostream>
#include <conio.h>
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"
#include "Defines.h"

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
    std::thread inputThread(ServerInputThread);

    while (running)
    {
        MultiThreadGame::Instance.ApplyOnEachClient([](MultiThreadClient* pClient) {
            MultiThreadGame::Instance.PickItem(pClient, MONEY, 1);
            });

        MultiThreadGame::Instance.ProcessOrders();


        std::this_thread::sleep_for(20ms);
    }

    inputThread.join();
    return 0;
}