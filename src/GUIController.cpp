#include "GUIController.h"

GUIController::GUIController(App* app, MilitaryUnit* p)
	: Controller<MilitaryUnit>(p),
	mApp(app)
{
}

GUIController::~GUIController()
{
}

bool GUIController::control(float dt)
{
	return true;
}

void GUIController::receiveMessage(const Message& m)
{
	switch(m.mType) {
		case MessageType::ClaimArea:
			{
				std::cout << "You should go to " << m.mData->area << "\n";
				mApp->setTargetArea(m.mData->area);
			}
			break;

		case MessageType::EnemyDiscovered:
			MessageDispatcher::instance().dispatchMessage(Message(mUnit->getEntityID(), mUnit->getCommandingUnit()->getEntityID(),
						0.0f, 0.0f, MessageType::EnemyDiscovered, m.mData->platoon));
			break;

		case MessageType::AttackEnemy:
			// mAIController->pushController(std::unique_ptr<PlatoonAIState>(new PlatoonAICombatState(mUnit, mAIController, m.mData->platoon)));
			break;

		default:
			std::cout << "Unhandled message " << int(m.mType) << " in PlatoonAIDefendState.\n";
			break;
	}
}

