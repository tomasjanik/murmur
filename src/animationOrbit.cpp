//
//  animationOrbit.cpp
//  murmur
//
//  Created by Julien on 25/09/2014.
//
//

#include "animationOrbit.h"
#include "globals.h"

//--------------------------------------------------------------
BoidOrbit::BoidOrbit(AnimationOrbit* pAnimation, float x, float y) : Boid(x,y)
{
	mp_animation = pAnimation;
	setSpeedMinMax(1.0,2.0);
	setForceMax(0.1);
	m_locationsNbMax = 200;
	mp_particlePath = 0;
	for (int i=0;i<m_locationsNbMax;i++)
	{
		m_polyline.addVertex( ofVec2f(0.0f,0.0f) );
	}
}

//--------------------------------------------------------------
void BoidOrbit::setSpeedMinMax(float speedMin, float speedMax)
{
	maxspeed = ofRandom(speedMin, speedMax);
}

//--------------------------------------------------------------
void BoidOrbit::setForceMax(float forceMax)
{
	maxforce = forceMax;
}



//--------------------------------------------------------------
void BoidOrbit::flock(vector<Boid*>& boids)
{

	if (mp_animation->m_boidsSeparation>0.0f)
	{
		ofVec2f sep = separate(boids);   // Separation
		sep *= mp_animation->m_boidsSeparation;
		applyForce(sep);
	}
	
	if (mp_animation->m_boidsAlignement>0.0f)
	{
		ofVec2f ali = align(boids);      // Alignment
		ali *= mp_animation->m_boidsAlignement;
		applyForce(ali);
	}

	if (mp_animation->m_boidsCohesion>0.0f)
	{
		ofVec2f coh = cohesion(boids);   // Cohesion
		coh *= mp_animation->m_boidsCohesion;
		applyForce(coh);
	}
}

//--------------------------------------------------------------
void BoidOrbit::follow(ofVec2f& target, vector<Boid*>& boids)
{
	applyForce( seek(target) );
	flock(boids);
	update(0.0f);

	m_locations.push_back( location );
	if (m_locations.size()>m_locationsNbMax)
	{
		m_locations.erase(m_locations.begin());
	}
}


//--------------------------------------------------------------
void BoidOrbit::draw()
{
	int nbPoints = m_locations.size();
	for (int i=0;i<nbPoints;i++)
	{
		m_polyline[i].set(m_locations[i]);
	}

	m_polyline.draw();
}


//--------------------------------------------------------------
ParticlePath::ParticlePath()
{
	mp_orbitA = 0;
	mp_orbitB = 0;
	
	m_speed = 50;
}


//--------------------------------------------------------------
void ParticlePath::setSegment(int indexA, ParticleOrbit* pA, int indexB, ParticleOrbit* pB)
{
	m_indexA = indexA;
	mp_orbitA = pA;
	m_A = mp_orbitA->getPoints()[indexA];
	
	m_indexB = indexB;
	mp_orbitB = pB;
	m_B = mp_orbitB->getPoints()[indexB];

	m_u = (m_B-m_A).normalized();
	m_isReachB = false;

	m_pos = m_A;
}

//--------------------------------------------------------------
void ParticlePath::update(float dt)
{
	if (mp_orbitA == 0) return;

	m_pos += m_u*m_speed*dt;

	if ( m_pos.distance(m_A) >=  m_A.distance(m_B))
	{
		if (mp_orbitB == mp_orbitA)
		{
			int indexA = m_indexB;
			int indexB = (m_indexA+1)%mp_orbitA->getPoints().size();
			setSegment(indexA,mp_orbitA,indexB,mp_orbitB);
		}

	}
}


//--------------------------------------------------------------
ParticleOrbitEllipse::ParticleOrbitEllipse()
{
	setRadius( ofVec2f(100.0,50.0) );
	setOffset( ofVec3f(0.0,0.0,0.0) );
	setRotation(20.0f);
}

