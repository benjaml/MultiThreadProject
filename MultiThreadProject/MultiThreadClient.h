#pragma once
#include <map>
#include <mutex>
#include "GUID.h"


class MultiThreadClient
{
public:
    Game::GUID ClientId;
    void PickItem(std::string_view name, int amount);
    void DropItem(std::string_view name, int amount);
    int GetItemCount(std::string_view name);

    MultiThreadClient();
    ~MultiThreadClient();

private:
    std::map<std::string_view, int> inventory;
    std::mutex lock;
};