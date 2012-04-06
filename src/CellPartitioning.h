#ifndef CELLPARTITIONING_H
#define CELLPARTITIONING_H

#include <list>
#include <vector>
#include <set>

#include "Terrain.h"

template<class T>
class CellPartitioning {
	public:
		CellPartitioning(float w, int cells);
		void addEntity(const T& t);
		void updateEntity(const T& t, const Vector2& oldpos);
		void getNeighbouringEntities(const T& t, float range);
		T getNextNeighbouringEntity();
		bool hasNextNeighbouringEntity();

	private:
		size_t getCellIndex(Vector2 v) const;
		std::vector<std::set<T>> mCells;
		std::vector<T> mNeighbours;
		int mCurrentNeighbour;

		float mTotalWidth;
		size_t mNumCells;
		float mCellWidth;
};

#endif

