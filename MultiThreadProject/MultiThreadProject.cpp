#pragma once
#ifdef SERVER
#include "Server.h"
#endif // SERVER
#ifdef CLIENT
#include "Client.h"
#endif // CLIENT

int main(int argc, char* argv[])
{
#ifdef SERVER
    return ServerMain();
#endif //SERVER
#ifdef CLIENT
    return ClientMain();
#endif //CLIENT
    return 0;
}