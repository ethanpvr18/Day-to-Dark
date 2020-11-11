#include <string>
#include "BaseCue.h"

/*
    TODO:
*/

// This is the class for an indiviual audio cue

using namespace std;

class AudioCue: public BaseCue {
    public:
        //Definition of Setters
        void setVolume(double volume);
        void setFileName(string fileName);

        //Definiion of Getters
        double getVolume(void);
        string getFileName(void);

        //Definition of Constructor

        //Default Constructor
        AudioCue();

        //User Defined Constructor
        AudioCue(double cueNum, string cueName, string fileName, double volume, int hoursStart, int minutesStart, int secondsStart, int hoursEnd, int minutesEnd, int secondsEnd);

    private:
        // Cue Descriptive Attributes
        string file;

        //Volume
        double volume;

};