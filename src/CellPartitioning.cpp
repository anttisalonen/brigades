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
	mCells.at(i).insert(t);
}

template<class T>
void CellPartitioning<T>::updateEntity(const T& t, const Vector2& oldpos)
{
	size_t i = getCellIndex(oldpos);
	size_t j = getCellIndex(t->getPosition());
	if(i != j) {
		if(mCells.at(i).erase(t) != 1) {
			std::cout << "Updating entity " << t->getEntityID() << " that does not exist?\n";
			for(size_t i = 0; i < mNumCells - 1; i++) {
				for(size_t j = 0; j < mNumCells - 1; j++) {
					for(T t2 : mCells.at(j * mNumCells + i)) {
						if(t2->getEntityID() == t->getEntityID()) {
							std::cout << "Entity was supposed to be at " << i << ", but it is at " << j * mNumCells + i << ".\n";
							std::cout << "(New position: " << j << "\n";
						}
					}
				}
			}
			std::cout << "Done searching for the entity.\n";
			return;
		}
		mCells.at(j).insert(t);
	}
}

template<class T>
void CellPartitioning<T>::getNeighbouringEntities(const T& t, float range)
{
	size_t numcells = 1 + range / mCellWidth;
	size_t ci = getCellIndex(t->getPosition());
	size_t si = ci % mNumCells;
	size_t sj = ci / mNumCells;
	mNeighbours.clear();
#ifdef CELL_DEBUG
	size_t cell1006 = 0;
	bool checked = false;
	Platoon* pl = dynamic_cast<Platoon*>(EntityManager::instance().getEntity(1004));
	std::vector<std::pair<size_t, size_t>> checkedcells;
	if(t->getEntityID() == 1006) {
		// std::cout << "Pos: " << t->getPosition() << " Index: " << ci << " si: " << si << " sj: " << sj << "\n";
		cell1006 = getCellIndex(pl->getPosition());
	}
#endif
	for(size_t i = std::max<size_t>(0, si - numcells - 1); i <= std::min<size_t>(mNumCells - 1, si + numcells - 1); i++) {
		for(size_t j = std::max<size_t>(0, sj - numcells - 1); j <= std::min<size_t>(mNumCells - 1, sj + numcells - 1); j++) {
			if(j * mNumCells + i >= mCells.size()) {
				std::cout << "\nAbout to crash. j * mNumCells + i >= mCells.size().\n" <<
					j << " * " << mNumCells << " + " << i << " = " << j * mNumCells + i << " >= " << mCells.size() << ".\n";
			}
#ifdef CELL_DEBUG
			if(j * mNumCells + i == cell1006)
				checked = true;
			checkedcells.push_back(std::make_pair(i, j));
#endif
			for(T t2 : mCells.at(j * mNumCells + i)) {
#ifdef CELL_DEBUG
				if(t->getEntityID() == 1006 && t != t2)
					std::cout << "At " << j * mNumCells + i << ": " << t2->getEntityID() << "\n";
#endif
				if(t != t2 && (t->getPosition() - t2->getPosition()).length() <= range) {
					mNeighbours.push_back(t2);
				}
			}
		}
	}
#ifdef CELL_DEBUG
	if(t->getEntityID() == 1006) {
		std::cout << "Diff to 1004: " << (t->getPosition() - pl->getPosition()).length();
		std::cout << " My cell: (" << si << ", " << sj << ") = " << ci << " - his cell: (" << cell1006 % mNumCells << ", " << cell1006 / mNumCells << ") = " << cell1006 << "\n";
		if(!checked) {
			std::cout << "Did not check his cell. Checked cells:";
			for(auto c : checkedcells)
				std::cout << " (" << c.first << ", " << c.second << ") = " << c.second * mNumCells + c.first;
			std::cout << "\n";
		}
	}
#endif
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


