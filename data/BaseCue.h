#include <string>

/*
    TODO:
*/

// This is the class for all indiviual cues to be built on or inherited from

using namespace std;

class BaseCue{
    public:
        //Definition of Setters
        void setCueNum(double cueNum);
        void setCueName(string cueName);
        void setStartTimeInSeq(int hours, int minutes, int seconds);
        void setEndTimeInSeq(int hours, int minutes, int seconds);

        //Definiion of Getters
        double getCueNum(void);
        string getCueName(void);
        int* startTimeInSeq(void);
        int* endTimeInSeq(void);

        //Definition of Constructor

        //Default Constructor
        BaseCue();

        //User Defined Constructor
        BaseCue(double cueNum, string cueName, int hoursStart, int minutesStart, int secondsStart, int hoursEnd, int minutesEnd, int secondsEnd);

    private:
        // Cue Descriptive Attributes
        double num;
        string name;

        //Values to express where the cue shall be in the timeline
        int start[3];
        int end[3];
};