#include <iostream>
#include "MultiThreadGame.h"
#include "MultiThreadClient.h"

MultiThreadGame MultiThreadGame::Instance;

void MultiThreadGame::RegisterClient(MultiThreadClient* pClient)
{
    if (pClient != NULL)
    {
        lock.lock();
        printf("Registered client : %s\n", pClient->ClientId.ToString().c_str());
        clients[pClient->ClientId] = pClient;
        lock.unlock();
    }
    else
    {
        printf("Trying to register a null client\n");
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

void MultiThreadGame::PickItem(MultiThreadClient* pClient, std::string name, int amount)
{
    if (pClient != NULL)
    {
        pClient->PickItem(name, amount);
    }
    else
    {
        printf("Trying to pick an item on a null client\n");
    }
}

int MultiThreadGame::DropItem(MultiThreadClient* pClient, std::string name, int amount)
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
        printf("Trying to drop an item on a null client\n");
        return 0;
    }
}

void MultiThreadGame::GiveItem(MultiThreadClient* pFromClient, MultiThreadClient* pToClient, std::string name, int amount)
{
    if (pFromClient == NULL)
    {
        printf("Trying to give an item from a null client\n");
        return;
    }

    if (pToClient == NULL)
    {
        printf("Trying to give an item to a null client\n");
        return;
    }

    int removed = Instance.DropItem(pFromClient, name, amount);
    if (removed < amount)
    {
        printf("As giving client only have %d, only giving %d\n", removed, removed);
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

void MultiThreadGame::ProcessMessages()
{
    while (!MessageStack.empty())
    {
        Message* message = MessageStack.front();
        switch (message->Type)
        {
        case Message::MessageType::PickItem:
        {
            PickItemMessage* pMessage = (PickItemMessage*)message;

            MultiThreadClient* client = clients[pMessage->ClientGUID];
            if (client == NULL)
            {
                printf("PickItem : Invalid client GUID\n");
                continue;
            }
            else
            {
                printf("Pick Item\n");
                Instance.PickItem(client, pMessage->ItemName, pMessage->Amount);
                printf("Client %s now have %d %s\n", client->ClientId.ToString().c_str(), client->GetItemCount(pMessage->ItemName), pMessage->ItemName.c_str());
            }
        }

        break; 

        case Message::MessageType::DropItem:
        {
            DropItemMessage* pMessage = (DropItemMessage*)message;

            MultiThreadClient* client = clients[pMessage->ClientGUID];
            if (client == NULL)
            {
                printf("DropItem : Invalid client GUID\n");
                continue;
            }
            else
            {
                printf("Drop Item\n");
                Instance.DropItem(client, pMessage->ItemName, pMessage->Amount);
                printf("Client %s now have %d %s\n", client->ClientId.ToString().c_str(), client->GetItemCount(pMessage->ItemName), pMessage->ItemName.c_str());
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
                printf("DropItem : Invalid client GUID\n");
                continue;
            }
            else
            {
                printf("Give Item\n");
                Instance.GiveItem(fromClient, toClient, pMessage->ItemName, pMessage->Amount);
                printf("Client %s now have %d %s and Client %s now have %d %s\n", fromClient->ClientId.ToString().c_str(), fromClient->GetItemCount(pMessage->ItemName), pMessage->ItemName.c_str(), toClient->ClientId.ToString().c_str(), toClient->GetItemCount(pMessage->ItemName), pMessage->ItemName.c_str());
            }
        }

        break;

        case Message::MessageType::RegisterPlayer:
        {
            RegisterPlayerMessage* pMessage = (RegisterPlayerMessage*)message;

            MultiThreadClient* client = MultiThreadClient::CreateFromRegisterPlayerMessage(pMessage);

            // Make sure the client is not already existing before registering it
            if (clients.find(client->ClientId) == clients.end())
            {
                MultiThreadGame::Instance.RegisterClient(client);
            }
            else
            {
                delete client;
            }
        }

        break;

        default:
            printf("Order not supported\n");
            break;
        }

        delete MessageStack.front();
        MessageStack.pop();
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
