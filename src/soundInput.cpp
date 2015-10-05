//
//  soundInput.cpp
//  murmur
//
//  Created by Julien on 15/04/13.
//
//

#include "soundInput.h"
#include "Sample.h"



//--------------------------------------------------------------
SoundInput::~SoundInput()
{
	//delete mp_soundStreamInput;
	delete m_sampleData;
}

//--------------------------------------------------------------
void SoundInput::setup(int deviceId, int nChannels)
{
	m_useRawVol = false;
    m_smoothedVol = 0.0f;
    m_scaledVol = 0.0f;
    m_volHistoryNb = 400;
    m_heightDraw = 70.0f;
    m_volEmpiricalMax = 0.10f;
	
 
	//mp_soundStreamInput = new ofSoundStream();
	m_soundStreamInput.listDevices();
    
	//if you want to set a different device id
    //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
    if (deviceId>=0)
        m_soundStreamInput.setDeviceID(deviceId);

    m_bufferSize = 256; // 256 to be compliant with FFT
    m_nbChannels = nChannels;

	m_soundStreamInput.setup(ofGetAppPtr(), 0, m_nbChannels, 44100, m_bufferSize, 4);
#if SOUNDINPUT_USE_FFT
    if (nChannels==1 /*&& m_bufferSize == m_fft.getNoOfBands()*/){
//        m_fft.setNoOfBands(20);
//        m_fft.setThreshold(0.8);
        m_fft.setup(&m_soundStreamInput);
    }
    else
        printf(">>> FFT, nChannels should be 1 & m_bufferSize should be %d\n", m_fft.getNoOfBands());
#endif
    
    initAudioBuffers();
    setVolHistorySize(400);

    m_soundStreamInput.start();
	mute(false);
	setInputVolumeModulate(1.0f);
	
	mp_sample = 0;
	m_sampleData = 0;
	m_sampleVolume = 0.35f;
}

//--------------------------------------------------------------
void SoundInput::setup(int nChannels)
{
	m_useRawVol = false;
    m_smoothedVol = 0.0f;
    m_scaledVol = 0.0f;
    m_volHistoryNb = 400;
    m_heightDraw = 70.0f;
    m_volEmpiricalMax = 0.10f;
 
    m_bufferSize = 256; // 256 to be compliant with FFT
    m_nbChannels = nChannels;
	
	mp_sample=0;

#if SOUNDINPUT_USE_FFT
    if (nChannels==1 /*&& m_bufferSize == m_fft.getNoOfBands()*/){
//        m_fft.setNoOfBands(20);
//        m_fft.setThreshold(0.8);
//        m_fft.setup(&m_soundStreamInput);
    }
    else
        printf(">>> FFT, nChannels should be 1 & m_bufferSize should be %d\n", m_fft.getNoOfBands());
#endif
    
    initAudioBuffers();
    setVolHistorySize(400);
	mute(false);
	setInputVolumeModulate(1.0f);
}

//--------------------------------------------------------------
float SoundInput::getVolHistoryLast()
{
	if (m_volHistory.size()>0)
		return m_volHistory[0]; // we use insert to save values in this array
	return 0.0f;
}

//--------------------------------------------------------------
void SoundInput::setVolume(float vol)
{
	if (m_isStopInput == true)
	{
		m_smoothedVol = vol;
	}
}

//--------------------------------------------------------------
void SoundInput::setVolHistorySize(int nb)
{
    m_volHistoryNb = nb;
	m_volHistory.clear();
    m_volHistory.assign(m_volHistoryNb, 0.0f);
    m_volHistoryMeanFiltered = 0.0f;
}

//--------------------------------------------------------------
void SoundInput::setVolHistoryValue(int i, float value)
{
    if (i<m_volHistory.size())
    {
        m_volHistory[i] = value;
    }
}

//--------------------------------------------------------------
float SoundInput::getVolHistoryMean()
{
    return m_volHistoryMean;
}

//--------------------------------------------------------------
float SoundInput::getVolHistoryMeanFiltered()
{
    return m_volHistoryMeanFiltered;
}

//--------------------------------------------------------------
void SoundInput::updateVolHistoryMean()
{
    int nb = m_volHistory.size();
    m_volHistoryMean = 0.0f;
    for (int i=0; i<nb ; i++)
        m_volHistoryMean+= m_volHistory[i];
    m_volHistoryMean /= (float)nb;
    m_volHistoryMeanFiltered += (m_volHistoryMean-m_volHistoryMeanFiltered)*0.2f;
}


//--------------------------------------------------------------
void SoundInput::update()
{
	//lets scale the vol up to a 0-1 range
	m_scaledVol = ofMap(m_smoothedVol, 0.0, m_volEmpiricalMax, 0.0, 1.0, true); // TODO : adjust ?
	
	if (m_isMute)
	{
		m_scaledVol = 0.0f;
	}

   // printf("SoundInput::update vol=%.2f-", m_scaledVol);
	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( m_volHistory.size() >= m_volHistoryNb ){
		m_volHistory.erase(m_volHistory.end()-1, m_volHistory.end());
	}

	//lets record the volume into an array
	m_volHistory.insert( m_volHistory.begin(), m_scaledVol );

    // Compute mean
    updateVolHistoryMean();
    
    // FFT
#if SOUNDINPUT_USE_FFT
    m_fft.update();
#endif
}

