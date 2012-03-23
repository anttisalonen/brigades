#include <vector>
#include <iostream>
#include "MilitaryUnitAI.h"

SimpleMilitaryUnitController::SimpleMilitaryUnitController(MilitaryUnit* m)
	: Controller<MilitaryUnit>(m)
{
}

bool SimpleMilitaryUnitController::control(float dt)
{
	return false;
}

void SimpleMilitaryUnitController::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				std::vector<MilitaryUnit*> combatUnits;
				for(auto& u : mUnit->getUnits()) {
					if(isCombatBranch(u->getBranch()))
						combatUnits.push_back(u.get());
				}
				if(combatUnits.size()) {
					float awidth = m.mData->area.x2 - m.mData->area.x1;
					float aheight = m.mData->area.y2 - m.mData->area.y1;
					std::vector<Area2> areas;
					if(awidth > aheight) {
						for(size_t i = 0; i < combatUnits.size(); i++) {
							areas.push_back(Area2(m.mData->area.x1 + awidth * i / combatUnits.size(),
										m.mData->area.y1,
										m.mData->area.x1 + awidth * (i + 1) / combatUnits.size(),
										m.mData->area.y2));
						}
					}
					else {
						for(size_t i = 0; i < combatUnits.size(); i++) {
							areas.push_back(Area2(m.mData->area.x1,
										m.mData->area.y1 + aheight * i / combatUnits.size(),
										m.mData->area.x2,
										m.mData->area.y1 + aheight * (i + 1) / combatUnits.size()));
						}
					}
					for(size_t i = 0; i < combatUnits.size(); i++) {
						MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(),
									combatUnits[i]->getEntityID(),
									0.0f, 0.0f, MessageType::ClaimArea, areas[i]));
					}
				}
			}
			break;
		case MessageType::EnemyDiscovered:
			{
			}
			break;
		default:
			std::cout << "Unhandled message " << int(m.mType) << " in SimpleMilitaryUnitController.\n";
			break;
	}
}

