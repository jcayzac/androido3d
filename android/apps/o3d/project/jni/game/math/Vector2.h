//	Vector2.h
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


#ifndef _VECTOR2_H
#define _VECTOR2_H

#include <math.h>	// for sqrtf

#include "my_assert.h"
#include "Serializable.h"

class Vector2 : public Serializable
{
	public:
		Vector2();
		Vector2(const float x, const float y);
		Vector2(const Vector2& other);
		
		float dot(const Vector2& other) const;
		
		float length() const;
		float length2() const;
		
		float normalize();
		
		float& operator[](const unsigned int index);
		float operator[](const unsigned int index) const;
		Vector2 operator+(const Vector2& other) const;
		Vector2 operator-(const Vector2& other) const;
		Vector2 operator*(const float magnitude) const;
		Vector2 operator/(const float magnitude) const;
		
		Vector2 operator=(const Vector2& other);
		
		Vector2 operator+=(const Vector2& other);
		Vector2 operator-=(const Vector2& other);
		Vector2 operator*=(const float magnitude);
		Vector2 operator/=(const float magnitude);

		
		static Vector2 ZERO;
		static Vector2 ONE;
		
	private:
		float mAxes[2];
};

inline Vector2::Vector2()
{
	if (!getCreateInPlace())
	{
		mAxes[0] = 0.0f;
		mAxes[1] = 0.0f;
	}
}

inline Vector2::Vector2(const float x, const float y)
{
	mAxes[0] = x;
	mAxes[1] = y;
}

inline Vector2::Vector2(const Vector2& other)
{
	mAxes[0] = other[0];
	mAxes[1] = other[1];
}

inline float Vector2::dot(const Vector2& other) const
{
	return (mAxes[0] * other[0]) + (mAxes[1] * other[1]);
}

inline float Vector2::length() const
{
	return sqrtf(length2());
}

inline float Vector2::length2() const
{
	return (mAxes[0] * mAxes[0]) + (mAxes[1] * mAxes[1]);
}

inline float Vector2::normalize()
{
	const float magnitude = length();
	
	// TODO: I'm choosing safety over speed here.
	if (mAxes[0] != 0.0f)
	{
		mAxes[0] /= magnitude;
	}
	
	if (mAxes[1] != 0.0f)
	{
		mAxes[1] /= magnitude;
	}
	
	
	return magnitude;
}

inline float& Vector2::operator[](const unsigned int index)
{
	ASSERT(index < 2, "Vector index is out of range!");
	return mAxes[index];
}

inline float Vector2::operator[](const unsigned int index) const
{
	ASSERT(index < 2, "Vector index is out of range!");
	return mAxes[index];
}

inline Vector2 Vector2::operator+(const Vector2& other) const
{
	Vector2 result(*this);
	
	result[0] += other[0];
	result[1] += other[1];
	
	return result;
}

inline Vector2 Vector2::operator-(const Vector2& other) const
{
	Vector2 result(*this);
	
	result[0] -= other[0];
	result[1] -= other[1];
	
	return result;
}

inline Vector2 Vector2::operator*(const float magnitude) const
{
	Vector2 result(*this);

	result[0] *= magnitude;
	result[1] *= magnitude;

	return result;
}

inline Vector2 Vector2::operator/(const float magnitude) const
{
	ASSERT(magnitude != 0.0f, "Vector division by zero.");

	Vector2 result(*this);

	if (result[0] != 0.0f)
	{
		result[0] /= magnitude;
	}
	
	if (result[1] != 0.0f)
	{
		result[1] /= magnitude;
	}
	
	return result;
}


inline Vector2 Vector2::operator=(const Vector2& other)
{
	mAxes[0] = other[0];
	mAxes[1] = other[1];
	
	return *this;
}

inline Vector2 Vector2::operator+=(const Vector2& other)
{
	*this = *this + other;
	
	return *this;
}

inline Vector2 Vector2::operator-=(const Vector2& other)
{
	*this = *this - other;
	
	return *this;
}

inline Vector2 Vector2::operator*=(const float magnitude)
{
	*this = *this * magnitude;
	
	return *this;
}

inline Vector2 Vector2::operator/=(const float magnitude)
{
	*this = *this / magnitude;
	
	return *this;
}


#endif //_VECTOR3_H