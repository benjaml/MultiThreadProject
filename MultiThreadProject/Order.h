#pragma once
#include "GUID.h"

enum OrderType
{
	PickItem,
	DropItem,
	GiveItem

};

struct Order
{
	OrderType Type;
	GUID ClientGUID;
	char* Parameters;
	int ParameterSize;
};

struct OrderWithOtherPlayer : Order 
{
	GUID OtherClientGUID;
};