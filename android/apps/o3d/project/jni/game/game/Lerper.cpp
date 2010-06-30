//	Lerper.cpp
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

#include "Lerper.h"

#include <math.h>

float Lerper::lerp(const float start, const float target, const float duration, const float timeSinceStart)
{
	float value = start;
	if (timeSinceStart > 0.0f && timeSinceStart < duration)
	{
		const float range = target - start;
		const float percent = timeSinceStart / duration;
		value = start + (range * percent);
	}
	else if (timeSinceStart >= duration)
	{
		value = target;
	}
	return value;
}

float Lerper::ease(const float start, const float target, const float duration, const float timeSinceStart)
{
	float value = start;
	if (timeSinceStart > 0.0f && timeSinceStart < duration)
	{
		const float range = target - start;
		const float percent = timeSinceStart / (duration / 2.0f);
		if (percent < 1.0f)
		{
			value = start + ((range / 2.0f) * percent * percent * percent);
		}
		else
		{
			const float shiftedPercent = percent - 2.0f;
			value = start + ((range / 2.0f) * ((shiftedPercent * shiftedPercent * shiftedPercent) + 2.0f));
		}
	}
	else if (timeSinceStart >= duration)
	{
		value = target;
	}
	return value;
}