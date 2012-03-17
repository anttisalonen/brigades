#include <iostream>
#include "Messaging.h"

Message::Message(EntityID sender, EntityID receiver, float creationTime, float delay,
		MessageType type, void* data)
	: mSender(sender),
	mReceiver(receiver),
	mCreationTime(creationTime),
	mSendTime(creationTime + delay),
	mType(type),
	mData(data)
{
}

EntityManager::EntityManager()
	: mNextEntityID(1)
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

void MessageDispatcher::dispatchMessage(const Message& m)
{
	Entity* e = EntityManager::instance().getEntity(m.mReceiver);
	if(e) {
		e->receiveMessage(m);
	}
	else {
		std::cerr << "Message to " << m.mReceiver << " could not be delivered.\n";
	}
}

