#include <iostream>

#include "Papaya.h"
#include "PlatoonAI.h"
#include "Army.h"

PlatoonController::PlatoonController(Platoon* p)
	: mPlatoon(p)
{
}

PlatoonAIController::PlatoonAIController(Platoon* p)
	: PlatoonController(p)
{
	mControllerStack.push(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mPlatoon, this)));
}

bool PlatoonAIController::control(float dt)
{
	if(mControllerStack.empty())
		mControllerStack.push(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mPlatoon, this)));
	return mControllerStack.top()->control(dt);
}

void PlatoonAIController::receiveMessage(const Message& m)
{
	if(mControllerStack.empty())
		mControllerStack.push(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mPlatoon, this)));
	mControllerStack.top()->receiveMessage(m);
}

void PlatoonAIController::pushController(std::unique_ptr<PlatoonAIState> c)
{
	mControllerStack.push(std::move(c));
}

void PlatoonAIController::popController()
{
	mControllerStack.pop();
}

PlatoonAIState::PlatoonAIState(Platoon* p, PlatoonAIController* c)
	: PlatoonController(p),
	mAIController(c)
{
}

PlatoonAIDefendState::PlatoonAIDefendState(Platoon* p, PlatoonAIController* c)
	: PlatoonAIState(p, c),
	mAsleep(false)
{
}

bool PlatoonAIDefendState::control(float dt)
{
	bool ret = false;
	if(!mAsleep) {
		mAsleep = true;
		ret = true;
	}
	return ret;
}

void PlatoonAIDefendState::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				Vector2 v((m.mData->area.x2 + m.mData->area.x1) / 2.0f,
						(m.mData->area.y2 + m.mData->area.y1) / 2.0f);
				mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAIMoveState(mPlatoon, mAIController, v)));
			}
			break;
		case MessageType::EnemyDiscovered:
			std::cout << "Discovered enemy while defending.\n";
			mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAICombatState(mPlatoon, mAIController, m.mData->platoon)));
			break;
		default:
			std::cout << "Unhandled message " << int(m.mType) << " in PlatoonAIDefendState.\n";
			break;
	}
}

PlatoonAIMoveState::PlatoonAIMoveState(Platoon* p, PlatoonAIController* c, const Vector2& t)
	: PlatoonAIState(p, c),
	mTargetPos(t)
{
}

bool PlatoonAIMoveState::control(float dt)
{
	Vector2 diffvec = mTargetPos - mPlatoon->getPosition();
	if(diffvec.length() > 0.5) {
		mPlatoon->moveTowards(mTargetPos, dt);
	}
	else {
		mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mPlatoon, mAIController)));
		MessageDispatcher::instance().dispatchMessage(Message(mPlatoon->getEntityID(), mPlatoon->getCommandingUnit()->getEntityID(),
					0.0f, 0.0f, MessageType::ReachedPosition, MessageData()));
	}
	return true;
}

void PlatoonAIMoveState::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::EnemyDiscovered:
			std::cout << "Discovered enemy while moving.\n";
			mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAICombatState(mPlatoon, mAIController, m.mData->platoon)));
			break;
		default:
			std::cout << "Unhandled message " << int(m.mType) << " in PlatoonAIMoveState.\n";
			break;
	}
}

PlatoonAICombatState::PlatoonAICombatState(Platoon* p, PlatoonAIController* c, Platoon* ep)
	: PlatoonAIState(p, c),
	mEnemyPlatoon(ep)
{
}

bool PlatoonAICombatState::control(float dt)
{
	Vector2 targetPos = mEnemyPlatoon->getPosition();
	Vector2 diffvec = targetPos - mPlatoon->getPosition();
	if(diffvec.length() > 1.0f) {
		mPlatoon->moveTowards(targetPos, dt);
	}
	else {
		float damage = dt * (rand() % 100) * 0.01f;
		mEnemyPlatoon->loseHealth(damage);
		if(mEnemyPlatoon->isDead()) {
			mAIController->popController();
		}
	}
	return true;
}

void PlatoonAICombatState::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::EnemyDiscovered:
			// already in combat
			break;
		default:
			std::cout << "Unhandled message " << int(m.mType) << " in PlatoonAICombatState.\n";
			break;
	}
}

