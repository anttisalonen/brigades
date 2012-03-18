#include <iostream>
#include "Army.h"
#include "Papaya.h"

int Army::nextPid = 0;

Platoon::Platoon(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side, int pid)
	: MilitaryUnit(b, side),
	mCommandingUnit(commandingunit),
	mPosition(pos),
	mPid(pid),
	mController(nullptr),
	mHealth(100.0f)
{
	mController = std::unique_ptr<PlatoonController>(new PlatoonAIController(this));
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

int Platoon::getPlatoonID() const
{
	return mPid;
}

const Vector2& Platoon::getPosition() const
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

void Platoon::setController(std::unique_ptr<PlatoonController> c)
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

void Platoon::moveTowards(const Vector2& v, float dt)
{
	Vector2 diffvec = v - getPosition();
	Vector2 velvec = diffvec.normalized();
	velvec *= 0.1f * dt * Papaya::instance().getPlatoonSpeed(*this);
	velvec += getPosition();
	setPosition(velvec);
}

const MilitaryUnit* Platoon::getCommandingUnit() const
{
	return mCommandingUnit;
}

MilitaryUnit* Platoon::getCommandingUnit()
{
	return mCommandingUnit;
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

MilitaryUnit::MilitaryUnit(ServiceBranch b, int side)
	: mBranch(b),
	mSide(side),
	mController(nullptr)
{
	mController = std::unique_ptr<MilitaryUnitController>(new SimpleMilitaryUnitController(this));
}

MilitaryUnitController::MilitaryUnitController(MilitaryUnit* m)
	: mUnit(m)
{
}

SimpleMilitaryUnitController::SimpleMilitaryUnitController(MilitaryUnit* m)
	: MilitaryUnitController(m)
{
}

const std::vector<std::unique_ptr<MilitaryUnit>>& MilitaryUnit::getUnits() const
{
	return mUnits;
}

bool SimpleMilitaryUnitController::control(float dt)
{
	return false;
}

void SimpleMilitaryUnitController::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				std::vector<MilitaryUnit*> combatUnits;
				for(auto& u : mUnit->getUnits()) {
					if(isCombatBranch(u->getBranch()))
						combatUnits.push_back(u.get());
				}
				if(combatUnits.size()) {
					float awidth = m.mData->area.x2 - m.mData->area.x1;
					float aheight = m.mData->area.y2 - m.mData->area.y1;
					std::vector<Area2> areas;
					if(awidth > aheight) {
						for(size_t i = 0; i < combatUnits.size(); i++) {
							areas.push_back(Area2(m.mData->area.x1 + awidth * i / combatUnits.size(),
										m.mData->area.y1,
										m.mData->area.x1 + awidth * (i + 1) / combatUnits.size(),
										m.mData->area.y2));
						}
					}
					else {
						for(size_t i = 0; i < combatUnits.size(); i++) {
							areas.push_back(Area2(m.mData->area.x1,
										m.mData->area.y1 + aheight * i / combatUnits.size(),
										m.mData->area.x2,
										m.mData->area.y1 + aheight * (i + 1) / combatUnits.size()));
						}
					}
					for(size_t i = 0; i < combatUnits.size(); i++) {
						MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(),
									combatUnits[i]->getEntityID(),
									0.0f, 0.0f, MessageType::ClaimArea, areas[i]));
					}
				}
			}
			break;
		default:
			std::cout << "Unhandled message " << int(m.mType) << " in SimpleMilitaryUnitController.\n";
			break;
	}
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

Company::Company(const Vector2& pos, ServiceBranch b, int side)
	: MilitaryUnit(b, side)
{
	for(int i = 0; i < 4; i++) {
		mUnits.push_back(std::unique_ptr<Platoon>(new Platoon(this, pos, mBranch, mSide, Army::getNextPid())));
	}
}

Battalion::Battalion(const Vector2& pos, ServiceBranch b, int side)
	: MilitaryUnit(b, side)
{
	for(int i = 0; i < 4; i++) {
		mUnits.push_back(std::unique_ptr<Company>(new Company(pos, mBranch, mSide)));
	}
}

Brigade::Brigade(const Vector2& pos, ServiceBranch b, int side, const std::vector<ServiceBranch>& config)
	: MilitaryUnit(b, side)
{
	for(auto& br : config) {
		mUnits.push_back(std::unique_ptr<Battalion>(new Battalion(pos, br, mSide)));
	}
}

Army::Army(const Terrain& t, const Vector2& base, int side,
		const std::vector<ServiceBranch>& armyConfiguration)
	: MilitaryUnit(ServiceBranch::Infantry, side),
	mTerrain(t),
	mBase(base)
{
	mUnits.push_back(std::unique_ptr<Brigade>(new Brigade(mBase, ServiceBranch::Infantry, mSide, armyConfiguration)));
	MessageDispatcher::instance().dispatchMessage(Message(mEntityID, mUnits[0]->getEntityID(),
				0.0f, 0.0f, MessageType::ClaimArea, MessageData(Area2(0, 0, mTerrain.getWidth(), mTerrain.getWidth()))));
}

EntityID Entity::getEntityID() const
{
	return mEntityID;
}

int Army::getNextPid()
{
	return ++nextPid;
}

const char* branchToName(ServiceBranch b)
{
	switch(b) {
		case ServiceBranch::Infantry:
			return "Infantry";
		case ServiceBranch::Armored:
			return "Armored";
		case ServiceBranch::Artillery:
			return "Artillery";
		case ServiceBranch::Engineer:
			return "Engineer";
		case ServiceBranch::Recon:
			return "Recon";
		case ServiceBranch::Signal:
			return "Signal";
		case ServiceBranch::Supply:
			return "Supply";
	}
	return "";
}

bool isCombatBranch(ServiceBranch b)
{
	switch(b) {
		case ServiceBranch::Infantry:
		case ServiceBranch::Armored:
			return true;
		default:
			return false;
	}
	return false;
}

bool branchOnFoot(ServiceBranch b)
{
	switch(b) {
		case ServiceBranch::Infantry:
		case ServiceBranch::Recon:
			return true;
		default:
			return false;
	}
	return false;
}

