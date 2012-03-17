#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "Papaya.h"

static const float maximum_tank_vegetation = 0.2f;
int Army::nextPid = 0;

PlatoonController::PlatoonController(Platoon* p)
	: mPlatoon(p)
{
}

DummyPlatoonController::DummyPlatoonController(Platoon* p)
	: PlatoonController(p),
	mAsleep(false)
{
}

bool DummyPlatoonController::control()
{
	bool ret = false;
	if(!mAsleep) {
		mAsleep = true;
		ret = true;
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
				mPlatoon->setPosition(v);
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
	return;
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
				0.0f, 0.0f, MessageType::ClaimArea, MessageData(Area2(0, mTerrain.getWidth(), 0, mTerrain.getWidth()))));
}

EntityID Entity::getEntityID() const
{
	return mEntityID;
}

int Army::getNextPid()
{
	return ++nextPid;
}

Papaya::Papaya(const Terrain& t)
	: mTerrain(t)
{
	float xp1 = 1.0f;
	float yp1 = 1.0f;
	Vector2 base1, base2;
	bool basefound = false;
	for(xp1 = 1.0f; xp1 < mTerrain.getWidth() * 0.25f; xp1 += 1.0f) {
		for(yp1 = 1.0f; yp1 < mTerrain.getWidth() * 0.25f; yp1 += 1.0f) {
			if(mTerrain.getVegetationAt(xp1, yp1) < maximum_tank_vegetation) {
				base1 = Vector2(xp1, yp1);
				basefound = true;
				break;
			}
		}
	}
	if(!basefound) {
		throw std::runtime_error("Could not find a suitable base position for team 1 - too much vegetation.\n");
	}
	basefound = false;
	for(xp1 = t.getWidth() - 1.0f; xp1 > mTerrain.getWidth() * 0.75f; xp1 -= 1.0f) {
		for(yp1 = t.getWidth() - 1.0f; yp1 > mTerrain.getWidth() * 0.75f; yp1 -= 1.0f) {
			if(mTerrain.getVegetationAt(xp1, yp1) < maximum_tank_vegetation) {
				base2 = Vector2(xp1, yp1);
				basefound = true;
				break;
			}
		}
	}
	if(!basefound) {
		throw std::runtime_error("Could not find a suitable base position for team 2 - too much vegetation.\n");
	}
	std::vector<ServiceBranch> armyConfiguration;
	armyConfiguration.push_back(ServiceBranch::Infantry);
	armyConfiguration.push_back(ServiceBranch::Infantry);
	armyConfiguration.push_back(ServiceBranch::Armored);
	armyConfiguration.push_back(ServiceBranch::Artillery);
	armyConfiguration.push_back(ServiceBranch::Engineer);
	armyConfiguration.push_back(ServiceBranch::Recon);
	armyConfiguration.push_back(ServiceBranch::Signal);
	armyConfiguration.push_back(ServiceBranch::Supply);
	mArmies.push_back(std::unique_ptr<Army>(new Army(mTerrain, base1, 1, armyConfiguration)));
	mArmies.push_back(std::unique_ptr<Army>(new Army(mTerrain, base2, 2, armyConfiguration)));
}

void Papaya::process(float dt)
{
	for(auto& a : mArmies) {
		auto pl = a->update();
		for(auto p : pl) {
			for(auto l : mListeners) {
				l->PlatoonStatusChanged(p);
			}
		}
	}
}

const Army* Papaya::getArmy(size_t side) const
{
	if(side >= mArmies.size())
		return nullptr;
	else
		return mArmies[side].get();
}

void Papaya::addEventListener(PapayaEventListener* l)
{
	mListeners.push_back(l);
}

void Papaya::removeEventListener(PapayaEventListener* l)
{
	mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), l), mListeners.end());
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


