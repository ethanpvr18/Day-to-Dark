#include <iostream>
#include "data/AudioCue.h"

using namespace std;

int main(){

    AudioCue cueOne(10.0, "Test Cue", "/Users/ethan/desktop/testFile.mp3", 0.0, 0, 0, 0, 0, 0, 10);

    cout << (cueOne.getCueName());

    return 0;
}