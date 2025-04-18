#include "ofSoundStream.h"
#include "ofAppRunner.h"
#include "ofLog.h"

//------------------------------------------------ soundstream
// check if any soundstream api is defined from the compiler
#if !defined(OF_SOUNDSTREAM_RTAUDIO) && !defined(OF_SOUNDSTREAM_ANDROID) && !defined(OF_SOUNDSTREAM_IOS) && !defined(OF_SOUNDSTREAM_EMSCRIPTEN)
	#if defined(TARGET_LINUX) || defined(TARGET_WIN32) || defined(TARGET_OSX)
		#define OF_SOUNDSTREAM_RTAUDIO
	#elif defined(TARGET_ANDROID)
		#define OF_SOUNDSTREAM_ANDROID
	#elif defined(TARGET_OF_IOS)
		#define OF_SOUNDSTREAM_IOS
	#elif defined(TARGET_EMSCRIPTEN)
		#define OF_SOUNDSTREAM_EMSCRIPTEN
	#endif
#endif

#if defined(OF_SOUND_PLAYER_FMOD)
#include "ofSoundPlayer.h"
#include "ofFmodSoundPlayer.h"
#endif

#ifdef OF_SOUNDSTREAM_RTAUDIO
#include "ofRtAudioSoundStream.h"
#define OF_SOUND_STREAM_TYPE ofRtAudioSoundStream
#elif defined(OF_SOUNDSTREAM_ANDROID)
#include "ofxAndroidSoundStream.h"
#define OF_SOUND_STREAM_TYPE ofxAndroidSoundStream
#elif defined(OF_SOUNDSTREAM_IOS)
#include "ofxiOSSoundStream.h"
#define OF_SOUND_STREAM_TYPE ofxiOSSoundStream
#elif defined(OF_SOUNDSTREAM_EMSCRIPTEN)
#include "ofxEmscriptenSoundStream.h"
#define OF_SOUND_STREAM_TYPE ofxEmscriptenSoundStream
#endif

void ofFmodSetBuffersize(unsigned int bs);
float * ofFmodSoundGetSpectrum(int nBands);

//FIXME: this is needed to make video work on emscripten
//See: https://github.com/openframeworks/openFrameworks/issues/7377 
#ifdef OF_SOUNDSTREAM_EMSCRIPTEN
    namespace{
        ofSoundStream systemSoundStream;
    }
    #define OF_SYSTEM_SS systemSoundStream
#else
    namespace{
        ofSoundStream &getSystemSoundStream() {
            static ofSoundStream _;
            return _;
        }
    }
    #define OF_SYSTEM_SS getSystemSoundStream()
#endif


using std::shared_ptr;
using std::vector;

//------------------------------------------------------------
bool ofSoundStreamSettings::setInDevice(const ofSoundDevice & device){
	if(api!=ofSoundDevice::UNSPECIFIED && device.api!=api){
		ofLogWarning("ofSoundStreamSettings") << "Setting IN device with api: " << toString(device.api) << " will override the previously set: " << toString(api);
	}
	api = device.api;
	inDevice = device;
	inDevice.api = api;
	return true;
}

//------------------------------------------------------------
bool ofSoundStreamSettings::setOutDevice(const ofSoundDevice & device){
	if(api!=ofSoundDevice::UNSPECIFIED && device.api!=api){
		ofLogWarning("ofSoundStreamSettings") << "Setting OUT device with api: " << toString(device.api) << " will override the previously set: " << toString(api);
	}
	api = device.api;
	outDevice = device;
	outDevice.api = api;
	return true;
}

//------------------------------------------------------------
bool ofSoundStreamSettings::setApi(ofSoundDevice::Api api){
	if(api!=ofSoundDevice::UNSPECIFIED && inDevice.deviceID!=-1 && inDevice.api != api){
		ofLogError("ofSoundStreamSettings") << "Setting API after setting IN device with api: " << toString(inDevice.api) << " won't do anything";
		return false;
	}
	if(api!=ofSoundDevice::UNSPECIFIED && outDevice.deviceID!=-1 && outDevice.api != api){
		ofLogError("ofSoundStreamSettings") << "Setting API after setting OUT device with api: " << toString(outDevice.api) << " won't do anything";
		return false;
	}
	this->api = api;
	return true;
}

//------------------------------------------------------------
const ofSoundDevice * ofSoundStreamSettings::getInDevice() const{
	return inDevice.deviceID==-1 ? nullptr : &inDevice;
}

