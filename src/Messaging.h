#ifndef MESSAGING_H
#define MESSAGING_H

#include <map>
#include <memory>
#include <vector>
#include "Terrain.h"

enum class MessageType {
	ClaimArea,
	EnemyDiscovered,
	ReachedPosition,
	PlatoonDied,
};

typedef int EntityID;

const EntityID WORLD_ENTITY_ID = 1;

class Platoon;

union MessageData {
	MessageData(const Area2& a) : area(a) { }
	MessageData(Platoon* p) : platoon(p) { }
	MessageData() { }
	Area2 area;
	Platoon* platoon;
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

class WorldEntity {
	public:
		virtual void receiveMessage(const Message& m) = 0;
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
		void registerWorldEntity(WorldEntity* e);
	private:
		std::vector<WorldEntity*> mWorldEntities;
};

#endif
