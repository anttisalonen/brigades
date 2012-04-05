#ifndef STEERING_H
#define STEERING_H

#include "Terrain.h"

class Platoon;

enum class SteeringType {
	None,
	Seek,
};

class Steering {
	public:
		Steering(Platoon* p);
		void clear();
		Vector2 steer();
		void setSeek(const Vector2& tgt);
	private:
		Platoon* mPlatoon;
		SteeringType mSteerings[10];
		Vector2 mSeekTarget;
};

#endif
