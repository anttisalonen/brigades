#include <string.h>

#include "Steering.h"
#include "MilitaryUnit.h"

Steering::Steering(Platoon* p)
	: mPlatoon(p)
{
	clear();
}

void Steering::clear()
{
	memset(mSteerings, 0x00, sizeof(mSteerings));
}

Vector2 Steering::steer()
{
	if(mSteerings[0] == SteeringType::Seek) {
		return mSeekTarget - mPlatoon->getPosition();
	}
	else {
		return Vector2();
	}
}

void Steering::setSeek(const Vector2& tgt)
{
	mSteerings[0] = SteeringType::Seek;
	mSeekTarget = tgt;
}


