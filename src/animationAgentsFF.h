//
//  animationAgentsFF.h
//  murmur
//
//  Created by Julien on 22/09/2015.
//
//

#pragma once
#include "animations.h"

class AnimationAgentsFF : public Animation
{
	public:
		AnimationAgentsFF			(string name);
		~AnimationAgentsFF			();
 
 
        virtual void			VM_update				(float dt);
        virtual void			VM_draw					(float w, float h);
};