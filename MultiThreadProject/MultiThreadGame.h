#pragma once
#include <map>
#include <mutex>
#include <functional>
#include <queue>
#include "Message.h"
#include "MultiThreadClient.h"

class MultiThreadGame
{
public:
    static MultiThreadGame Instance;
    void RegisterClient(MultiThreadClient* pClient);
    void UnregisterClient(const Game::GUID& guid);
    MultiThreadClient* const GetRandomClient();
    void PickItem(MultiThreadClient* pClient, std::string_view name, int amount);
    int DropItem(MultiThreadClient* pClient, std::string_view name, int amount);
    void GiveItem(MultiThreadClient* pFromClient, MultiThreadClient* pToClient, std::string_view name, int amount);
    void ApplyOnEachClient(std::function<void(MultiThreadClient*)> function);
    void QueueMessage(Message* order);
    void ProcessOrders();

    MultiThreadGame();
    ~MultiThreadGame();

private:
    std::map<Game::GUID, MultiThreadClient*> clients;
    std::queue<Message*> MessageStack;
    std::mutex lock;
};