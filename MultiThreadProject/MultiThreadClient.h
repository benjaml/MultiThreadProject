#pragma once
#include <map>
#include <mutex>
#include "GUID.h"
#include "Message.h"


class MultiThreadClient
{
public:
    Game::GUID ClientId;
    static MultiThreadClient* CreateFromRegisterPlayerMessage(RegisterPlayerMessage* message);
    void PickItem(std::string name, int amount);
    void DropItem(std::string name, int amount);
    int GetItemCount(std::string name);
    void FillRegisterPlayerMessageContent(RegisterPlayerMessage* message);

    MultiThreadClient();
    MultiThreadClient(Game::GUID guid);
    ~MultiThreadClient();

private:
    std::map<std::string, int> inventory;
    std::mutex lock;
};