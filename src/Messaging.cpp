#include <iostream>
#include "Messaging.h"

Message::Message(EntityID sender, EntityID receiver, float creationTime, float delay,
		MessageType type, const MessageData& data)
	: mSender(sender),
	mReceiver(receiver),
	mCreationTime(creationTime),
	mSendTime(creationTime + delay),
	mType(type),
	mData(std::unique_ptr<MessageData>(new MessageData(data)))
{
}

Entity::Entity()
{
	mEntityID = EntityManager::instance().registerEntity(this);
}

EntityManager::EntityManager()
	: mNextEntityID(1000)
{
}

static EntityManager singletonEntityManager;

EntityManager& EntityManager::instance()
{
	return singletonEntityManager;
}

EntityID EntityManager::registerEntity(Entity* e)
{
	mEntityMap[mNextEntityID] = e;
	return mNextEntityID++;
}

Entity* EntityManager::getEntity(EntityID e)
{
	auto it = mEntityMap.find(e);
	if(it == mEntityMap.end())
		return nullptr;
	else
		return it->second;
}

MessageDispatcher::MessageDispatcher()
{
}

static MessageDispatcher singletonMessageDispatcher;

MessageDispatcher& MessageDispatcher::instance()
{
	return singletonMessageDispatcher;
}

void MessageDispatcher::registerWorldEntity(WorldEntity* e)
{
	mWorldEntities.push_back(e);
}

void MessageDispatcher::dispatchMessage(const Message& m)
{
	if(m.mReceiver == WORLD_ENTITY_ID) {
		for(auto l : mWorldEntities) {
			l->receiveMessage(m);
		}
	}
	else {
		Entity* e = EntityManager::instance().getEntity(m.mReceiver);
		if(e) {
			e->receiveMessage(m);
		}
		else {
			std::cerr << "Message to " << m.mReceiver << " could not be delivered.\n";
		}
	}
}

