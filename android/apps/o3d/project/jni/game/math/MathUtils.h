//	MathUtils.h
//	Copyright (C) 2008 Chris Pruett.		c_pruett@efn.org
//
//	FarClip Engine
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//			http://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.


#ifndef _MATHUTILS_H
#define _MATHUTILS_H

#include "my_assert.h"

#include <stdlib.h>


inline int Abs(const int& value)
{
	return (value < 0) ? -value : value;
} 

inline float Abs(const float& value)
{
	return (value < 0.0f) ? -value : value;
} 

inline bool Close(const float& value1, const float& value2, const float epsilon = 0.00001f)
{
	return (Abs(value1 - value2) < epsilon);	
} 

template <typename T>
inline T Max(const T& value1, const T& value2)
{
	return (value1 > value2) ? value1 : value2;
}

template <typename T>
inline T Min(const T& value1, const T& value2)
{
	return (value1 < value2) ? value1 : value2;
}

template <typename T>
inline int Sign(const T& value)
{
	return (value < 0) ? -1 : 1;
}

template <typename T>
inline T Clamp(const T& value, const T& min, const T& max)
{
	T result = value;
	
	if (value > max)
	{
		result = max;
	}
	else if (value < min)
	{
		result = min;
	}
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// This will rescale a value from one range into another
// If start > end or Min > Max then the arguments are swapped
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline T Rescale(const T& tValue, const T& tStart, const T& tEnd, const T& tMin, const T& tMax)
{
	// When mapping to or from a range that is too small, return one end of the range
	// it doesn't matter which...

	if (Abs(tEnd - tStart) == 0.0f)
	{
		return tEnd;
	}

	if (Abs(tMax - tMin) == 0.0f)
	{
		return tMax;
	}

	// Get the arguments the right way around

	T tMin2 = Min(tMin, tMax);
	T tMax2 = Max(tMin, tMax);;
	T tStart2 = Min(tStart, tEnd);
	T tEnd2 = Max(tStart, tEnd);

	// Do the actual rescaling
	const T tScale = (tMax2 - tMin2) / (tEnd2 - tStart2);
	const T tScaledValue = ((tValue - tStart2) * tScale) + tMin2;

	return Clamp(tScaledValue, tMin2, tMax2);
}

// returns a random value between zero and 1.0
inline float Random()
{
	float numerator = (rand() % 1000);
	if (numerator == 0.0f)
	{
		numerator = 1.0f;
	}
	return numerator / 1000.0f;
}

// returns a random value between zero and range
inline float Random(int range)
{
	ASSERT(range != 0, "Attempt to calculate random value with a range of zero!");
	return rand() % range;
}


#endif //_MATHUTILS_H