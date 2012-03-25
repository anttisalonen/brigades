#include <vector>
#include <iostream>
#include "MilitaryUnitAI.h"

MilitaryUnitAIController::MilitaryUnitAIController(MilitaryUnit* m)
	: Controller<MilitaryUnit>(m)
{
}

bool MilitaryUnitAIController::control(float dt)
{
	return false;
}

void MilitaryUnitAIController::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				std::vector<MilitaryUnit*> combatUnits = getCombatUnits();
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
			break;

		case MessageType::EnemyDiscovered:
			attackPlatoon(m.mData->platoon);
			if(mUnit->getCommandingUnit())
				MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(), mUnit->getCommandingUnit()->getEntityID(),
							0.0f, 0.0f, MessageType::EnemyDiscovered, m.mData->platoon));
			break;

		case MessageType::AttackEnemy:
			attackPlatoon(m.mData->platoon);
			break;

		case MessageType::ReachedPosition:
			// Nothing to do, I guess.
			break;

		default:
			std::cout << "Unhandled message " << int(m.mType) << " in MilitaryUnitAIController.\n";
			break;
	}
}

std::vector<MilitaryUnit*> MilitaryUnitAIController::getCombatUnits() const
{
	std::vector<MilitaryUnit*> combatUnits;
	for(auto& u : mUnit->getUnits()) {
		if(isCombatBranch(u->getBranch()))
			combatUnits.push_back(u.get());
	}
	return combatUnits;
}

void MilitaryUnitAIController::attackPlatoon(Platoon* p)
{
	for(auto& u : getCombatUnits()) {
		MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(),
					u->getEntityID(),
					0.0f, 0.0f, MessageType::AttackEnemy, p));
	}
}

