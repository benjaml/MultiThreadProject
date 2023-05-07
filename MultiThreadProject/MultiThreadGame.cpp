#include <iostream>
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"

MultiThreadGame MultiThreadGame::Instance;

void MultiThreadGame::RegisterClient(MultiThreadClient* pClient)
{
    if (pClient != NULL)
    {
        lock.lock();
        clients[pClient->ClientId] = pClient;
        lock.unlock();
    }
    else
    {
        std::cerr << "Trying to register a null client" << std::endl;
    }
}

MultiThreadClient* const MultiThreadGame::GetRandomClient()
{
    int count = clients.size();
    if (count == 0)
    {
        throw std::exception("Cannot Get a random client as there is none");
    }

    int random = rand() % count;
    auto iterator = clients.begin();
    for (int index = 0; index < random; index++)
    {
        iterator++;
    }

    return (*iterator).second;
}

void MultiThreadGame::PickItem(MultiThreadClient* pClient, std::string_view name, int amount)
{
    if (pClient != NULL)
    {
        pClient->PickItem(name, amount);
    }
    else
    {
        std::cerr << "Trying to pick an item on a null client" << std::endl;
    }
}

void MultiThreadGame::DropItem(MultiThreadClient* pClient, std::string_view name, int amount)
{
    if (pClient != NULL)
    {
        pClient->DropItem(name, amount);
    }
    else
    {
        std::cerr << "Trying to drop an item on a null client" << std::endl;
    }
}

void MultiThreadGame::GiveItem(MultiThreadClient* pFromClient, MultiThreadClient* pToClient, std::string_view name, int amount)
{
    if (pFromClient == NULL)
    {
        std::cerr << "Trying to give an item from a null client" << std::endl;
        return;
    }

    if (pToClient == NULL)
    {
        std::cerr << "Trying to give an item to a null client" << std::endl;
        return;
    }

    pFromClient->DropItem(name, amount);
    pToClient->PickItem(name, amount);
}

MultiThreadGame::MultiThreadGame()
{
}

MultiThreadGame::~MultiThreadGame()
{
    for (auto iterator = clients.begin(); iterator != clients.end(); iterator++)
    {
        delete (*iterator).second;
    }
}
