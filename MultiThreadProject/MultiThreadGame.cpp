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

void MultiThreadGame::UnregisterClient(const Game::GUID& guid)
{
    lock.lock();
    delete clients[guid];
    clients.erase(guid);
    lock.unlock();
}

MultiThreadClient* const MultiThreadGame::GetRandomClient()
{
    lock.lock();
    size_t count = clients.size();
    if (count == 0)
    {
        lock.unlock();
        throw std::exception("Cannot Get a random client as there is none");
    }

    int random = rand() % count;
    auto iterator = clients.begin();
    std::advance(iterator, random);

    MultiThreadClient* result = (*iterator).second;
    lock.unlock();

    return result;
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

int MultiThreadGame::DropItem(MultiThreadClient* pClient, std::string_view name, int amount)
{
    if (pClient != NULL)
    {
        int remaining = pClient->GetItemCount(name);
        if (remaining < amount)
        {
            amount = remaining;
        }

        pClient->DropItem(name, amount);
        return amount;
    }
    else
    {
        std::cerr << "Trying to drop an item on a null client" << std::endl;
        return 0;
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

    int removed = Instance.DropItem(pFromClient, name, amount);
    if (removed < amount)
    {
        std::cout << "As giving client only have " << removed << " " << name << ", only giving " << removed << std::endl;
    }

    Instance.PickItem(pToClient, name, removed);
}

void MultiThreadGame::ApplyOnEachClient(std::function<void(MultiThreadClient*)> function)
{
    lock.lock();
    for (auto iterator = clients.begin(); iterator != clients.end(); ++iterator)
    {
        function((*iterator).second);
    }

    lock.unlock();
}

void MultiThreadGame::QueueMessage(Message* message)
{
    MessageStack.push(message);
}

void MultiThreadGame::ProcessOrders()
{
    while (!MessageStack.empty())
    {
        Message* message = MessageStack.front();
        MessageStack.pop();
        switch (message->Type)
        {
        case Message::MessageType::PickItem:
        {
            PickItemMessage* pMessage = (PickItemMessage*)message;

            MultiThreadClient* client = clients[pMessage->ClientGUID];
            if (client == NULL)
            {
                std::cerr << "PickItem : Invalid client GUID" << std::endl;
                continue;
            }
            else
            {
                std::cout << "Pick Item" << std::endl;
                Instance.PickItem(client, pMessage->ItemName, pMessage->Amount);
                std::cout << "Client " << client->ClientId.ToString() << " now have " << client->GetItemCount(pMessage->ItemName) << std::endl;
            }
        }

        break; 

        case Message::MessageType::DropItem:
        {
            DropItemMessage* pMessage = (DropItemMessage*)message;

            MultiThreadClient* client = clients[pMessage->ClientGUID];
            if (client == NULL)
            {
                std::cerr << "DropItem : Invalid client GUID" << std::endl;
                continue;
            }
            else
            {
                std::cout << "Drop Item" << std::endl;
                Instance.DropItem(client, pMessage->ItemName, pMessage->Amount);
                std::cout << "Client " << client->ClientId.ToString() << " now have " << client->GetItemCount(pMessage->ItemName) << std::endl;
            }
        }

        break;

        case Message::MessageType::GiveItem:
        {
            GiveItemMessage* pMessage = (GiveItemMessage*)message;

            MultiThreadClient* fromClient = clients[pMessage->ClientGUID];
            MultiThreadClient* toClient = clients[pMessage->ClientGUID];
            if (fromClient == NULL || toClient == NULL)
            {
                std::cerr << "DropItem : Invalid client GUID" << std::endl;
                continue;
            }
            else
            {
                std::cout << "Give Item" << std::endl;
                Instance.GiveItem(fromClient, toClient, pMessage->ItemName, pMessage->Amount);            
                std::cout << "Client " << fromClient->ClientId.ToString() << " now have " << fromClient->GetItemCount(pMessage->ItemName) << " and Client " << toClient->ClientId.ToString() << " now have " << toClient->GetItemCount(pMessage->ItemName) << std::endl;

            }
        }
            
        break;

        default:
            std::cerr << "Order not supported" << std::endl;
            break;
        }

        delete message;
    }
}

MultiThreadGame::MultiThreadGame()
{
}

MultiThreadGame::~MultiThreadGame()
{
    lock.lock();
    for (auto iterator = clients.begin(); iterator != clients.end(); ++iterator)
    {
        delete (*iterator).second;
    }

    lock.unlock();
}
