#ifndef MESSAGING_H
#define MESSAGING_H

#include <map>

enum class MessageType {
	UnitSpawned,
};

typedef int EntityID;

class Message {
	public:
		Message(EntityID sender, EntityID receiver, float creationTime, float delay,
				MessageType type, void* data);
		const EntityID mSender;
		const EntityID mReceiver;
		const float mCreationTime;
		const float mSendTime;
		const MessageType mType;
		void* mData;
};

class Entity {
	public:
		virtual void receiveMessage(const Message& m) = 0;
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