//--------------------------------------------------------------
void SoundInput::drawVolume(float x, float y)
{
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(x,y,0);

    ofSetColor(255,70);

    float width = (float)m_volHistory.size();
    
    ofNoFill();
    ofRect(-width, 0, width, m_heightDraw);
    
    //printf("drawVolume %.3f-", m_heightDraw);

    
    ofSetColor(255,255);
    ofBeginShape();
    for (int i = 0; i < m_volHistory.size(); i++){
        if( i == 0 ) ofVertex(-width+i, m_heightDraw);
        
        ofVertex(-width+i, m_heightDraw- m_volHistory[i] * m_heightDraw);
        
        if( i == m_volHistory.size() -1 ) ofVertex(-width+i, m_heightDraw);
    }
    ofEndShape(false);
    
    ofSetColor(255,0,0,100);
    float yMean = m_heightDraw*(1.0f-m_volHistoryMeanFiltered);
    ofLine(-width,yMean, 0, yMean);

    
#if SOUNDINPUT_USE_FFT
    
    ofSetColor(255);
    m_fft.draw(0, 0, width, m_heightDraw);

    ofNoFill();
    ofSetColor(255);
    ofRect(0, 0, width, m_heightDraw);
#endif
    
    
    ofPopMatrix();
    ofPopStyle();

    

}


//--------------------------------------------------------------
void SoundInput::initAudioBuffers()
{
    if (m_nbChannels == 1)
    {
        m_mono.assign(m_bufferSize, 0.0f);
    }
    else
    if (m_nbChannels == 2)
    {
        m_left.assign(m_bufferSize, 0.0f);
        m_right.assign(m_bufferSize, 0.0f);
    }
}

//--------------------------------------------------------------
void SoundInput::setSample(Sample* pSample)
{
	mp_sample = pSample;
	
	if (mp_sample==0)
	{
		stopInput(false);
		
		delete m_sampleData;
		m_sampleData = 0;
	}else
	{
		stopInput();

		mp_sample->setLooping(false);
		mp_sample->play();
	}
}

//--------------------------------------------------------------
void SoundInput::audioIn(float * input, int bufferSize, int nChannels)
{
	if (m_isStopInput == false)
	{
		processAudio(input,bufferSize,nChannels);
	}
	else
	{
		// lock the mutex from audio thread
		Sample::lock();
		if (mp_sample)
		{
			// Feed audioIn with our own data from sample
			int nbChannelsSample = mp_sample->getChannels();
		
			if (m_sampleData==0)
				m_sampleData = new float[bufferSize * nbChannelsSample];
				
			for (int i=0; i<bufferSize ; i++)
			{
			   if (nbChannelsSample==1){
			   		m_sampleData[i] = (float)mp_sample->update();
			   }
			   else if (nbChannelsSample==2){
				   m_sampleData[2*i] = (float)mp_sample->update() * m_sampleVolume;
				   m_sampleData[2*i+1] = (float)mp_sample->update() * m_sampleVolume;
			   }
			}
			
			// Process
			processAudio(m_sampleData,bufferSize,nbChannelsSample);

		}
		Sample::unlock();
	}
}

//--------------------------------------------------------------
void SoundInput::processAudio(float * input, int bufferSize, int nChannels)
{
	float curVol = 0.0;
    int numCounted = 0;

    if (m_nbChannels==1)
    {

		for (int i = 0; i < bufferSize; i++){
			m_mono[i] = input[i];
			curVol += m_mono[i]*m_mono[i];
			numCounted++;
		}

        //this is how we get the mean of rms :)
        curVol /= (float)numCounted;
        
        // this is how we get the root of rms :)
        curVol = sqrt( curVol );

		if (m_useRawVol)
		{
		 	m_smoothedVol = curVol;
		}
		else
		{
	        m_smoothedVol *= 0.93;
    	    m_smoothedVol += 0.07 * curVol;
	 	}
		
		m_smoothedVol *= m_inputVolumeModulate;
		 
#if SOUNDINPUT_USE_FFT
        m_fft.audioIn(input, bufferSize, nChannels);
#endif
        
    }
    else
    if (m_nbChannels==2)
    {
        // samples are "interleaved"
        
        for (int i = 0; i < bufferSize; i++)
        {
            m_left[i]	= input[i*2]*0.5;
            m_right[i]	= input[i*2+1]*0.5;
            
            curVol += m_left[i] * m_left[i];
            curVol += m_right[i] * m_right[i];
            numCounted+=2;
        }
        
        //this is how we get the mean of rms :)
        curVol /= (float)numCounted;
        
        // this is how we get the root of rms :)
        curVol = m_inputVolumeModulate*sqrt( curVol );
        
        m_smoothedVol *= 0.93;
        m_smoothedVol += 0.07 * curVol;

		m_smoothedVol *= m_inputVolumeModulate;
     
        //bufferCounter++;
    }
}



