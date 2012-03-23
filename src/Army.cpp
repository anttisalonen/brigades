#include <iostream>
#include "Army.h"
#include "Papaya.h"

int Army::nextPid = 0;

Army::Army(const Terrain& t, const Vector2& base, int side,
		const std::vector<ServiceBranch>& armyConfiguration)
	: MilitaryUnit(nullptr, ServiceBranch::Infantry, side),
	mTerrain(t),
	mBase(base)
{
	mUnits.push_back(std::unique_ptr<Brigade>(new Brigade(this, mBase, ServiceBranch::Infantry, mSide, armyConfiguration)));
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

const char* unitSizeToName(UnitSize s)
{
	switch(s) {
		case UnitSize::Single:
			return "Single";
		case UnitSize::Squad:
			return "Squad";
		case UnitSize::Platoon:
			return "Platoon";
		case UnitSize::Company:
			return "Company";
		case UnitSize::Battalion:
			return "Battalion";
		case UnitSize::Brigade:
			return "Brigade";
		case UnitSize::Division:
			return "Division";
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