//------------------------------------------------------------
const ofSoundDevice * ofSoundStreamSettings::getOutDevice() const{
	return outDevice.deviceID==-1 ? nullptr : &outDevice;
}

//------------------------------------------------------------
ofSoundDevice::Api ofSoundStreamSettings::getApi() const{
	return api;
}


//------------------------------------------------------------
void ofSoundStreamSetup(int nOutputChannels, int nInputChannels, ofBaseApp * appPtr){
	if( appPtr == nullptr ){
		appPtr = ofGetAppPtr();
	}
	ofSoundStreamSettings settings;
	settings.numOutputChannels = nOutputChannels;
	settings.numInputChannels = nInputChannels;
	settings.setOutListener(appPtr);
	settings.setInListener(appPtr);
	ofSoundStreamSetup(settings);
}

//------------------------------------------------------------
void ofSoundStreamSetup(int nOutputChannels, int nInputChannels, int sampleRate, int bufferSize, int nBuffers){
	ofSoundStreamSettings settings;
	settings.numOutputChannels = nOutputChannels;
	settings.numInputChannels = nInputChannels;
	settings.setOutListener(ofGetAppPtr());
	settings.setInListener(ofGetAppPtr());
	settings.numBuffers = nBuffers;
	settings.sampleRate = sampleRate;
	settings.bufferSize = bufferSize;
	ofSoundStreamSetup(settings);
}

//------------------------------------------------------------
void ofSoundStreamSetup(int nOutputChannels, int nInputChannels, ofBaseApp * appPtr, int sampleRate, int bufferSize, int nBuffers){
	ofSoundStreamSettings settings;
	settings.numOutputChannels = nOutputChannels;
	settings.numInputChannels = nInputChannels;
	settings.setOutListener(appPtr);
	settings.setInListener(appPtr);
	settings.numBuffers = nBuffers;
	settings.sampleRate = sampleRate;
	settings.bufferSize = bufferSize;
	ofSoundStreamSetup(settings);
}

//------------------------------------------------------------
void ofSoundStreamSetup(ofSoundStreamSettings & settings) {
    OF_SYSTEM_SS.setup(settings);
}

//------------------------------------------------------------
void ofSoundStreamStop(){
    OF_SYSTEM_SS.stop();
}

//------------------------------------------------------------
void ofSoundStreamStart(){
    OF_SYSTEM_SS.start();
}

//------------------------------------------------------------
void ofSoundStreamClose(){
    OF_SYSTEM_SS.close();
}

//------------------------------------------------------------
vector<ofSoundDevice> ofSoundStreamListDevices(){
	vector<ofSoundDevice> deviceList = OF_SYSTEM_SS.getDeviceList();
	ofLogNotice("ofSoundStreamListDevices") << std::endl << deviceList;
	return deviceList;
}

//------------------------------------------------------------
ofSoundStream::ofSoundStream(){
	#ifdef OF_SOUND_STREAM_TYPE
		setSoundStream(std::make_shared<OF_SOUND_STREAM_TYPE>());
	#endif
}

//------------------------------------------------------------
void ofSoundStream::setSoundStream(shared_ptr<ofBaseSoundStream> soundStreamPtr){
	soundStream = soundStreamPtr;
}

//------------------------------------------------------------
shared_ptr<ofBaseSoundStream> ofSoundStream::getSoundStream(){
	return soundStream;
}

//------------------------------------------------------------
vector<ofSoundDevice> ofSoundStream::getDeviceList(ofSoundDevice::Api api) const{
	if( soundStream ){
		return soundStream->getDeviceList(api);
	} else {
		return vector<ofSoundDevice>();
	}
}

//------------------------------------------------------------
vector<ofSoundDevice> ofSoundStream::listDevices() const{
	vector<ofSoundDevice> deviceList = getDeviceList();
	ofLogNotice("ofSoundStream::listDevices") << std::endl << deviceList;
	return deviceList;
}

//------------------------------------------------------------
void ofSoundStream::printDeviceList()  const{
	if( soundStream ) {
		soundStream->printDeviceList();
	}
}

//------------------------------------------------------------
void ofSoundStream::setDeviceID(int deviceID){
	if( soundStream ){
		tmpDeviceId = deviceID;
	}
}

//------------------------------------------------------------
void ofSoundStream::setDevice(const ofSoundDevice &device) {
    if( soundStream ){
        tmpDeviceId = device.deviceID;
    }
}

