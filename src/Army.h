#ifndef ARMY_H
#define ARMY_H

#include <list>
#include <vector>

#include "Messaging.h"
#include "PlatoonAI.h"
#include "MilitaryUnit.h"
#include "Terrain.h"

class Army : public MilitaryUnit {
	public:
		Army(const Terrain& t, const Vector2& base, int side,
				const std::vector<ServiceBranch>& armyConfiguration);
		UnitSize getUnitSize() const;
		virtual std::list<Platoon*> update(float dt);
	private:
		const Terrain& mTerrain;
		Vector2 mBase;
		bool mSentAttackMessage;
};

#endif

