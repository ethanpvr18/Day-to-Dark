/*
 * Application's Start Code
 */
 
#include <JuceHeader.h>
#include "MainComponent.h"

class TFXApplication  : public juce::JUCEApplication
{
public:
    //Application Constructor
    TFXApplication() {}

    //To get Application Name
    const juce::String getApplicationName() override       { return "TFX"; }
    //To get Application Version
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    
    //When started
    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    //When shutdown
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
    //Quit Request
    void systemRequestedQuit() override
    {
        quit();
    }
    
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        //Main Window Constructor
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

        
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());

            setVisible (true);
        }

        //Close Button Request
        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//JUCE APPLICATION START
START_JUCE_APPLICATION (TFXApplication)
