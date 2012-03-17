#ifndef PAPAYA_H
#define PAPAYA_H

#include <memory>
#include <vector>

#include "Terrain.h"
#include "Messaging.h"

struct Vector2 {
	float x;
	float y;
	Vector2() : x(0), y(0) { }
	Vector2(float x_, float y_) : x(x_), y(y_) { }
};

class Platoon;

class PlatoonController {
	public:
		virtual bool control() = 0;
};

class DummyPlatoonController : public PlatoonController {
	public:
		DummyPlatoonController(Platoon* p);
		bool control();
	private:
		bool mAsleep;
};

class Platoon : public Entity {
	public:
		Platoon(const Vector2& pos, int side, int pid);
		const Vector2& getPosition() const;
		int getSide() const;
		int getPlatoonID() const;
		bool update();
		void receiveMessage(const Message& m);
	private:
		Vector2 mPosition;
		int mSide;
		int mPid;
		std::unique_ptr<PlatoonController> mController;
};

class Army {
	public:
		Army(const Terrain& t, const Vector2& base, int side);
		std::vector<const Platoon*> getPlatoons() const;
		std::vector<Platoon*> getPlatoons();
		void addPlatoon();
	private:
		const Terrain& mTerrain;
		Vector2 mBase;
		std::vector<std::unique_ptr<Platoon>> mPlatoons;
		int mSide;
		static int nextPid;
		static int getNextPid();
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
