#include <string>
#include "BaseCue.h"

/*
    TODO:
*/

// This is the class for all indiviual cues to be built on or inherited from

using namespace std;

//Implementation of Constructor

//Default Constructor

BaseCue::BaseCue(){
    num = 0.0;
    
    name = "Empty Cue";

    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    end[0] = 0;
    end[1] = 0;
    end[2] = 0;
}

//User Defined Constructor

BaseCue::BaseCue(double cueNum, string cueName, int hoursStart, int minutesStart, int secondsStart, int hoursEnd, int minutesEnd, int secondsEnd){
    num = cueNum;
    
    name = cueName;

    start[0] = hoursStart;
    start[1] = minutesStart;
    start[2] = secondsStart;

    end[0] = hoursEnd;
    end[1] = minutesEnd;
    end[2] = secondsEnd;
}

// Implementation of Setters

void BaseCue::setCueNum(double cueNum){
    num = cueNum;
}

void BaseCue::setCueName(string cueName){
    name = cueName;
}

void BaseCue::setStartTimeInSeq(int hours, int minutes, int seconds){
    start[0] = hours;
    start[1] = minutes;
    start[2] = seconds;
}

void BaseCue::setEndTimeInSeq(int hours, int minutes, int seconds){
    end[0] = hours;
    end[1] = minutes;
    end[2] = seconds;
}

// Implementation of Getters

double BaseCue::getCueNum(void){
    return num;
}

string BaseCue::getCueName(void){
    return name;
}

int* BaseCue::startTimeInSeq(void){
    return start;
}

int* BaseCue::endTimeInSeq(void){
    return end;
}