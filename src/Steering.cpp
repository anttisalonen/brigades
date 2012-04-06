#include <string.h>
#include <iostream>

#include "Steering.h"
#include "Papaya.h"
#include "MilitaryUnit.h"

Steering::Steering(Platoon* p)
	: mPlatoon(p)
{
	clear();
}

void Steering::clear()
{
	memset(mSteerings, 0x00, sizeof(mSteerings));
	mSteeringsActivated.clear();
}

Vector2 Steering::steer()
{
	Vector2 v;
	for(int i = 0; i < MAX_STEERINGS; i++) {
		switch(mSteerings[i]) {
			case SteeringType::None:
				return v;
			case SteeringType::Seek:
				if(seek(v))
					return v;
				break;
			case SteeringType::Separation:
				if(separate(v))
					return v;
				break;
		}
	}
	return v;
}

bool Steering::seek(Vector2& v) const
{
#ifdef STEERING_DEBUG
	if(mPlatoon->getEntityID() == 1006)
		std::cout << "Seeking ";
#endif
	return accumulateSteering(v, mSeekTarget - mPlatoon->getPosition());
}

bool Steering::separate(Vector2& v) const
{
	static const float maxSeparationDistance = 2.0f;
	for(Platoon* p = Papaya::instance().getNeighbouringPlatoons(mPlatoon, maxSeparationDistance);
			p != nullptr;
			p = Papaya::instance().getNextNeighbouringPlatoon()) {
		if(p->getSide() == mPlatoon->getSide() && !p->isDead()) {
			Vector2 diff = mPlatoon->getPosition() - p->getPosition();
			float vl = diff.length() / maxSeparationDistance;
#ifdef STEERING_DEBUG
			if(mPlatoon->getEntityID() == 1006)
				std::cout << "Separate " << vl;
#endif
			if(accumulateSteering(v, diff * (1.0f - vl)))
				return true;
		}
	}
	return false;
}

bool Steering::accumulateSteering(Vector2& accumulated, const Vector2& toAdd) const
{
	float acclen = accumulated.length();
	if(acclen >= 1.0f)
		return true;
	float leftlen = 1.0f - acclen;
	float addlen = toAdd.length();
#ifdef STEERING_DEBUG
	if(mPlatoon->getEntityID() == 1006)
		std::cout << toAdd << " => ";
#endif
	if(leftlen < addlen) {
		accumulated += toAdd.normalized() * leftlen;
#ifdef STEERING_DEBUG
		if(mPlatoon->getEntityID() == 1006)
			std::cout << accumulated << " done.\n";
#endif
		return true;
	}
	else {
		accumulated += toAdd;
#ifdef STEERING_DEBUG
		if(mPlatoon->getEntityID() == 1006)
			std::cout << accumulated << ".\n";
#endif
		return false;
	}
}

void Steering::setSeek(const Vector2& tgt)
{
	addSteering(SteeringType::Seek);
	mSeekTarget = tgt;
}

void Steering::setSeparation()
{
	addSteering(SteeringType::Separation);
}

void Steering::addSteering(enum SteeringType t)
{
	if(mSteeringsActivated.find(t) != mSteeringsActivated.end())
		return;
	for(int i = 0; i < MAX_STEERINGS; i++) {
		if(mSteerings[i] == SteeringType::None) {
			mSteerings[i] = t;
			mSteeringsActivated.insert(t);
			return;
		}
	}
	std::cout << "Warning: Not enough steering slots. Increase MAX_STEERINGS.\n";
}

