#pragma once
#include "GUID.h"

struct Message
{
	enum class MessageType
	{
		RegisterPlayerRequest, // from client -> server
		RegisterPlayer, // from server -> client
		PickItem,
		DropItem,
		GiveItem
	};

	MessageType Type;
	Game::GUID ClientGUID;

	virtual const int size()
	{
		return sizeof(Message);
	}
};

struct PickItemMessage : Message
{
	std::string_view ItemName;
	int Amount;

	PickItemMessage(Game::GUID clientGUID, std::string_view name, int amount)
	{
		Type = Message::MessageType::PickItem;
		ClientGUID = clientGUID;
		ItemName = name;
		Amount = amount;
	}

	virtual const int size() override
	{
		return sizeof(PickItemMessage);
	}
};

struct DropItemMessage : Message
{
	std::string_view ItemName;
	int Amount;

	DropItemMessage(Game::GUID clientGUID, std::string_view name, int amount)
	{
		Type = Message::MessageType::DropItem;
		ClientGUID = clientGUID;
		ItemName = name;
		Amount = amount;
	}

	virtual const int size() override
	{
		return sizeof(DropItemMessage);
	}
};

struct GiveItemMessage : Message
{
	Game::GUID OtherClientGUID;
	std::string_view ItemName;
	int Amount;

	GiveItemMessage(Game::GUID fromClientGUID, Game::GUID toClientGUID, std::string_view name, int amount)
	{
		Type = Message::MessageType::GiveItem;
		ClientGUID = fromClientGUID;
		OtherClientGUID = toClientGUID;
		ItemName = name;
		Amount = amount;
	}

	virtual const int size() override
	{
		return sizeof(GiveItemMessage);
	}
};

struct RegisterPlayerRequestMessage : Message
{
	RegisterPlayerRequestMessage()
	{
		Type = Message::MessageType::RegisterPlayerRequest;
	}

	virtual const int size() override
	{
		return sizeof(RegisterPlayerRequestMessage);
	}
};

struct RegisterPlayerMessage : Message
{
	char* inventoryBuffer;
	int inventoryBufferSize;

	RegisterPlayerMessage()
	{
		Type = Message::MessageType::RegisterPlayerRequest;
		inventoryBuffer = NULL;
		inventoryBufferSize = 0;
	}

	virtual const int size() override
	{
		return sizeof(Message) + inventoryBufferSize + sizeof(inventoryBufferSize);
	}
};