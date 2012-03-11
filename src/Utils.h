#ifndef UTILS_H
#define UTILS_H

template<typename T>
T clamp(const T& mn, const T& v, const T& mx)
{
	return std::max(mn, std::min(v, mx));
}

#endif
