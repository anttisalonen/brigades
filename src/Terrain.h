#ifndef TERRAIN_H
#define TERRAIN_H

#include <noise/noise.h>

class Terrain {
	public:
		Terrain();
		float getHeightAt(float x, float y) const;
		float getVegetationAt(float x, float y) const;
		float getHeightScale() const;
		float getWidth() const;
	private:
		noise::module::Perlin mPerlin;
		float mHeightScale;
		float mHeightPlaneScale;
		float mVegetationScale;
};



#endif

