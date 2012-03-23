#ifndef MILITARYUNITAI_H
#define MILITARYUNITAI_H

#include "MilitaryUnit.h"
#include "Messaging.h"

class MilitaryUnitAIController : public Controller<MilitaryUnit> {
	public:
		MilitaryUnitAIController(MilitaryUnit* m);
		virtual void receiveMessage(const Message& m);
		virtual bool control(float dt);
};

#endif
