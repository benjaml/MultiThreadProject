#include <iostream>
#include "MultiThreadClient.h"

void MultiThreadClient::PickItem(std::string_view name, int amount)
{
    lock.lock();
    inventory[name] += amount;
    lock.unlock();
}

void MultiThreadClient::DropItem(std::string_view name, int amount)
{
    lock.lock();
    inventory[name] -= inventory[name] > amount ? amount : inventory[name];
    lock.unlock();
}

int MultiThreadClient::GetItemCount(std::string_view name)
{
    lock.lock();
    int value = inventory[name];
    lock.unlock();
    return value;
}

MultiThreadClient::MultiThreadClient()
{
    ClientId = Game::GUID::GetNextGUID();
}

MultiThreadClient::~MultiThreadClient()
{
    std::cout << "Delete client : " << ClientId.ToString() << std::endl;
}