//------------------------------------------------------------
bool ofSoundStream::setup(const ofSoundStreamSettings & settings)
{
	if (soundStream) {
#if defined(OF_SOUND_PLAYER_FMOD)
		ofFmodSetBuffersize(settings.bufferSize);
#endif
		return soundStream->setup(settings);
	}
	return false;
}

//------------------------------------------------------------
bool ofSoundStream::setup(ofBaseApp * app, int outChannels, int inChannels, int sampleRate, int bufferSize, int nBuffers){
	if( soundStream ){
		ofSoundStreamSettings settings;
		settings.setInListener(app);
		settings.setOutListener(app);
		settings.numOutputChannels = outChannels;
		settings.numInputChannels = inChannels;
		settings.sampleRate = sampleRate;
		settings.bufferSize = bufferSize;
		settings.numBuffers = nBuffers;
		if(tmpDeviceId!=-1){
			ofSoundDevice device;
			device.deviceID = tmpDeviceId;
			settings.setInDevice(device);
			settings.setOutDevice(device);
		}
		return soundStream->setup(settings);
	}
	return false;
}

//------------------------------------------------------------
bool ofSoundStream::setup(int outChannels, int inChannels, int sampleRate, int bufferSize, int nBuffers){
	if( soundStream ){
		ofSoundStreamSettings settings;
		settings.setInListener(ofGetAppPtr());
		settings.setOutListener(ofGetAppPtr());
		settings.numOutputChannels = outChannels;
		settings.numInputChannels = inChannels;
		settings.sampleRate = sampleRate;
		settings.bufferSize = bufferSize;
		settings.numBuffers = nBuffers;
		if(tmpDeviceId!=-1){
			ofSoundDevice device;
			device.deviceID = tmpDeviceId;
			settings.setInDevice(device);
			settings.setOutDevice(device);
		}
		return soundStream->setup(settings);
	}
	return false;
}

//------------------------------------------------------------
void ofSoundStream::setInput(ofBaseSoundInput * soundInput){
	if( soundStream ){
		soundStream->setInput(soundInput);
	}
}

//------------------------------------------------------------
void ofSoundStream::setInput(ofBaseSoundInput &soundInput){
	setInput(&soundInput);
}

//------------------------------------------------------------
void ofSoundStream::setOutput(ofBaseSoundOutput * soundOutput){
	if( soundStream ){
		soundStream->setOutput(soundOutput);
	}
}

//------------------------------------------------------------
void ofSoundStream::setOutput(ofBaseSoundOutput &soundOutput){
	setOutput(&soundOutput);
}

//------------------------------------------------------------
void ofSoundStream::start(){
	if( soundStream ){
		soundStream->start();
	}
}

//------------------------------------------------------------
void ofSoundStream::stop(){
	if( soundStream ){
		soundStream->stop();
	}
}

//------------------------------------------------------------
void ofSoundStream::close(){
	if( soundStream ){
		soundStream->close();
	}
}

//------------------------------------------------------------
uint64_t ofSoundStream::getTickCount() const{
	if( soundStream ){
		return soundStream->getTickCount();
	}
	return 0;
}

//------------------------------------------------------------
int ofSoundStream::getNumInputChannels() const{
	if( soundStream ){
		return soundStream->getNumInputChannels();
	}
	return 0;
}

//------------------------------------------------------------
int ofSoundStream::getNumOutputChannels() const{
	if( soundStream ){
		return soundStream->getNumOutputChannels();
	}
	return 0;
}

//------------------------------------------------------------
int ofSoundStream::getSampleRate() const{
	if( soundStream ){
		return soundStream->getSampleRate();
	}
	return 0;
}

//------------------------------------------------------------
int ofSoundStream::getBufferSize() const{
	if( soundStream ){
		return soundStream->getBufferSize();
	}
	return 0;
}

//------------------------------------------------------------
vector<ofSoundDevice> ofSoundStream::getMatchingDevices(const std::string& name, unsigned int inChannels, unsigned int outChannels, ofSoundDevice::Api api) const {
	vector<ofSoundDevice> devs = getDeviceList(api);
	vector<ofSoundDevice> hits;
	
	for(size_t i = 0; i < devs.size(); i++) {
		bool nameMatch = devs[i].name.find(name) != std::string::npos;
		bool inMatch = (inChannels == UINT_MAX) || (devs[i].inputChannels == inChannels);
		bool outMatch = (outChannels == UINT_MAX) || (devs[i].outputChannels == outChannels);
		
		if(nameMatch && inMatch && outMatch) {
			hits.push_back(devs[i]);
		}
	}
	
	return hits;
}
