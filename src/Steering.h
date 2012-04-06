#ifndef STEERING_H
#define STEERING_H

#include <set>

#include "Terrain.h"

#define MAX_STEERINGS 10

class Platoon;

enum class SteeringType {
	None,
	Seek,
	Separation,
};

class Steering {
	public:
		Steering(Platoon* p);
		void clear();
		Vector2 steer();
		void setSeek(const Vector2& tgt);
		void setSeparation();
	private:
		void addSteering(enum SteeringType t);
		bool seek(Vector2& v) const;
		bool separate(Vector2& v) const;
		bool accumulateSteering(Vector2& accumulated, const Vector2& toAdd) const;
		Platoon* mPlatoon;
		SteeringType mSteerings[MAX_STEERINGS];
		std::set<SteeringType> mSteeringsActivated;
		Vector2 mSeekTarget;
};

#endif
