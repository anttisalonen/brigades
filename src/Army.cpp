#include <iostream>
#include "Army.h"

int Army::nextPid = 0;

PlatoonController::PlatoonController(Platoon* p)
	: mPlatoon(p)
{
}

DummyPlatoonController::DummyPlatoonController(Platoon* p)
	: PlatoonController(p),
	mAsleep(false),
	mTargetPos(p->getPosition())
{
}

bool DummyPlatoonController::control()
{
	bool ret = false;
	if(!mAsleep) {
		mAsleep = true;
		ret = true;
	}
	else {
		Vector2 diffvec = mTargetPos - mPlatoon->getPosition();
		if(diffvec.length() > 0.5) {
			Vector2 velvec = diffvec.normalized();
			velvec *= 0.1f;
			velvec += mPlatoon->getPosition();
			mPlatoon->setPosition(velvec);
			ret = true;
		}
	}
	return ret;
}

Platoon::Platoon(const Vector2& pos, ServiceBranch b, int side, int pid)
	: MilitaryUnit(b, side),
	mPosition(pos),
	mPid(pid),
	mController(nullptr)
{
	mController = std::unique_ptr<PlatoonController>(new DummyPlatoonController(this));
}

ServiceBranch Platoon::getBranch() const
{
	return mBranch;
}

std::list<Platoon*> Platoon::update()
{
	if(mController->control())
		return std::list<Platoon*>(1, this);
	else
		return std::list<Platoon*>();
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

void DummyPlatoonController::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				Vector2 v((m.mData->Area.x2 + m.mData->Area.x1) / 2.0f,
						(m.mData->Area.y2 + m.mData->Area.y1) / 2.0f);
				std::cout << "I must go to " << v << ".\n";
				mTargetPos = v;
			}
			break;
	}
}

MilitaryUnit::MilitaryUnit(ServiceBranch b, int side)
	: mBranch(b),
	mSide(side)
{
}

void MilitaryUnit::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				std::vector<MilitaryUnit*> combatUnits;
				for(auto& u : mUnits) {
					if(isCombatBranch(u->getBranch()))
						combatUnits.push_back(u.get());
				}
				if(combatUnits.size()) {
					float awidth = m.mData->Area.x2 - m.mData->Area.x1;
					float aheight = m.mData->Area.y2 - m.mData->Area.y1;
					std::vector<Area2> areas;
					if(awidth > aheight) {
						for(size_t i = 0; i < combatUnits.size(); i++) {
							areas.push_back(Area2(m.mData->Area.x1 + awidth * i / combatUnits.size(),
										m.mData->Area.y1,
										m.mData->Area.x1 + awidth * (i + 1) / combatUnits.size(),
										m.mData->Area.y2));
						}
					}
					else {
						for(size_t i = 0; i < combatUnits.size(); i++) {
							areas.push_back(Area2(m.mData->Area.x1,
										m.mData->Area.y1 + aheight * i / combatUnits.size(),
										m.mData->Area.x2,
										m.mData->Area.y1 + aheight * (i + 1) / combatUnits.size()));
						}
					}
					for(size_t i = 0; i < combatUnits.size(); i++) {
						MessageDispatcher::instance().dispatchMessage(Message(mEntityID, combatUnits[i]->getEntityID(),
									0.0f, 0.0f, MessageType::ClaimArea, areas[i]));
					}
				}
			}
			break;
	}
}

ServiceBranch MilitaryUnit::getBranch() const
{
	return mBranch;
}

int MilitaryUnit::getSide() const
{
	return mSide;
}

std::list<Platoon*> MilitaryUnit::update()
{
	std::list<Platoon*> units;
	for(auto& u : mUnits) {
		units.splice(units.end(), u->update());
	}
	return units;
}

Company::Company(const Vector2& pos, ServiceBranch b, int side)
	: MilitaryUnit(b, side)
{
	for(int i = 0; i < 4; i++) {
		mUnits.push_back(std::unique_ptr<Platoon>(new Platoon(pos, mBranch, mSide, Army::getNextPid())));
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

const char* BranchToName(ServiceBranch b)
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
