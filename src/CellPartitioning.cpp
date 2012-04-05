#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#include <stdexcept>
#include "CellPartitioning.h"
#include "MilitaryUnit.h"

template<class T>
CellPartitioning<T>::CellPartitioning(float w, int cells)
	: mTotalWidth(w),
	mNumCells(cells),
	mCellWidth(w / (float)cells)
{
	mCells.resize(mNumCells * mNumCells);
}

template<class T>
void CellPartitioning<T>::addEntity(const T& t)
{
	size_t i = getCellIndex(t->getPosition());
	mCells.at(i).push_back(t);
}

template<class T>
void CellPartitioning<T>::updateEntity(const T& t, const Vector2& oldpos)
{
	size_t i = getCellIndex(oldpos);
	size_t j = getCellIndex(t->getPosition());
	if(i != j) {
		mCells.at(i).remove(t);
		mCells.at(j).push_back(t);
	}
}

template<class T>
void CellPartitioning<T>::getNeighbouringEntities(const T& t, float range)
{
	size_t numcells = 1 + range / mCellWidth;
	size_t ci = getCellIndex(t->getPosition());
	size_t si = ci / mNumCells;
	size_t sj = ci % mNumCells;
	mNeighbours.clear();
	size_t maxcells = mNumCells * mNumCells - 1;
	for(size_t i = std::max<size_t>(0, si - numcells - 1); i <= std::min<size_t>(maxcells, si + numcells - 1); i++) {
		for(size_t j = std::max<size_t>(0, sj - numcells - 1); j <= std::min<size_t>(maxcells, sj + numcells - 1); j++) {
			for(T t2 : mCells.at(j * mNumCells + i)) {
				if((t->getPosition() - t2->getPosition()).length() <= range) {
					mNeighbours.push_back(t2);
				}
			}
		}
	}
	if(mNeighbours.size())
		mCurrentNeighbour = 0;
	else
		mCurrentNeighbour = -1;
}

template<class T>
T CellPartitioning<T>::getNextNeighbouringEntity()
{
	if(!hasNextNeighbouringEntity())
		throw std::runtime_error("No more neighbours - use CellPartitioning<T>::hasNextNeighbouringEntity()");
	return mNeighbours[mCurrentNeighbour++];
}

template<class T>
bool CellPartitioning<T>::hasNextNeighbouringEntity()
{
	return mCurrentNeighbour != -1 && mCurrentNeighbour < (int)mNeighbours.size();
}

template<class T>
size_t CellPartitioning<T>::getCellIndex(Vector2 v) const
{
	if(v.x < 0.0f)
		v.x = 0.0f;
	if(v.y < 0.0f)
		v.y = 0.0f;
	if(v.x >= mTotalWidth)
		v.x = mTotalWidth - mCellWidth * 0.5f;
	if(v.y >= mTotalWidth)
		v.y = mTotalWidth - mCellWidth * 0.5f;
	size_t value = (int)(v.x / mCellWidth) + mNumCells * (int)(v.y / mCellWidth);
	assert(value >= 0 && value < mCells.size());
	return value;
}

template class CellPartitioning<Platoon*>;


