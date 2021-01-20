//====================================================================================
//File [3] - Custom Mixer

#include <JuceHeader.h>
#include "InputCachingMixerAudioSource.h"

using namespace std;
using namespace juce;

//==============================================================================
InputCachingMixerAudioSource::InputCachingMixerAudioSource()
: tempBuffer (2, 0),
cacheBuffer (2, 0),
currentSampleRate (0.0),
bufferSizeExpected (0)
{
}

InputCachingMixerAudioSource::~InputCachingMixerAudioSource()
{
removeAllInputs();
}

//==============================================================================
void InputCachingMixerAudioSource::addInputSource (AudioSource* input, const bool deleteWhenRemoved)
{
if (input != 0 && ! inputs.contains (input))
{
lock.enter();
double localRate = currentSampleRate;
int localBufferSize = bufferSizeExpected;
lock.exit();

    if (localRate != 0.0)
        input->prepareToPlay (localBufferSize, localRate);

    const ScopedLock sl (lock);

    inputsToDelete.setBit (inputs.size(), deleteWhenRemoved);
    inputs.add (input);
}
}

void InputCachingMixerAudioSource::removeInputSource (AudioSource* input, const bool deleteInput)
{
if (input != 0)
{
lock.enter();
const int index = inputs.indexOf (input);
    
    if (index >= 0)
    {
        inputsToDelete.shiftBits (index, 1);
        inputs.remove (index);
    }

    lock.exit();

    if (index >= 0)
    {
        input->releaseResources();
        
        if (deleteInput)
            delete input;
    }
}
}

void InputCachingMixerAudioSource::removeAllInputs()
{
lock.enter();
Array<AudioSource*> inputsCopy (inputs);
BigInteger inputsToDeleteCopy (inputsToDelete);
inputs.clear();
lock.exit();
    
for (int i = inputsCopy.size(); --i >= 0;)
    if (inputsToDeleteCopy[i])
        delete inputsCopy[i];
}

void InputCachingMixerAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
tempBuffer.setSize (2, samplesPerBlockExpected);
cacheBuffer.setSize (2, samplesPerBlockExpected);

const ScopedLock sl (lock);

currentSampleRate = sampleRate;
bufferSizeExpected = samplesPerBlockExpected;

for (int i = inputs.size(); --i >= 0;)
    inputs.getUnchecked(i)->prepareToPlay (samplesPerBlockExpected,
                                                            sampleRate);
}

void InputCachingMixerAudioSource::releaseResources()
{
const ScopedLock sl (lock);

    for (int i = inputs.size(); --i >= 0;){
        inputs.getUnchecked(i)->releaseResources();
    }
    
tempBuffer.setSize (2, 0);
cacheBuffer.setSize (2, 0);

currentSampleRate = 0;
bufferSizeExpected = 0;
}

void InputCachingMixerAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
        
const ScopedLock sl (lock);

    if(!inputs.isEmpty()){
        if (inputs.size() > 0)
        {
            cacheBuffer = *info.buffer;

            inputs[0]->getNextAudioBlock (info);

            if (inputs.size() > 1)
            {
                tempBuffer.setSize (jmax (1, info.buffer->getNumChannels()),
                                    info.buffer->getNumSamples());
                
                AudioSourceChannelInfo info2;
                info2.buffer = &tempBuffer;
                info2.numSamples = info.numSamples;
                info2.startSample = 0;

                for (int i = 1; i < inputs.size(); ++i)
                {
                    inputs[i]->getNextAudioBlock (info2);

                    for (int chan = 0; chan < info.buffer->getNumChannels(); ++chan)
                        info.buffer->addFrom (chan, info.startSample, tempBuffer, chan, 0, info.numSamples);

                    tempBuffer = cacheBuffer;
                }
            }
        }
        else
        {
            info.clearActiveBufferRegion();
        }
    }
    
}
