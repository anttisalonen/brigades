#ifndef MESSAGING_H
#define MESSAGING_H

#include <map>
#include <memory>
#include "Terrain.h"

enum class MessageType {
	ClaimArea,
};

typedef int EntityID;

union MessageData {
	MessageData(const Area2& a) : Area(a) { }
	Area2 Area;
};

class Message {
	public:
		Message(EntityID sender, EntityID receiver, float creationTime, float delay,
				MessageType type, const MessageData& data);
		const EntityID mSender;
		const EntityID mReceiver;
		const float mCreationTime;
		const float mSendTime;
		MessageType mType;
		std::unique_ptr<MessageData> mData;
};

class Entity {
	public:
		Entity();
		virtual void receiveMessage(const Message& m) = 0;
		EntityID getEntityID() const;
	protected:
		EntityID mEntityID;
};

class EntityManager {
	public:
		EntityManager();
		static EntityManager& instance();
		EntityID registerEntity(Entity* e);
		Entity* getEntity(EntityID e);
	private:
		EntityID mNextEntityID;
		std::map<EntityID, Entity*> mEntityMap;
};

class MessageDispatcher {
	public:
		MessageDispatcher();
		static MessageDispatcher& instance();
		void dispatchMessage(const Message& m);
};

#endif
