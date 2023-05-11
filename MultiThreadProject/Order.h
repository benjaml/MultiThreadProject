#pragma once
#include "GUID.h"

struct Order
{
	enum class OrderType
	{
		PickItem,
		DropItem,
		GiveItem

	};

	OrderType Type;
	GUID ClientGUID;
};

struct OrderWithOtherPlayer : Order 
{
	GUID OtherClientGUID;
};

struct PickItemOrder : Order
{
	std::string_view ItemName;
	int Amount;

	PickItemOrder(GUID clientGUID, std::string_view name, int amount)
	{
		Type = Order::OrderType::PickItem;
		ClientGUID = clientGUID;
		ItemName = name;
		Amount = amount;
	}
};

struct DropItemOrder : Order
{
	std::string_view ItemName;
	int Amount;

	DropItemOrder(GUID clientGUID, std::string_view name, int amount)
	{
		Type = Order::OrderType::DropItem;
		ClientGUID = clientGUID;
		ItemName = name;
		Amount = amount;
	}
};

struct GiveItemOrder : OrderWithOtherPlayer
{
	std::string_view ItemName;
	int Amount;

	GiveItemOrder(GUID fromClientGUID, GUID toClientGUID, std::string_view name, int amount)
	{
		Type = Order::OrderType::GiveItem;
		ClientGUID = fromClientGUID;
		OtherClientGUID = toClientGUID;
		ItemName = name;
		Amount = amount;
	}
};