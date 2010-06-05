//	Serializable.h
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


#ifndef _SERIALIZABLE_H
#define _SERIALIZABLE_H

// any class that needs to operate differently when being created
// on file load should inherit from this and test to see
// if this is in-place creation in the constructor before
// initializing data.

class Serializable
{
	public:
		static bool getCreateInPlace();
		static void setCreateInPlace(bool createInPlace);
		
	private:
		static bool s_mCreateInPlace;
		
};

inline bool Serializable::getCreateInPlace()
{
	return s_mCreateInPlace;
}

inline void Serializable::setCreateInPlace(bool createInPlace)
{
	s_mCreateInPlace = createInPlace;
}

#endif //_SERIALIZABLE_H