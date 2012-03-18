#ifndef ARMY_H
#define ARMY_H

#include <list>
#include <vector>

#include "Messaging.h"

class Platoon;

class PlatoonController {
	public:
		PlatoonController(Platoon* p);
		virtual bool control(float dt) = 0;
		virtual void receiveMessage(const Message& m) = 0;
	protected:
		Platoon* mPlatoon;
};

class DummyPlatoonController : public PlatoonController {
	public:
		DummyPlatoonController(Platoon* p);
		bool control(float dt);
		void receiveMessage(const Message& m);
	private:
		bool mAsleep;
		Vector2 mTargetPos;
};

enum class ServiceBranch {
	Infantry,
	Armored,
	Artillery,
	Engineer,
	Recon,
	Signal,
	Supply
};

class MilitaryUnit : public Entity {
	public:
		MilitaryUnit(ServiceBranch b, int side);
		ServiceBranch getBranch() const;
		int getSide() const;
		virtual std::list<Platoon*> update(float dt);
		virtual void receiveMessage(const Message& m);
	protected:
		ServiceBranch mBranch;
		int mSide;
		std::vector<std::unique_ptr<MilitaryUnit>> mUnits;
};

class MilitaryUnitController {
	public:
		MilitaryUnitController(MilitaryUnit* m);
		virtual bool control() = 0;
		virtual void receiveMessage(const Message& m);
	protected:
		MilitaryUnit* m;
};

class SimpleMilitaryUnitController : public MilitaryUnitController {
	public:
		bool control();
};

class Platoon : public MilitaryUnit {
	public:
		Platoon(const Vector2& pos, ServiceBranch b, int side, int pid);
		const Vector2& getPosition() const;
		void setPosition(const Vector2& v);
		ServiceBranch getBranch() const;
		int getSide() const;
		int getPlatoonID() const;
		std::list<Platoon*> update(float dt);
		void receiveMessage(const Message& m);
	private:
		Vector2 mPosition;
		int mPid;
		std::unique_ptr<PlatoonController> mController;
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

