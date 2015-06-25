//
//  animationsFactory.cpp
//  murmur
//
//  Created by Julien on 16/05/13.
//
//

#include "animationsFactory.h"
#include "animations.h"

#include "animationWaveShader.h"
#include "animationParticlesMega2.h"
#include "animationBox2D.h"
#include "animationTunnel.h"
#include "animationTunnelALB.h"
#include "animationOrbit.h"
#include "animationGravitatory.h"
#include "animationComposition.h"

Animation* AnimationsFactory::create(string name)
{
    Animation* pAnimation=0;

    if (name == "shaderWave")
        pAnimation = new AnimationShaderWave(name);
/*    if (name == "particlesMega")
        pAnimation = new AnimationParticlesMega(name);
*/
    else if (name == "particlesMega2")
        pAnimation = new AnimationParticlesMega2(name);
    else if (name == "box2DCircles")
        pAnimation = new AnimationBox2D_circles(name);
    else if (name == "tunnel")
        pAnimation = new AnimationTunnel(name);
    else if (name == "tunnelALB")
        pAnimation = new AnimationTunnelALB(name);
    else if (name == "orbit")
        pAnimation = new AnimationOrbit(name);
    else if (name == "gravitatory")
        pAnimation = new AnimationGravitatory(name);
    else if (name == "composition")
        pAnimation = new AnimationComposition(name);
 
    return pAnimation;
}