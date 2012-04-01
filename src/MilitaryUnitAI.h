#ifndef MILITARYUNITAI_H
#define MILITARYUNITAI_H

#include <vector>
#include "MilitaryUnit.h"
#include "Messaging.h"

class MilitaryUnitAIController : public Controller<MilitaryUnit> {
	public:
		MilitaryUnitAIController(MilitaryUnit* m);
		virtual void receiveMessage(const Message& m);
		virtual bool control(float dt);
	protected:
		std::vector<std::shared_ptr<MilitaryUnit>> getCombatUnits() const;
		void attackPlatoon(Platoon* p);
		bool mInCombat;
};

#endif
