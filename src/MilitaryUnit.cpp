#include <memory>
#include <list>
#include <iostream>

#include "MilitaryUnit.h"
#include "MilitaryUnitAI.h"
#include "PlatoonAI.h"
#include "Army.h"
#include "Papaya.h"

Platoon::Platoon(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side)
	: MilitaryUnit(commandingunit, b, side),
	mPosition(pos),
	mController(nullptr),
	mHealth(100.0f)
{
	mController = std::unique_ptr<Controller<Platoon>>(new PlatoonAIController(this));
}

ServiceBranch Platoon::getBranch() const
{
	return mBranch;
}

std::list<Platoon*> Platoon::update(float dt)
{
	if(isDead()) {
		return std::list<Platoon*>();
	}
	checkVisibility();
	if(mController->control(dt))
		return std::list<Platoon*>(1, this);
	else
		return std::list<Platoon*>();
}

std::list<Platoon*> Platoon::getPlatoons()
{
	return std::list<Platoon*>(1, this);
}

int Platoon::getSide() const
{
	return mSide;
}

Vector2 Platoon::getPosition() const
{
	return mPosition;
}

void Platoon::setPosition(const Vector2& v)
{
	mPosition = v;
}

void Platoon::receiveMessage(const Message& m)
{
	mController->receiveMessage(m);
}

void Platoon::setController(std::unique_ptr<Controller<Platoon>> c)
{
	mController.swap(c);
}

void Platoon::checkVisibility()
{
	size_t enemyside = getSide() == 1 ? 1 : 0;
	Army* enemyarmy = Papaya::instance().getArmy(enemyside);
	if(enemyarmy) {
		std::list<Platoon*> enemyplatoons = enemyarmy->getPlatoons();
		for(auto ep : enemyplatoons) {
			if(!ep->isDead()) {
				if((getPosition() - ep->getPosition()).length() < 4.0f) {
					MessageDispatcher::instance().dispatchMessage(Message(mEntityID, mEntityID,
								0.0f, 0.0f, MessageType::EnemyDiscovered, ep));
				}
			}
		}
	}
	else {
		std::cerr << "No enemy army found?\n";
	}
}

UnitSize Platoon::getUnitSize() const
{
	return UnitSize::Platoon;
}

UnitSize Company::getUnitSize() const
{
	return UnitSize::Company;
}

UnitSize Battalion::getUnitSize() const
{
	return UnitSize::Battalion;
}

UnitSize Brigade::getUnitSize() const
{
	return UnitSize::Brigade;
}

UnitSize Army::getUnitSize() const
{
	return UnitSize::Division;
}

void Platoon::moveTowards(const Vector2& v, float dt)
{
	Vector2 diffvec = v - getPosition();
	Vector2 velvec = diffvec.normalized();
	velvec *= 0.1f * dt * Papaya::instance().getPlatoonSpeed(*this);
	velvec += getPosition();
	setPosition(velvec);
}

void Platoon::loseHealth(float damage)
{
	bool wasdead = isDead();
	mHealth -= damage;
	if(!wasdead && isDead()) {
		MessageDispatcher::instance().dispatchMessage(Message(mEntityID, WORLD_ENTITY_ID,
					0.0f, 0.0f, MessageType::PlatoonDied, this));
	}
}

bool Platoon::isDead() const
{
	return mHealth <= 0.0f;
}

float Platoon::getHealth() const
{
	return mHealth;
}

MilitaryUnit::MilitaryUnit(MilitaryUnit* commandingunit, ServiceBranch b, int side)
	: mCommandingUnit(commandingunit),
	mBranch(b),
	mSide(side),
	mController(nullptr)
{
	mController = std::unique_ptr<Controller<MilitaryUnit>>(new MilitaryUnitAIController(this));
}

const MilitaryUnit* MilitaryUnit::getCommandingUnit() const
{
	return mCommandingUnit;
}

MilitaryUnit* MilitaryUnit::getCommandingUnit()
{
	return mCommandingUnit;
}

const std::vector<std::unique_ptr<MilitaryUnit>>& MilitaryUnit::getUnits() const
{
	return mUnits;
}

void MilitaryUnit::receiveMessage(const Message& m)
{
	mController->receiveMessage(m);
}

ServiceBranch MilitaryUnit::getBranch() const
{
	return mBranch;
}

int MilitaryUnit::getSide() const
{
	return mSide;
}

std::list<Platoon*> MilitaryUnit::getPlatoons()
{
	std::list<Platoon*> units;
	for(auto& u : mUnits) {
		units.splice(units.end(), u->getPlatoons());
	}
	return units;
}

std::list<Platoon*> MilitaryUnit::update(float dt)
{
	std::list<Platoon*> units;
	for(auto& u : mUnits) {
		units.splice(units.end(), u->update(dt));
	}
	return units;
}

Vector2 MilitaryUnit::getPosition() const
{
	Vector2 pos;
	for(auto& u : mUnits) {
		pos += u->getPosition();
	}
	if(mUnits.size())
		pos *= 1.0f / mUnits.size();
	return pos;
}

float MilitaryUnit::distanceTo(const MilitaryUnit& m) const
{
	Vector2 diff = getPosition() - m.getPosition();
	return diff.length();
}

Company::Company(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side)
	: MilitaryUnit(commandingunit, b, side)
{
	for(int i = 0; i < 4; i++) {
		mUnits.push_back(std::unique_ptr<Platoon>(new Platoon(this, pos, mBranch, mSide)));
	}
}

Battalion::Battalion(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side)
	: MilitaryUnit(commandingunit, b, side)
{
	for(int i = 0; i < 4; i++) {
		mUnits.push_back(std::unique_ptr<Company>(new Company(this, pos, mBranch, mSide)));
	}
}

Brigade::Brigade(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side, const std::vector<ServiceBranch>& config)
	: MilitaryUnit(commandingunit, b, side)
{
	for(auto& br : config) {
		mUnits.push_back(std::unique_ptr<Battalion>(new Battalion(this, pos, br, mSide)));
	}
}


