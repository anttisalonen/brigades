#ifndef PAPAYA_H
#define PAPAYA_H

#include <memory>
#include <vector>
#include <list>

#include "Terrain.h"
#include "Messaging.h"

class Platoon;

class PlatoonController : public Entity {
	public:
		PlatoonController(Platoon* p);
		virtual bool control() = 0;
	protected:
		Platoon* mPlatoon;
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

const char* BranchToName(ServiceBranch b);

class MilitaryUnit : public Entity {
	public:
		MilitaryUnit(ServiceBranch b, int side);
		ServiceBranch getBranch() const;
		int getSide() const;
		virtual std::list<Platoon*> update();
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

class DummyPlatoonController : public PlatoonController {
	public:
		DummyPlatoonController(Platoon* p);
		bool control();
		void receiveMessage(const Message& m);
	private:
		bool mAsleep;
};

class Platoon : public MilitaryUnit {
	public:
		Platoon(const Vector2& pos, ServiceBranch b, int side, int pid);
		const Vector2& getPosition() const;
		void setPosition(const Vector2& v);
		ServiceBranch getBranch() const;
		int getSide() const;
		int getPlatoonID() const;
		std::list<Platoon*> update();
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

class PapayaEventListener {
	public:
		virtual void PlatoonStatusChanged(const Platoon* p) = 0;
};

class Papaya {
	public:
		Papaya(const Terrain& t);
		void process(float dt);
		const Army* getArmy(size_t side) const;
		void addEventListener(PapayaEventListener* l);
		void removeEventListener(PapayaEventListener* l);
	private:
		const Terrain& mTerrain;
		std::vector<std::unique_ptr<Army>> mArmies;
		std::vector<PapayaEventListener*> mListeners;
};

#endif
