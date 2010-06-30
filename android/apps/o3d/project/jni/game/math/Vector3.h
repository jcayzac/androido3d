//	Vector3.h
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


#ifndef _VECTOR3_H
#define _VECTOR3_H

#include <math.h>	// for sqrtf

#include "Serializable.h"

#include "my_assert.h"

class Vector3 : public Serializable
{
	public:
		Vector3();
		Vector3(const float x, const float y, const float z);
		Vector3(const Vector3& other);
		
		float dot(const Vector3& other) const;
		Vector3 cross(const Vector3& other) const;
		
		float length() const;
		float length2() const;
		
		float normalize();
		
		float& operator[](const unsigned int index);
		float operator[](const unsigned int index) const;
		Vector3 operator+(const Vector3& other) const;
		Vector3 operator-(const Vector3& other) const;
		Vector3 operator*(const Vector3& other) const;
		Vector3 operator/(const Vector3& other) const;
		Vector3 operator*(const float magnitude) const;
		Vector3 operator/(const float magnitude) const;
				
		Vector3 operator=(const Vector3& other);
		
		Vector3 operator+=(const Vector3& other);
		Vector3 operator-=(const Vector3& other);
		Vector3 operator*=(const Vector3& other);
		Vector3 operator/=(const Vector3& other);
		Vector3 operator*=(const float magnitude);
		Vector3 operator/=(const float magnitude);

		
		static Vector3 ZERO;
		static Vector3 ONE;
		
	private:
		float mAxes[3];
};

inline Vector3::Vector3()
{
	if (!getCreateInPlace())
	{
		mAxes[0] = 0.0f;
		mAxes[1] = 0.0f;
		mAxes[2] = 0.0f;
	}
}

inline Vector3::Vector3(const float x, const float y, const float z)
{
	mAxes[0] = x;
	mAxes[1] = y;
	mAxes[2] = z;
}

inline Vector3::Vector3(const Vector3& other)
{
	mAxes[0] = other[0];
	mAxes[1] = other[1];
	mAxes[2] = other[2];
}

inline float Vector3::dot(const Vector3& other) const
{
	return (mAxes[0] * other[0]) + (mAxes[1] * other[1]) + (mAxes[2] * other[2]);
}

inline Vector3 Vector3::cross(const Vector3& other) const
{
	Vector3 normal;									
	
	normal[0] = ((mAxes[1] * other[2]) - (mAxes[2] * other[1]));
	normal[1] = ((mAxes[2] * other[0]) - (mAxes[0] * other[2]));
	normal[2] = ((mAxes[0] * other[1]) - (mAxes[1] * other[0]));
	
	return normal;		
}

inline float Vector3::length() const
{
	return sqrtf(length2());
}

inline float Vector3::length2() const
{
	return (mAxes[0] * mAxes[0]) + (mAxes[1] * mAxes[1]) + (mAxes[2] * mAxes[2]);
}

inline float Vector3::normalize()
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
	
	if (mAxes[2] != 0.0f)
	{
		mAxes[2] /= magnitude;
	}
	
	return magnitude;
}

inline float& Vector3::operator[](const unsigned int index)
{
	ASSERT(index < 3, "Vector index is out of range!");
	return mAxes[index];
}

inline float Vector3::operator[](const unsigned int index) const
{
	ASSERT(index < 3, "Vector index is out of range!");
	return mAxes[index];
}

inline Vector3 Vector3::operator+(const Vector3& other) const
{
	Vector3 result(*this);
	
	result[0] += other[0];
	result[1] += other[1];
	result[2] += other[2];
	
	return result;
}

inline Vector3 Vector3::operator-(const Vector3& other) const
{
	Vector3 result(*this);
	
	result[0] -= other[0];
	result[1] -= other[1];
	result[2] -= other[2];
	
	return result;
}

inline Vector3 Vector3::operator*(const Vector3& other) const
{
	Vector3 result(*this);
	
	result[0] *= other[0];
	result[1] *= other[1];
	result[2] *= other[2];
	
	return result;
}

inline Vector3 Vector3::operator/(const Vector3& other) const
{
	Vector3 result(*this);
	
	if (result[0] != 0.0f && other[0] != 0.0f)
	{
		result[0] /= other[0];
	}
	
	if (result[1] != 0.0f && other[1] != 0.0f)
	{
		result[1] /= other[1];
	}
	
	if (result[2] != 0.0f && other[2] != 0.0f)
	{
		result[2] /= other[2];
	}
	
	return result;
}

inline Vector3 Vector3::operator*(const float magnitude) const
{
	Vector3 result(*this);

	result[0] *= magnitude;
	result[1] *= magnitude;
	result[2] *= magnitude;
	
	return result;
}

inline Vector3 Vector3::operator/(const float magnitude) const
{
	ASSERT(magnitude != 0.0f, "Vector division by zero.");

	Vector3 result(*this);

	if (result[0] != 0.0f)
	{
		result[0] /= magnitude;
	}
	
	if (result[1] != 0.0f)
	{
		result[1] /= magnitude;
	}
	
	if (result[2] != 0.0f)
	{
		result[2] /= magnitude;
	}
	
	return result;
}


inline Vector3 Vector3::operator=(const Vector3& other)
{
	mAxes[0] = other[0];
	mAxes[1] = other[1];
	mAxes[2] = other[2];
	
	return *this;
}

inline Vector3 Vector3::operator+=(const Vector3& other)
{
	*this = *this + other;
	
	return *this;
}

inline Vector3 Vector3::operator-=(const Vector3& other)
{
	*this = *this - other;
	
	return *this;
}

inline Vector3 Vector3::operator*=(const Vector3& other)
{
	*this = *this * other;
	
	return *this;
}

inline Vector3 Vector3::operator/=(const Vector3& other)
{
	*this = *this / other;
	
	return *this;
}


inline Vector3 Vector3::operator*=(const float magnitude)
{
	*this = *this * magnitude;
	
	return *this;
}

inline Vector3 Vector3::operator/=(const float magnitude)
{
	*this = *this / magnitude;
	
	return *this;
}


#endif //_VECTOR3_H