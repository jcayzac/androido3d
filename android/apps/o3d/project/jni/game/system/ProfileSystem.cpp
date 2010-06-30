//	ProfileSystem.cpp
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


#include "ProfileSystem.h"

#include "my_assert.h"
#include "SystemRegistry.h"
#include "TimeSystem.h"


ProfileSystem::ProfileSystem()
:	mTimingTable(true, PROFILE_maxSimultaniousRecords, true)
{
	mTimingRecordPool.reserve(PROFILE_maxSimultaniousRecords);
}

void ProfileSystem::startTracking(const char* label)
{
	const int recordIndex = getTimingTable().findIndex(label);
	TimingRecord* record = NULL;
	
	if (recordIndex > -1)
	{
		record = getTimingTable().get(recordIndex);
	}
	else
	{
		TimingRecord newRecord;
		newRecord.mName = label;
		getTimingRecordPool().append(newRecord);
		record = getTimingRecordPool().getSlot(getTimingRecordPool().getCount() - 1);
		ASSERT(record, "Can't find record I just added!");
		ASSERT(record->mName == label, "Profile record mismatch!");
		
		getTimingTable().add(label, record);
		const int activeCount = getActiveStack().getCount();

		if (activeCount == 0)
		{
			// No parents means that this is a root record.
			getRootRecords().append(record);
		}
		else
		{
			TimingRecord* currentRecord = NULL;
			if (activeCount > 0)
			{
				currentRecord = getActiveStack().get(activeCount - 1);
				ASSERT(currentRecord != record, "Active profile cycle!");
				if (currentRecord->mChild)
				{
					record->mSibling = currentRecord->mChild;
				}
				currentRecord->mChild = record;
			}
		}
	}
	
	ASSERT(record, "New timing record could not be created!");
	if (record)
	{
		record->mLastStartTime = getSystem<TimeSystem>()->getRawOSTime();
		getActiveStack().append(record);
	}
}

void ProfileSystem::stopTracking()
{
	const int activeCount = getActiveStack().getCount();
	// If reset() was called in the middle of a profile, the active stack may be empty.  In that case
	// just ignore and continue to the next frame.
	if (activeCount > 0)
	{
		TimingRecord* currentRecord = getActiveStack().get(activeCount - 1);
		const double delta = getSystem<TimeSystem>()->getRawOSTime() - currentRecord->mLastStartTime;
		currentRecord->mTotalTime += delta;
		currentRecord->mSampleCount++;
		getActiveStack().remove(activeCount - 1);
	}
}
	
float ProfileSystem::getAverageDuration(const char* label)
{
	float duration = 0.0f;
	int recordIndex = getTimingTable().findIndex(label);
	if (recordIndex > -1)
	{
		duration = computeAverageDuration(getTimingTable().get(recordIndex));
	}
	return duration;
}

float ProfileSystem::computeAverageDuration(const TimingRecord* record)
{
	ASSERT(record, "NULL timing record!");
	const float ms = (record->mTotalTime / record->mSampleCount) * 1000;
	return ms;
}

void ProfileSystem::printResults(const float currentGameTime) 
{	
	printf("-------------------------------------\n");
	const float sampleDuration = currentGameTime - getLastIntervalStart();
	const float fps = getIntervalSamples() / sampleDuration;
	printf("FPS: %g\n", fps);
	printf("-------------------------------------\n");
	const int rootRecordCount = getRootRecords().getCount();
	for (int x = 0; x < rootRecordCount; x++)
	{
		printRecordTree(getRootRecords().get(x), sampleDuration, sampleDuration, 0);
	}
}

void ProfileSystem::printRecordTree(const TimingRecord* record, const float sampleDuration, const float parentDuration, int level)
{
	for (int x = 0; x < level; x++)
	{
		printf("  ");
	}
	const float time = computeAverageDuration(record);
	const float percentageTotal = (record->mTotalTime / sampleDuration) * 100.0f;
	const float percentageRelative = (record->mTotalTime / parentDuration) * 100.0f;

	printf("* %s \t %g ms \t %g%% (relative) \t %g%% (total) \t %d invocations (%d / frame)\n", record->mName, time, percentageRelative, percentageTotal, 
		record->mSampleCount, record->mSampleCount / getIntervalSamples());
	
	TimingRecord const* child = record->mChild;
	while (child)
	{
		printRecordTree(child, sampleDuration, record->mTotalTime, level + 1);
		child = child->mSibling;
	}
}

void ProfileSystem::reset() 
{
	getTimingRecordPool().removeAll();
	getTimingTable().removeAll();
	getActiveStack().removeAll();
	getRootRecords().removeAll();
	setIntervalSamples(0);
}

void ProfileSystem::update(const float, const UpdatePhase)
{
	const float currentTime = getSystem<TimeSystem>()->getGameTime();
	if (currentTime - getLastIntervalStart() > getTrackingInterval())
	{
		printResults(currentTime);
		reset();
		setLastIntervalStart(currentTime);
	}
	else
	{
		setIntervalSamples(getIntervalSamples() + 1);
	}
}