//--------------------------------------------------------------
void ParticleOrbitEllipse::computePoints()
{
	m_points.clear();

	ofVec2f pointEllipse;
	ofVec3f point;
	float aR = 0.0f;
	for (float angle = 0.0f; angle < 360.0f; angle+=30.0f)
	{
		aR = ofDegToRad(angle);
		pointEllipse.set(
							m_radius.x * cos(aR),
							m_radius.y * sin(aR)
						);

		float rotR = ofDegToRad(m_rot);
		point.set(
					m_center.x+m_offset.x + pointEllipse.x*cos(rotR) - pointEllipse.y*sin(rotR),
					m_center.y+m_offset.y + pointEllipse.x*sin(rotR) + pointEllipse.y*cos(rotR)
		);
		
		m_points.push_back( point );
	}

}

//--------------------------------------------------------------
void ParticleOrbitEllipse::setOffset(ofVec3f offset)
{
	m_offset = offset;
	computePoints();
}

//--------------------------------------------------------------
void ParticleOrbitEllipse::setRadius(ofVec2f radius)
{
	m_radius = radius;
	computePoints();
}

//--------------------------------------------------------------
void ParticleOrbitEllipse::setRotation(float rot)
{
	m_rot = rot;
	computePoints();
}


//--------------------------------------------------------------
AnimationOrbit::AnimationOrbit(string name) : Animation(name)
{
	mp_testParticle = new ParticlePath();
/*
	int nbPaths = 3;
	for (int i=0;i<nbPaths;i++)
	{
		ParticlePath* pParticlePath = new ParticlePath();
		m_particlePaths.push_back( pParticlePath );
	}
*/

	for (int i=0;i<100;i++)
		m_boids.push_back( new BoidOrbit(this,0,0) );


	m_isDrawDebug 		= false;
	m_isFrameBlending	= false;
	m_frameBlending		= 0.25f;

	m_boidsSeparation 	= 1.5f;
	m_boidsCohesion		= 1.0f;
	m_boidsAlignement	= 1.0f;
	m_boidsMaxSpeedMin	= 1.0f;
	m_boidsMaxSpeedMax	= 2.0f;

	m_rotationForms		= 20.0f;
	m_widthForms		= 200.0f;
	m_heightForms		= 100.0f;
	
	mp_labelDeviceId	= 0;
	mp_sliderFormRot	= 0;
	mp_sliderFormWidth	= 0;
	mp_sliderFormHeight	= 0;
	mp_deviceCurrent	= 0;
}

//--------------------------------------------------------------
AnimationOrbit::~AnimationOrbit()
{
	for (m_orbitsIt = m_orbits.begin(); m_orbitsIt != m_orbits.end(); ++m_orbitsIt)
	{
		delete m_orbitsIt->second;
	}

	delete mp_testParticle;
}
 
//--------------------------------------------------------------
void AnimationOrbit::VM_enter()
{
}

//--------------------------------------------------------------
void AnimationOrbit::VM_update(float dt)
{
	if (mp_testParticle)
	{
		mp_testParticle->update(dt);

		vector<Boid*>::iterator it = m_boids.begin();
		BoidOrbit* pBoid=0;
		for ( ; it != m_boids.end(); ++it)
		{
			pBoid = (BoidOrbit*) *it;
			pBoid->follow(mp_testParticle->m_pos, m_boids);
		}
	}

	// Look for change on Device
	if (mp_deviceCurrent != getDeviceCurrent())
	{
		mp_deviceCurrent = getDeviceCurrent();
		updateUI();
	}

}

//--------------------------------------------------------------
void AnimationOrbit::VM_draw(float w, float h)
{
	if (m_isFrameBlending)
	{
		ofPushStyle();
		ofFill();
		ofSetColor(0,m_frameBlending*255.0f);
		ofRect(0,0,w,h);
		ofPopStyle();
	}
	else{
		ofBackground(0);
	}


	if (m_isDrawDebug)
	{
		ofPushStyle();
		for (m_orbitsIt = m_orbits.begin(); m_orbitsIt != m_orbits.end(); ++m_orbitsIt)
		{
			ParticleOrbit* pOrbit = m_orbitsIt->second;
			Device* pDevice = getDevice( m_orbitsIt->first );
		
			ofFill();
			ofSetColor(255,0,0);
			ofPushMatrix();
				drawOrbit(pOrbit);
			ofPopMatrix();
		}
	}

	if (mp_testParticle)
	{
		if (m_isDrawDebug)
		{
			ofSetColor(0,0,255);
			ofEllipse(mp_testParticle->m_pos, 5,5);
		}
		
		ofSetColor(0,255,255);
		vector<Boid*>::iterator it = m_boids.begin();
		for ( ; it != m_boids.end(); ++it)
		{
			(*it)->draw();
			if (m_isDrawDebug)
				ofEllipse((*it)->location, 5,5);
		}


	}

	ofPopStyle();
}

