#ifndef MILITARYUNITAI_H
#define MILITARYUNITAI_H

#include "MilitaryUnit.h"
#include "Messaging.h"

class SimpleMilitaryUnitController : public MilitaryUnitController {
	public:
		SimpleMilitaryUnitController(MilitaryUnit* m);
		virtual void receiveMessage(const Message& m);
		virtual bool control(float dt);
};

#endif
