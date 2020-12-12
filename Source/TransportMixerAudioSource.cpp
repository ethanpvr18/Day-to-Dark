#pragma once

#include <JuceHeader.h>

using namespace juce;

class TransportMixerAudioSource  : public MixerAudioSource
{
public:
    TransportMixerAudioSource() : currentSampleRate (0.0), bufferSizeExpected (0){
        
    }

    ~TransportMixerAudioSource(){
        removeAllInputs();
    }
    
    void addInputSource (AudioTransportSource* newInput, bool deleteWhenRemoved) {
        if (newInput != nullptr && ! inputs.contains (newInput))
        {
            double localRate;
            int localBufferSize;

            {
                const ScopedLock sl (lock);
                localRate = currentSampleRate;
                localBufferSize = bufferSizeExpected;
            }

            if (localRate > 0.0)
                newInput->prepareToPlay (localBufferSize, localRate);

            const ScopedLock sl (lock);

            inputsToDelete.setBit (inputs.size(), deleteWhenRemoved);
            inputs.add (newInput);
        }
    }

    void removeInputSource (AudioTransportSource* input) {
        if (input != nullptr)
        {
            std::unique_ptr<AudioTransportSource> toDelete;

            {
                const ScopedLock sl (lock);
                const int index = inputs.indexOf (input);

                if (index < 0)
                    return;

                if (inputsToDelete [index])
                    toDelete.reset (input);

                inputsToDelete.shiftBits (-1, index);
                inputs.remove (index);
            }

            input->releaseResources();
        }
    }

    void removeAllInputs() {
        OwnedArray<AudioTransportSource> toDelete;

        {
            const ScopedLock sl (lock);

            for (int i = inputs.size(); --i >= 0;)
                if (inputsToDelete[i])
                    toDelete.add (inputs.getUnchecked(i));

            inputs.clear();
        }

        for (int i = toDelete.size(); --i >= 0;)
            toDelete.getUnchecked(i)->releaseResources();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override{
        tempBuffer.setSize (2, samplesPerBlockExpected);

        const ScopedLock sl (lock);

        currentSampleRate = sampleRate;
        bufferSizeExpected = samplesPerBlockExpected;

        for (int i = inputs.size(); --i >= 0;)
            inputs.getUnchecked(i)->prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() override{
        const ScopedLock sl (lock);

        for (int i = inputs.size(); --i >= 0;)
            inputs.getUnchecked(i)->releaseResources();

        tempBuffer.setSize (2, 0);

        currentSampleRate = 0;
        bufferSizeExpected = 0;
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& info) override{
        const ScopedLock sl (lock);

        if (inputs.size() > 0)
        {
            inputs.getUnchecked(0)->getNextAudioBlock (info);

            if (inputs.size() > 1)
            {
                tempBuffer.setSize (jmax (1, info.buffer->getNumChannels()),
                                    info.buffer->getNumSamples());

                AudioSourceChannelInfo info2 (&tempBuffer, 0, info.numSamples);

                for (int i = 1; i < inputs.size(); ++i)
                {
                    inputs.getUnchecked(i)->getNextAudioBlock (info2);

                    for (int chan = 0; chan < info.buffer->getNumChannels(); ++chan)
                        info.buffer->addFrom (chan, info.startSample, tempBuffer, chan, 0, info.numSamples);
                }
            }
        }
        else
        {
            info.clearActiveBufferRegion();
        }
    }
private:
    Array<AudioTransportSource*> inputs;
    BigInteger inputsToDelete;
    CriticalSection lock;
    AudioBuffer<float> tempBuffer;
    double currentSampleRate;
    int bufferSizeExpected;
};

