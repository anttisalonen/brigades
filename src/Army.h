#ifndef ARMY_H
#define ARMY_H

#include <list>
#include <vector>

#include "Messaging.h"
#include "PlatoonAI.h"

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

class MilitaryUnit;

class MilitaryUnitController {
	public:
		MilitaryUnitController(MilitaryUnit* m);
		virtual bool control(float dt) = 0;
		virtual void receiveMessage(const Message& m) = 0;
	protected:
		MilitaryUnit* mUnit;
};

class SimpleMilitaryUnitController : public MilitaryUnitController {
	public:
		SimpleMilitaryUnitController(MilitaryUnit* m);
		virtual void receiveMessage(const Message& m);
		virtual bool control(float dt);
};

class MilitaryUnit : public Entity {
	public:
		MilitaryUnit(ServiceBranch b, int side);
		ServiceBranch getBranch() const;
		int getSide() const;
		virtual std::list<Platoon*> update(float dt);
		virtual void receiveMessage(const Message& m);
		virtual std::list<Platoon*> getPlatoons();
		const std::vector<std::unique_ptr<MilitaryUnit>>& getUnits() const;
	protected:
		ServiceBranch mBranch;
		int mSide;
		std::vector<std::unique_ptr<MilitaryUnit>> mUnits;
		std::unique_ptr<MilitaryUnitController> mController;
};

class Platoon : public MilitaryUnit {
	public:
		Platoon(MilitaryUnit* commandingunit, const Vector2& pos,
				ServiceBranch b, int side, int pid);
		const Vector2& getPosition() const;
		void setPosition(const Vector2& v);
		ServiceBranch getBranch() const;
		int getSide() const;
		int getPlatoonID() const;
		std::list<Platoon*> update(float dt);
		void receiveMessage(const Message& m);
		std::list<Platoon*> getPlatoons();
		void setController(std::unique_ptr<PlatoonController> c);
		const MilitaryUnit* getCommandingUnit() const;
		MilitaryUnit* getCommandingUnit();
		void loseHealth(float damage);
		bool isDead() const;
		float getHealth() const;
		void moveTowards(const Vector2& v, float dt);
	private:
		void checkVisibility();
		MilitaryUnit* mCommandingUnit;
		Vector2 mPosition;
		int mPid;
		std::unique_ptr<PlatoonController> mController;
		float mHealth;
};

class Company : public MilitaryUnit {
	public:
		Company(const Vector2& pos, ServiceBranch b, int side);
};

class Battalion : public MilitaryUnit {
	public:
		Battalion(const Vector2& pos, ServiceBranch b, int side);
};

class Brigade : public MilitaryUnit {
	public:
		Brigade(const Vector2& pos, ServiceBranch b, int side, const std::vector<ServiceBranch>& config);
};

class Army : public MilitaryUnit {
	public:
		Army(const Terrain& t, const Vector2& base, int side,
				const std::vector<ServiceBranch>& armyConfiguration);
		static int getNextPid();
	private:
		const Terrain& mTerrain;
		Vector2 mBase;
		static int nextPid;
};

const char* branchToName(ServiceBranch b);
bool isCombatBranch(ServiceBranch b);
bool branchOnFoot(ServiceBranch b);

#endif

