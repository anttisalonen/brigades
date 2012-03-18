#include <iostream>

#include "Terrain.h"
#include "Utils.h"

std::ostream& operator<<(std::ostream& out, const Vector2& vec)
{
	out << "(" << vec.x << ", " << vec.y << ")";
	return out;
}

Vector2 Vector2::operator+(const Vector2& v2) const
{
	return Vector2(x + v2.x, y + v2.y);
}

Vector2 Vector2::operator-(const Vector2& v2) const
{
	return Vector2(x - v2.x, y - v2.y);
}

float Vector2::length() const
{
	return sqrt(length2());
}

float Vector2::length2() const
{
	return x * x + y * y;
}

void Vector2::normalize()
{
	if(x == 0.0f && y == 0.0f)
		return;
	float l = length();
	x /= l;
	y /= l;
}

Vector2 Vector2::normalized() const
{
	Vector2 v(*this);
	v.normalize();
	return v;
}

void Vector2::operator+=(const Vector2& v2)
{
	x += v2.x;
	y += v2.y;
}

void Vector2::operator*=(float f)
{
	x *= f;
	y *= f;
}

Terrain::Terrain()
	: mHeightScale(10.0f),
	mHeightPlaneScale(0.04f),
	mVegetationScale(0.08f)
{
}

float Terrain::getHeightAt(const Vector2& v) const
{
	float value = mPerlin.GetValue(v.x * mHeightPlaneScale, v.y * mHeightPlaneScale, 0.0);
	return clamp(0.0f, (value * 0.6f + 1.0f) / 2.0f, 1.0f);
}

float Terrain::getVegetationAt(const Vector2& v) const
{
	float value = mPerlin.GetValue(v.x * mVegetationScale, v.y * mVegetationScale, 1.0);
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

