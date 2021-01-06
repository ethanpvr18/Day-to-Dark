//=======================================================================================================================================
//File [1] - TFX App Start Code
 
#include <JuceHeader.h>
#include "MainComponent.cpp"

class TFXApplication  : public juce::JUCEApplication
{
public:
    //Application Constructor
    TFXApplication() = default;

    //To get Application Name
    const juce::String getApplicationName() override       { return "Day to Dark"; }
    //To get Application Version
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    
    //When started
    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName(), new MainComponent(), *this));
    }

    //When shutdown
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
    //When quit
    void systemRequestedQuit() override
    {
        quit();
    }
    
private:
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        //Main Window Constructor
        MainWindow (juce::String name, juce::Component* c, JUCEApplication& a)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons),
            app (a)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);

        
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());

            setVisible (true);
        }

        //Close Button Request
        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

    private:
        JUCEApplication& app;
    
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

//JUCE APPLICATION START
START_JUCE_APPLICATION (TFXApplication)
//========================================================================================================================================
