//	AnimationComponent.cpp
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

#include "AnimationComponent.h"

#include "GameObject.h"
#include "SystemRegistry.h"
#include "TimeSystem.h"

#include "MathUtils.h"

#include "scene.h"

#include "third_party/loggingshim/base/logging.h"


float AnimationComponent::AnimationRecord::getAnimationTime(
  float startTime, float currentTime) const
{
  float localTime = 0.0f;
  
  const float length = getFrameCount() / getFramesPerSecond();
  const float timeDelta = currentTime - startTime;
  if (getLooping() && timeDelta > length)
  {
    localTime = fmodf(timeDelta, length);
  } 
  else 
  {
    localTime = Min(timeDelta, length);
  }
  
  const float animationTime = localTime + (getStartFrame() / getFramesPerSecond());
  
  return animationTime;
}


void AnimationComponent::update(const float timeDelta, GameObject* pParentObject)
{
  const int requestedAnim = pParentObject->getRuntimeData()->getInt("currentAnimation");
  if (requestedAnim != getCurrentAnimation())
  {
    playAnimation(requestedAnim);
  }
  
  const int currentAnim = getCurrentAnimation();
  o3d_utils::Scene* scene = getSceneRoot();
  
  
  if (currentAnim >= 0 && scene)
  {
    AnimationRecord* anim = getAnimations()->get(currentAnim);
    if (anim)
    {
	    const TimeSystem* pTimeSystem = getSystem<TimeSystem>();
      
      const float animTime = anim->getAnimationTime(
        getCurrentAnimationStartTime(), pTimeSystem->getGameTime());
              
      scene->SetAnimationTime(animTime);
    }
  }
}

	
int AnimationComponent::addAnimation(AnimationRecord* animation)
{
  getAnimations()->append(animation);
  return getAnimations()->getCount() - 1;
}

void AnimationComponent::playAnimation(int animation)
{
  if (animation < getAnimations()->getCount())
  {
	  const TimeSystem* pTimeSystem = getSystem<TimeSystem>();
    mCurrentAnimationStartTime = pTimeSystem->getGameTime();
    mCurrentAnimation = animation;
  }
}
