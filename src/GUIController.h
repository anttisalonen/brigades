#ifndef GUICONTROLLER_H
#define GUICONTROLLER_H

#include "MilitaryUnit.h"
#include "Messaging.h"
#include "App.h"

class GUIController : public Controller<MilitaryUnit> {
	public:
		GUIController(App* app, MilitaryUnit* p);
		~GUIController();
		bool control(float dt);
		void receiveMessage(const Message& m);
	private:
		App* mApp;
};

#endif

