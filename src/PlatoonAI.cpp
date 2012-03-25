#include <iostream>

#include "Papaya.h"
#include "PlatoonAI.h"
#include "Army.h"

PlatoonAIController::PlatoonAIController(Platoon* p)
	: Controller<Platoon>(p)
{
	mControllerStack.push(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mUnit, this)));
}

bool PlatoonAIController::control(float dt)
{
	if(mControllerStack.empty())
		mControllerStack.push(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mUnit, this)));
	return mControllerStack.top()->control(dt);
}

void PlatoonAIController::receiveMessage(const Message& m)
{
	if(mControllerStack.empty())
		mControllerStack.push(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mUnit, this)));
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
	: Controller<Platoon>(p),
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
				mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAIMoveState(mUnit, mAIController, v)));
			}
			break;

		case MessageType::EnemyDiscovered:
			mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAICombatState(mUnit, mAIController, m.mData->platoon)));
			MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(), mUnit->getCommandingUnit()->getEntityID(),
						0.0f, 0.0f, MessageType::EnemyDiscovered, m.mData->platoon));
			break;

		case MessageType::AttackEnemy:
			mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAICombatState(mUnit, mAIController, m.mData->platoon)));
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
	Vector2 diffvec = mTargetPos - mUnit->getPosition();
	if(diffvec.length() > 0.5) {
		mUnit->moveTowards(mTargetPos, dt);
	}
	else {
		mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAIDefendState(mUnit, mAIController)));
		MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(), mUnit->getCommandingUnit()->getEntityID(),
					0.0f, 0.0f, MessageType::ReachedPosition, MessageData()));
	}
	return true;
}

void PlatoonAIMoveState::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				Vector2 v((m.mData->area.x2 + m.mData->area.x1) / 2.0f,
						(m.mData->area.y2 + m.mData->area.y1) / 2.0f);
				mTargetPos = v;
			}
			break;

		case MessageType::EnemyDiscovered:
			mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAICombatState(mUnit, mAIController, m.mData->platoon)));
			MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(), mUnit->getCommandingUnit()->getEntityID(),
						0.0f, 0.0f, MessageType::EnemyDiscovered, m.mData->platoon));
			break;

		case MessageType::AttackEnemy:
			mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAICombatState(mUnit, mAIController, m.mData->platoon)));
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
	Vector2 diffvec = targetPos - mUnit->getPosition();
	if(diffvec.length() > 1.0f) {
		mUnit->moveTowards(targetPos, dt);
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
			if(mUnit->distanceTo(*m.mData->platoon) < mUnit->distanceTo(*mEnemyPlatoon))
				mEnemyPlatoon = m.mData->platoon;
			MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(), mUnit->getCommandingUnit()->getEntityID(),
						0.0f, 0.0f, MessageType::EnemyDiscovered, m.mData->platoon));
			break;

		case MessageType::AttackEnemy:
			// already in combat
			break;

		default:
			std::cout << "Unhandled message " << int(m.mType) << " in PlatoonAICombatState.\n";
			break;
	}
}

