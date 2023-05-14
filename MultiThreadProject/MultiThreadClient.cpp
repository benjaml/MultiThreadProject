#include <iostream>
#include <string>
#include "MultiThreadClient.h"

void MultiThreadClient::PickItem(std::string name, int amount)
{
    lock.lock();
    inventory[name] += amount;
    lock.unlock();
}

void MultiThreadClient::DropItem(std::string name, int amount)
{
    lock.lock();
    inventory[name] -= inventory[name] > amount ? amount : inventory[name];
    lock.unlock();
}

int MultiThreadClient::GetItemCount(std::string name)
{
    lock.lock();
    int value = inventory[name];
    lock.unlock();
    return value;
}

void MultiThreadClient::FillRegisterPlayerMessageContent(RegisterPlayerMessage* message)
{
    message->ClientGUID = ClientId;
    std::string inventoryString;
    for (auto iterator = inventory.begin(); iterator != inventory.end(); ++iterator)
    {
        inventoryString += (*iterator).first + ":" + std::to_string((*iterator).second) + "/";
    }

    if (!inventoryString.empty())
    {
        // Clean last "/"
        inventoryString.erase(inventoryString.end() - 1);

        message->InventoryString = inventoryString;
    }
}

MultiThreadClient* MultiThreadClient::CreateFromRegisterPlayerMessage(RegisterPlayerMessage* message)
{
    MultiThreadClient* client = new MultiThreadClient(message->ClientGUID);
    if (!message->InventoryString.empty())
    {
        std::istringstream stringStream(message->InventoryString);
        std::string substring;
        while (std::getline(stringStream, substring, '/'))
        {
            size_t separator = substring.find(':');
            std::string itemName = substring.substr(0, separator);
            int amount = std::atoi(substring.substr(separator + 1, substring.length() - (separator + 1)).c_str());
            client->inventory[itemName] = amount;
        }
    }

    return client;
}

MultiThreadClient::MultiThreadClient()
{
    ClientId = Game::GUID::GetNextGUID();
}

MultiThreadClient::MultiThreadClient(Game::GUID guid)
{
    ClientId = guid;
}

MultiThreadClient::~MultiThreadClient()
{
    std::cout << "Delete client : " << ClientId.ToString() << std::endl;
}

