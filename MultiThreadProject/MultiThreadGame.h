#pragma once
#include <map>
#include <mutex>
#include <functional>
#include <queue>
#include "Order.h"
#include "MultiThreadClient.h"

class MultiThreadGame
{
public:
    static MultiThreadGame Instance;
    void RegisterClient(MultiThreadClient* pClient);
    MultiThreadClient* const GetRandomClient();
    void PickItem(MultiThreadClient* pClient, std::string_view name, int amount);
    int DropItem(MultiThreadClient* pClient, std::string_view name, int amount);
    void GiveItem(MultiThreadClient* pFromClient, MultiThreadClient* pToClient, std::string_view name, int amount);
    void ApplyOnEachClient(std::function<void(MultiThreadClient*)> function);
    void QueueOrder(Order&& order);

    MultiThreadGame();
    ~MultiThreadGame();

private:
    std::map<GUID, MultiThreadClient*> clients;
    std::queue<Order> OrderStack;
    std::mutex lock;
};