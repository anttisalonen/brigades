#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "Papaya.h"

static const float maximum_tank_vegetation = 0.2f;

Papaya::Papaya()
{
}

void Papaya::setup(const Terrain* t)
{
	mTerrain = t;
	float xp1 = 1.0f;
	float yp1 = 1.0f;
	Vector2 base1, base2;
	bool basefound = false;
	for(xp1 = 1.0f; xp1 < mTerrain->getWidth() * 0.25f; xp1 += 1.0f) {
		for(yp1 = 1.0f; yp1 < mTerrain->getWidth() * 0.25f; yp1 += 1.0f) {
			if(mTerrain->getVegetationAt(Vector2(xp1, yp1)) < maximum_tank_vegetation) {
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
	for(xp1 = t->getWidth() - 1.0f; xp1 > mTerrain->getWidth() * 0.75f; xp1 -= 1.0f) {
		for(yp1 = t->getWidth() - 1.0f; yp1 > mTerrain->getWidth() * 0.75f; yp1 -= 1.0f) {
			if(mTerrain->getVegetationAt(Vector2(xp1, yp1)) < maximum_tank_vegetation) {
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
	mArmies.push_back(std::unique_ptr<Army>(new Army(*mTerrain, base1, 1, armyConfiguration)));
	mArmies.push_back(std::unique_ptr<Army>(new Army(*mTerrain, base2, 2, armyConfiguration)));
}

void Papaya::process(float dt)
{
	for(auto& a : mArmies) {
		auto pl = a->update(dt);
		for(auto p : pl) {
			for(auto l : mListeners) {
				l->PlatoonStatusChanged(p);
			}
		}
	}
}

const Army* Papaya::getArmy(size_t side) const
{
	return getArmy(side);
}

Army* Papaya::getArmy(size_t side)
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

static Papaya singletonPapaya;

Papaya& Papaya::instance()
{
	return singletonPapaya;
}

float Papaya::getPlatoonSpeed(const Platoon& p) const
{
	float base = 1.0f;
	float terraincoeff = mTerrain->getVegetationAt(p.getPosition());
	if(branchOnFoot(p.getBranch())) {
		base *= 5.0f * (1.0f - terraincoeff);
	}
	else {
		base *= 1.0f * (1.0f - (terraincoeff / 2.0f));
	}
	return base;
}

