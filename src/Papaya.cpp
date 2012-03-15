#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "Papaya.h"

static const float maximum_tank_vegetation = 0.2f;
int Army::nextPid = 0;

Platoon::Platoon(const Vector2& pos, int side, int pid)
	: mPosition(pos),
	mSide(side),
	mPid(pid),
	mAsleep(false)
{
}

void Platoon::update()
{
}

bool Platoon::asleep() const
{
	return mAsleep;
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

Army::Army(const Terrain& t, const Vector2& base, int side)
	: mTerrain(t),
	mBase(base),
	mSide(side)
{
}

std::vector<const Platoon*> Army::getPlatoons() const
{
	return getPlatoons();
}

std::vector<Platoon*> Army::getPlatoons()
{
	std::vector<Platoon*> ret;
	for(auto& p : mPlatoons) {
		ret.push_back(p.get());
	}
	return ret;
}

void Army::addPlatoon()
{
	Vector2 pos(mBase);
	pos.x += mPlatoons.size() * 0.5f;
	mPlatoons.push_back(std::unique_ptr<Platoon>(new Platoon(pos, mSide, getNextPid())));
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
	mArmies.push_back(std::unique_ptr<Army>(new Army(mTerrain, base1, 1)));
	mArmies.push_back(std::unique_ptr<Army>(new Army(mTerrain, base2, 2)));
	for(auto& a : mArmies) {
		for(int i = 0; i < 4; i++) {
			a->addPlatoon();
		}
	}
}

void Papaya::process(float dt)
{
	for(auto& a : mArmies) {
		for(auto& p : a->getPlatoons()) {
			p->update();
			if(!p->asleep()) {
				for(auto& l : mListeners) {
					l->PlatoonStatusChanged(p);
				}
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

