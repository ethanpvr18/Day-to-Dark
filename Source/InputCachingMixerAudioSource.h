#include <JuceHeader.h>

using namespace std;
using namespace juce;


class InputCachingMixerAudioSource : public AudioSource
{
public:

InputCachingMixerAudioSource();

~InputCachingMixerAudioSource();

void addInputSource (AudioSource* newInput,
                     const bool deleteWhenRemoved);


void removeInputSource (AudioSource* input,
                        const bool deleteSource);


void removeAllInputs();


void prepareToPlay (int samplesPerBlockExpected, double sampleRate);


void releaseResources();

void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

private:
Array<AudioSource*> inputs;
BigInteger inputsToDelete;
CriticalSection lock;
AudioSampleBuffer tempBuffer;
AudioSampleBuffer cacheBuffer;
double currentSampleRate;
int bufferSizeExpected;

InputCachingMixerAudioSource (const InputCachingMixerAudioSource&);
const InputCachingMixerAudioSource& operator= (const InputCachingMixerAudioSource&);
};