//--------------------------------------------------------------
void AnimationOrbit::drawOrbit(ParticleOrbit* pOrbit)
{
	if (pOrbit==0) return;

	ofPushStyle();
	ofFill();
	ofSetColor(255,0,0);
	vector<ofVec3f>& points = pOrbit->getPoints();
	int nbPoints = points.size();
	for (int i=0;i<nbPoints;i++)
	{
		ofLine( points[i].x, points[i].y, points[(i+1)%nbPoints].x, points[(i+1)%nbPoints].y );
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void AnimationOrbit::VM_exit()
{
}

//--------------------------------------------------------------
void AnimationOrbit::onNewPacket(DevicePacket*, string deviceId, float x, float y)
{
	// Create Orbit for device
	ParticleOrbit* pOrbit = getOrbitForDevice(deviceId);
	if (pOrbit == 0)
	{
		// Orbit for device 01
		ParticleOrbitEllipse* pOrbitEllipse =  new ParticleOrbitEllipse();
		if (deviceId == "deviceEchoSimulator01" || deviceId == "chambreEcho_001")
		{
			pOrbitEllipse->setRotation( -m_rotationForms );


		}
		// Orbit for device 02
		else if (deviceId == "deviceEchoSimulator02" || deviceId == "chambreEcho_002"){
			pOrbitEllipse->setRotation( m_rotationForms );
		}
		
		// Set center
		pOrbitEllipse->setCenter( ofVec3f(x,y,0.0f) );

		// Save in map
		m_orbits[deviceId] = pOrbitEllipse;

		pOrbit = pOrbitEllipse;
	}

	// Position of deviceId changed ?
	if (pOrbit && updateDevicePosition(deviceId,x,y))
	{
		pOrbit->setCenter( ofVec3f(x,y) );
		// TEMP
		mp_testParticle->setSegment(0, pOrbit, 1, pOrbit);
	}
	
	
}

//--------------------------------------------------------------
void AnimationOrbit::createUICustom()
{
    if (mp_UIcanvas)
    {
		int widthDefault = 320;

	    mp_UIcanvas->addWidgetDown	( new ofxUILabel("Drawing", OFX_UI_FONT_SMALL) );
    	mp_UIcanvas->addWidgetDown	( new ofxUISpacer(widthDefault, 1));

        mp_UIcanvas->addToggle("draw_debug",	&m_isDrawDebug);
        mp_UIcanvas->addToggle("trace",			&m_isFrameBlending);
        mp_UIcanvas->addSlider("trace_level", 	0.0f, 1.0f, &m_frameBlending);


	    mp_UIcanvas->addWidgetDown	( new ofxUILabel("Flocking", OFX_UI_FONT_SMALL) );
    	mp_UIcanvas->addWidgetDown	( new ofxUISpacer(widthDefault, 1));

        mp_UIcanvas->addSlider("separation", 	0.0f, 1.5f, &m_boidsSeparation);
        mp_UIcanvas->addSlider("cohesion", 		0.0f, 1.5f, &m_boidsCohesion);
        mp_UIcanvas->addSlider("alignement", 	0.0f, 1.5f, &m_boidsAlignement);
        mp_UIcanvas->addSlider("speed_min", 	1.0f, 3.0f, 1.0f);
        mp_UIcanvas->addSlider("speed_max", 	1.0f, 3.0f, 2.0f);
        mp_UIcanvas->addSlider("force_max", 	0.0f, 0.2f, 0.1f);

	    mp_labelDeviceId = new ofxUILabel("Forms", OFX_UI_FONT_SMALL);
		mp_UIcanvas->addWidgetDown	( mp_labelDeviceId );
    	mp_UIcanvas->addWidgetDown	( new ofxUISpacer(widthDefault, 1));
		mp_sliderFormRot = mp_UIcanvas->addSlider("rotation_forms", 	-90.0f, 90.0f, &m_rotationForms);
		mp_sliderFormWidth = mp_UIcanvas->addSlider("width_forms", 		10.0f, 500.0f, &m_widthForms);
		mp_sliderFormHeight = mp_UIcanvas->addSlider("height_forms", 	10.0f, 500.0f, &m_heightForms);
    }
}

//--------------------------------------------------------------
ParticleOrbit* AnimationOrbit::getOrbitForDevice(Device* pDevice)
{
	if (pDevice)
		return getOrbitForDevice( pDevice->m_id );
	return 0;
}

//--------------------------------------------------------------
ParticleOrbit* AnimationOrbit::getOrbitForDevice(string deviceId)
{
	if (m_orbits.find(deviceId) != m_orbits.end())
		return m_orbits[deviceId];
	return 0;
}

//--------------------------------------------------------------
void AnimationOrbit::guiEvent(ofxUIEventArgs &e)
{
	Animation::guiEvent(e);
    string name = e.widget->getName();
	
    if (name == "speed_min" || name == "speed_max")
    {
		ofxUISlider* pSliderSpeedMin = (ofxUISlider*) mp_UIcanvas->getWidget("speed_min");
		ofxUISlider* pSliderSpeedMax = (ofxUISlider*) mp_UIcanvas->getWidget("speed_max");
		if (pSliderSpeedMin && pSliderSpeedMax)
		{
			vector<Boid*>::iterator it = m_boids.begin();
			BoidOrbit* pBoid=0;
			for ( ; it != m_boids.end(); ++it)
			{
				pBoid = (BoidOrbit*) *it;
				pBoid->setSpeedMinMax(pSliderSpeedMin->getScaledValue(), pSliderSpeedMax->getScaledValue());
			}
		}
	}
    else if (name == "force_max")
	{
		ofxUISlider* pSliderForceMax = (ofxUISlider*) e.widget;

		vector<Boid*>::iterator it = m_boids.begin();
		BoidOrbit* pBoid=0;
		for ( ; it != m_boids.end(); ++it)
		{
			pBoid = (BoidOrbit*) *it;
			pBoid->setForceMax(pSliderForceMax->getScaledValue());
		}
	}
    else if (name == "rotation_forms")
	{
		ofxUISlider* pSliderFormRot = (ofxUISlider*) e.widget;
	
		ParticleOrbitEllipse* pOrbitEllipse = (ParticleOrbitEllipse*) getOrbitForDevice(getDeviceCurrent());
		if(pOrbitEllipse) pOrbitEllipse->setRotation( pSliderFormRot->getScaledValue() );
		
	}
    else if (name == "width_forms")
	{
		ParticleOrbitEllipse* pOrbitEllipse = (ParticleOrbitEllipse*) getOrbitForDevice(getDeviceCurrent());
		if(pOrbitEllipse) pOrbitEllipse->setWidth( ((ofxUISlider*) e.widget)->getScaledValue() );
	}
    else if (name == "height_forms")
	{
		ParticleOrbitEllipse* pOrbitEllipse = (ParticleOrbitEllipse*) getOrbitForDevice(getDeviceCurrent());
		if(pOrbitEllipse) pOrbitEllipse->setHeight( ((ofxUISlider*) e.widget)->getScaledValue() );
	}
}

//--------------------------------------------------------------
void AnimationOrbit::updateUI()
{
	ParticleOrbitEllipse* pOrbitEllipse = (ParticleOrbitEllipse*) getOrbitForDevice( getDeviceCurrent() );

	if (mp_labelDeviceId){
		Device* pDeviceCurrent = getDeviceCurrent();
		if (pDeviceCurrent) mp_labelDeviceId->setLabel("Forms for "+pDeviceCurrent->m_id);
	}
	
	if (mp_sliderFormRot && pOrbitEllipse)			mp_sliderFormRot->setValue( pOrbitEllipse->getRotation() );
	if (mp_sliderFormWidth && pOrbitEllipse)		mp_sliderFormWidth->setValue( pOrbitEllipse->getRadius().x );
	if (mp_sliderFormHeight && pOrbitEllipse)		mp_sliderFormHeight->setValue( pOrbitEllipse->getRadius().y );

}

