#ifndef PAPAYA_H
#define PAPAYA_H

#include <memory>
#include <vector>
#include <list>

#include "Terrain.h"
#include "Messaging.h"
#include "Army.h"
#include "CellPartitioning.h"

class PapayaEventListener {
	public:
		virtual void PlatoonStatusChanged(const Platoon* p) = 0;
};

class Papaya {
	public:
		Papaya();
		void setup(const Terrain* t);
		void process(float dt);
		const std::shared_ptr<Army> getArmy(size_t side) const;
		std::shared_ptr<Army> getArmy(size_t side);
		void addEventListener(PapayaEventListener* l);
		void removeEventListener(PapayaEventListener* l);
		static Papaya& instance();
		float getPlatoonSpeed(const Platoon& p) const;
		float getCurrentTime() const;
		Platoon* getNeighbouringPlatoons(Platoon* p, float range);
		Platoon* getNextNeighbouringPlatoon();
		void updateEntityPosition(Platoon* p, const Vector2& oldpos);
	private:
		const Terrain* mTerrain;
		std::vector<std::shared_ptr<Army>> mArmies;
		std::vector<PapayaEventListener*> mListeners;
		float mTime;
		CellPartitioning<Platoon*> mPlatoonCells;
};

#endif
