#ifndef MILITARYUNIT_H
#define MILITARYUNIT_H

#include <list>

#include "Messaging.h"

class Platoon;

enum class ServiceBranch {
	Infantry,
	Armored,
	Artillery,
	Engineer,
	Recon,
	Signal,
	Supply
};

enum class UnitSize {
	Single,
	Squad,
	Platoon,
	Company,
	Battalion,
	Brigade,
	Division
};

class MilitaryUnit;

template <class T>
class Controller {
	public:
		Controller<T>(T* m) : mUnit(m) { }
		~Controller<T>() { }
		virtual bool control(float dt) = 0;
		virtual void receiveMessage(const Message& m) = 0;
	protected:
		T* mUnit;
};

class MilitaryUnit : public Entity {
	public:
		MilitaryUnit(MilitaryUnit* commandingunit, ServiceBranch b, int side);
		~MilitaryUnit() { }
		ServiceBranch getBranch() const;
		int getSide() const;
		virtual std::list<Platoon*> update(float dt);
		virtual void receiveMessage(const Message& m);
		virtual std::list<Platoon*> getPlatoons();
		const std::vector<std::shared_ptr<MilitaryUnit>>& getUnits() const;
		std::vector<std::shared_ptr<MilitaryUnit>>& getUnits();
		virtual UnitSize getUnitSize() const = 0;
		const MilitaryUnit* getCommandingUnit() const;
		MilitaryUnit* getCommandingUnit();
		virtual Vector2 getPosition() const;
		float distanceTo(const MilitaryUnit& m) const;
		void setController(std::shared_ptr<Controller<MilitaryUnit>> c);
	protected:
		Vector2 spawnUnitDisplacement() const;
		MilitaryUnit* mCommandingUnit;
		ServiceBranch mBranch;
		int mSide;
		std::vector<std::shared_ptr<MilitaryUnit>> mUnits;
	private:
		std::shared_ptr<Controller<MilitaryUnit>> mController;
};

class Platoon : public MilitaryUnit {
	public:
		Platoon(MilitaryUnit* commandingunit, const Vector2& pos,
				ServiceBranch b, int side);
		Vector2 getPosition() const;
		void setPosition(const Vector2& v);
		ServiceBranch getBranch() const;
		int getSide() const;
		std::list<Platoon*> update(float dt);
		void receiveMessage(const Message& m);
		std::list<Platoon*> getPlatoons();
		void setController(std::shared_ptr<Controller<Platoon>> c);
		void loseHealth(float damage);
		bool isDead() const;
		float getHealth() const;
		void moveTowards(const Vector2& v, float dt);
		UnitSize getUnitSize() const;
	private:
		void checkVisibility();
		Vector2 mPosition;
		std::shared_ptr<Controller<Platoon>> mController;
		float mHealth;
};

class Company : public MilitaryUnit {
	public:
		Company(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side);
		UnitSize getUnitSize() const;
};

class Battalion : public MilitaryUnit {
	public:
		Battalion(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side);
		UnitSize getUnitSize() const;
};

class Brigade : public MilitaryUnit {
	public:
		Brigade(MilitaryUnit* commandingunit, const Vector2& pos, ServiceBranch b, int side, const std::vector<ServiceBranch>& config);
		UnitSize getUnitSize() const;
};

const char* branchToName(ServiceBranch b);
const char* unitSizeToName(UnitSize s);
bool isCombatBranch(ServiceBranch b);
bool branchOnFoot(ServiceBranch b);

#endif
