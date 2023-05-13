#pragma once
#include "GUID.h"

struct Message
{
	enum class MessageType
	{
		RegisterPlayer, // from server -> client
		PickItem,
		DropItem,
		GiveItem
	};

	MessageType Type;
	Game::GUID ClientGUID;

	virtual const size_t size()
	{
		return sizeof(Message);
	}
};

struct PickItemMessage : Message
{
	std::string ItemName;
	int Amount;

	PickItemMessage(Game::GUID clientGUID, std::string name, int amount)
	{
		Type = Message::MessageType::PickItem;
		ClientGUID = clientGUID;
		ItemName = name;
		Amount = amount;
	}

	virtual const size_t size() override
	{
		return sizeof(PickItemMessage);
	}
};

struct DropItemMessage : Message
{
	std::string ItemName;
	int Amount;

	DropItemMessage(Game::GUID clientGUID, std::string name, int amount)
	{
		Type = Message::MessageType::DropItem;
		ClientGUID = clientGUID;
		ItemName = name;
		Amount = amount;
	}

	virtual const size_t size() override
	{
		return sizeof(DropItemMessage);
	}
};

struct GiveItemMessage : Message
{
	Game::GUID OtherClientGUID;
	std::string ItemName;
	int Amount;

	GiveItemMessage(Game::GUID fromClientGUID, Game::GUID toClientGUID, std::string name, int amount)
	{
		Type = Message::MessageType::GiveItem;
		ClientGUID = fromClientGUID;
		OtherClientGUID = toClientGUID;
		ItemName = name;
		Amount = amount;
	}

	virtual const size_t size() override
	{
		return sizeof(GiveItemMessage);
	}
};

struct RegisterPlayerMessage : Message
{
	char* inventoryBuffer;
	size_t inventoryBufferSize;

	RegisterPlayerMessage()
	{
		Type = Message::MessageType::RegisterPlayer;
		inventoryBuffer = NULL;
		inventoryBufferSize = 0;
	}

	virtual const size_t size() override
	{
		return sizeof(Message) + inventoryBufferSize + sizeof(inventoryBufferSize);
	}
};