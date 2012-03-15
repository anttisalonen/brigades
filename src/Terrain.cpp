#include <iostream>

#include "Terrain.h"
#include "Utils.h"

Terrain::Terrain()
	: mHeightScale(10.0f),
	mHeightPlaneScale(0.04f),
	mVegetationScale(0.08f)
{
}

float Terrain::getHeightAt(float x, float y) const
{
	float value = mPerlin.GetValue(x * mHeightPlaneScale, y * mHeightPlaneScale, 0.0);
	return clamp(0.0f, (value * 0.6f + 1.0f) / 2.0f, 1.0f);
}

float Terrain::getVegetationAt(float x, float y) const
{
	float value = mPerlin.GetValue(x * mVegetationScale, y * mVegetationScale, 1.0);
	return clamp(0.0f, (value * 0.8f + 1.0f) / 2.0f, 1.0f);
}

float Terrain::getHeightScale() const
{
	return mHeightScale;
}

float Terrain::getWidth() const
{
	return 128.0f;
}

