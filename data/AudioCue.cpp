#include <string>
#include "AudioCue.h"

/*
    TODO:
*/

// This is the class for an indiviual audio cue

using namespace std;

//Implementation of Constructor

//Default Constructor

AudioCue::AudioCue(){
    setCueNum(0.0);

    setCueName("Empty Cue");

    file = "";

    volume = 0.0;

    setStartTimeInSeq(0,0,0);

    setEndTimeInSeq(0,0,0);
}

//User Defined Constructor

AudioCue::AudioCue(double cueNum, string cueName, string fileName, double volume, int hoursStart, int minutesStart, int secondsStart, int hoursEnd, int minutesEnd, int secondsEnd){
    setCueNum(cueNum);

    setCueName(cueName);

    file = fileName;

    volume = volume;

    setStartTimeInSeq(hoursStart,minutesStart,secondsStart);

    setEndTimeInSeq(hoursEnd,minutesEnd,secondsEnd);
}

// Implementation of Setters

void AudioCue::setFileName(string fileName){
    file = fileName;
}

void AudioCue::setVolume(double volume){
    volume = volume;
}

// Implementation of Getters

string AudioCue::getFileName(void){
    return file;
}

double AudioCue::getVolume(void){
    return volume;
}