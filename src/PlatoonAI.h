#ifndef PLATOONAI_H
#define PLATOONAI_H

#include <memory>
#include <stack>

#include "Messaging.h"
#include "Terrain.h"

class Platoon;

class PlatoonController {
	public:
		PlatoonController(Platoon* p);
		virtual bool control(float dt) = 0;
		virtual void receiveMessage(const Message& m) = 0;
	protected:
		Platoon* mPlatoon;
};

class PlatoonAIState;

class PlatoonAIController : public PlatoonController {
	public:
		PlatoonAIController(Platoon* p);
		bool control(float dt);
		void receiveMessage(const Message& m);
		void pushController(std::unique_ptr<PlatoonAIState> c);
		void popController();
	protected:
		std::stack<std::unique_ptr<PlatoonAIState>> mControllerStack;
};

class PlatoonAIState : public PlatoonController {
	public:
		PlatoonAIState(Platoon* p, PlatoonAIController* c);
	protected:
		PlatoonAIController* mAIController;
};

class PlatoonAIDefendState : public PlatoonAIState {
	public:
		PlatoonAIDefendState(Platoon* p, PlatoonAIController* c);
		virtual bool control(float dt);
		virtual void receiveMessage(const Message& m);
	protected:
		bool mAsleep;
};

class PlatoonAIMoveState : public PlatoonAIState {
	public:
		PlatoonAIMoveState(Platoon* p, PlatoonAIController* c, const Vector2& t);
		virtual bool control(float dt);
		virtual void receiveMessage(const Message& m);
	protected:
		Vector2 mTargetPos;
};

class PlatoonAICombatState : public PlatoonAIState {
	public:
		PlatoonAICombatState(Platoon* p, PlatoonAIController* c, Platoon* ep);
		virtual bool control(float dt);
		virtual void receiveMessage(const Message& m);
	protected:
		Platoon* mEnemyPlatoon;
};

#endif

