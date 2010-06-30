//	Renderer.cpp
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


#include "Renderer.h" // self

#include "RenderableObject.h"

Renderer::Renderer()
	: mObjectsToDraw(false, MAX_objectsPerFrame)
{
}

void Renderer::scheduleForDraw(RenderableObject* pObject, const Vector3& position, const int priority)
{
	DrawRecord record;
	record.mObject = pObject;
	record.mPosition = position;
	mObjectsToDraw.add(priority, record);
}

void Renderer::update(const float timeDelta, const UpdatePhase phase)
{
	mObjectsToDraw.removeAll();
}