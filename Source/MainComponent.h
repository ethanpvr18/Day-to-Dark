//=======================================================================================================================================
//File [2] - Main Component

#pragma once

#include <JuceHeader.h>
#include <iostream>

using namespace std;

class TableComponent: public juce::AudioAppComponent, public juce::TableListBoxModel, private KeyListener {
public:
    
    
    
    //=======================================================================================================================================
    //Section [1] -- Constructor
    TableComponent() : state(Stopped)
    {
        //Subsection [a] -- Create Add Audio Cue Button and its Function
        addAudioCueButton.onClick = [this] { addAudioCue(getNumRows()+1, "", ""); };
        addAndMakeVisible (addAudioCueButton);
        addAndMakeVisible (addGroupButton);
        addAndMakeVisible (addFadeButton);
        addAndMakeVisible (addPlayCueButton);
        addAndMakeVisible (addStopCueButton);
        addAndMakeVisible (addPauseCueButton);
        
        addAndMakeVisible (number);
        number.setTextToShowWhenEmpty("Enter Cue Number",juce::Colours::grey);
        addAndMakeVisible (name);
        name.setTextToShowWhenEmpty("Enter Cue Name",juce::Colours::grey);
        addAndMakeVisible (target);
        target.setTextToShowWhenEmpty("Enter Target File or Target Cue",juce::Colours::grey);
        updateCueButton.onClick = [this] { updateCueParams(number.getText(), name.getText(), target.getText()); };
        addAndMakeVisible (updateCueButton);
        
        addAndMakeVisible (newProject);
        newProject.onClick = [this] { newProjectWorkspace(); };
        addAndMakeVisible (openProject);
        openProject.onClick = [this] { openProjectWorkspace(); };

        //Subsection [b] -- Create Table and Load Data
        addAndMakeVisible (table);
        table.setModel(this);
        table.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
        table.setOutlineThickness(1);

        //Subsection [c] -- Get Necessary Formats and Permissions for Playing Audio
        formatManager.registerBasicFormats();
        if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio) && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio)){
            juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio, [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
        } else {
            setAudioChannels (2, 2);
        }
        
        setWantsKeyboardFocus(true);
        addKeyListener(this);
    }
    
    //Deconstructor
    ~TableComponent() override { shutdownAudio(); }
    //=======================================================================================================================================
    
    
    
    //=======================================================================================================================================
    //Section [2] -- Playing Audio Helper Methods
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override { transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate); }
    void releaseResources() override { transportSource.releaseResources(); }
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override {
        if (readerSource.get() == nullptr){
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        transportSource.getNextAudioBlock (bufferToFill);
    }
    //=======================================================================================================================================
    
    
    
    //=======================================================================================================================================
    //Section [3] -- User Input Helper Methods
    
    void newProjectWorkspace(){
        if(dirChooser.browseForDirectory()){
            juce::String dir = dirChooser.getResult().getFullPathName();
                        
            XmlDocument file (juce::File(dir+"/Untitled.xml"));

            XmlElement headTag (juce::String("TABLE_DATA"));
            
            XmlElement* headersTag = new XmlElement("HEADERS");
            headTag.addChildElement(headersTag);

            XmlElement* numColTag = new XmlElement("COLUMN");
            headersTag->addChildElement(numColTag);
            XmlElement* nameColTag = new XmlElement("COLUMN");
            headersTag->addChildElement(nameColTag);
            XmlElement* targetColTag = new XmlElement("COLUMN");
            headersTag->addChildElement(targetColTag);

            numColTag->setAttribute("name", "Number");
            numColTag->setAttribute("columnId", "1");
            numColTag->setAttribute("width", "200");

            nameColTag->setAttribute("name", "Name");
            nameColTag->setAttribute("columnId", "2");
            nameColTag->setAttribute("width", "200");

            targetColTag->setAttribute("name", "Target");
            targetColTag->setAttribute("columnId", "3");
            targetColTag->setAttribute("width", "200");

            XmlElement* dataTag = new XmlElement("DATA");
            headTag.addChildElement(dataTag);
            
            XmlElement* firstCue = new XmlElement("ITEM");
            dataTag->addChildElement(firstCue);
            
            firstCue->setAttribute("Number", "0");
            firstCue->setAttribute("Name", "");
            firstCue->setAttribute("Target", "");
            
            headTag.XmlElement::writeTo(juce::File(dir+"/Untitled.xml"), XmlElement::TextFormat());
            
            setCurrentFile(dir+"/Untitled.xml");
                        
            if (headersTag != nullptr){
                forEachXmlChildElement (*headersTag, columnXml){
                    table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                                 columnXml->getIntAttribute ("columnId"),
                                                 columnXml->getIntAttribute ("width"),
                                                 columnXml->getIntAttribute ("width"),
                                                 columnXml->getIntAttribute ("width"),
                                                 juce::TableHeaderComponent::ColumnPropertyFlags::notResizableOrSortable);
                    
                    numCols++;
                    
                }
            }
            
            table.updateContent();
            repaint();
        }
    }
    
    void openProjectWorkspace(){
        if(chooser.browseForFileToOpen(nullptr)){
            juce::String file = chooser.getResult().getFullPathName();
            
            setCurrentFile(file);
                        
            loadData(currentFile);
            
            if (columnList != nullptr){
                forEachXmlChildElement (*columnList, columnXml){
                    table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                                 columnXml->getIntAttribute ("columnId"),
                                                 columnXml->getIntAttribute ("width"),
                                                 columnXml->getIntAttribute ("width"),
                                                 columnXml->getIntAttribute ("width"),
                                                 juce::TableHeaderComponent::ColumnPropertyFlags::notResizableOrSortable);
                    
                    numCols++;
                    
                }
            }
            
            table.updateContent();
            repaint();
        }
    }
    
    //Subsection [a] -- Method to Add a Audio Cue to the GUI Table and the XML File
    void addAudioCue(int num, juce::String name, juce::String filePath) {
        loadData(currentFile);
        
        XmlElement* newItem = dataList->createNewChildElement("ITEM");
        
        newItem->setAttribute("Number", num);
        newItem->setAttribute("Name", name);
        newItem->setAttribute("Target", filePath);
                
        data->XmlElement::writeTo(currentFile, XmlElement::TextFormat());
        
        numRows++;
        
        table.updateContent();
        repaint();
    }
    
    void updateCueParams(juce::String num, juce::String name, juce::String filePath){
        XmlElement* cueToUpdate = dataList->getChildElement(rowNumSelected);
        
        cueToUpdate->setAttribute("Number", num);
        cueToUpdate->setAttribute("Name", name);
        cueToUpdate->setAttribute("Target", filePath);
        
        data->XmlElement::writeTo(currentFile, XmlElement::TextFormat());

        table.updateContent();
        repaint();
        
        this->number.clear();
        this->name.clear();
        this->target.clear();
    }
    
    //Subsection [c] -- Method to Play the selected Audio File by Highlighting a Row and pressing Enter/Return
    void deleteKeyPressed (int currentSelectedRow) override {
        XmlElement* existingItem = dataList->getChildElement(currentSelectedRow);
        
        dataList->removeChildElement(existingItem, true);
        
        data->XmlElement::writeTo(juce::File(currentFile), XmlElement::TextFormat());
        
        numRows--;
        
        table.updateContent();
        repaint();
    }
    
    bool keyPressed(const KeyPress &k, Component *c) override {
        if( k.getKeyCode() == juce::KeyPress::spaceKey ) {
            
            auto file = getAttributeFileForRowId(rowNumSelected);
            
            auto* reader = formatManager.createReaderFor (file);
            
            if (reader != nullptr) {
                std::unique_ptr<juce::AudioFormatReaderSource> newSource (new juce::AudioFormatReaderSource (reader, true));
                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                readerSource.reset (newSource.release());
            }
            
            changeState (Starting);
            return true;

        }
        
        if( k.getKeyCode() == juce::KeyPress::escapeKey ) {
            
            changeState (Stopping);
            return true;
        }
            
        return false;
    }
    //=======================================================================================================================================

    
    
    //=======================================================================================================================================
    //Section [4] -- GUI Helper Methods
    
    //Subsection [a] -- Get the Current Number of Rows
    int getNumRows() override {
        return numRows;
    }
    
    int getNumCols() {
        return numCols;
    }
    
    void setCurrentFile(juce::String file){
        currentFile = juce::File(file);
    }

    //Subsection [b] -- Paints the Alternating Row Colors
    void paintRowBackground (juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
                                               .interpolatedWith(getLookAndFeel()
                                               .findColour(juce::ListBox::textColourId), 0.03f);
        if (rowIsSelected){
            g.fillAll(juce::Colours::lightblue);
            rowNumSelected = rowNumber;
        }else if (rowNumber % 2){
            g.fillAll(alternateColour);
        }
    }

    //Subsection [c] -- Paints each cell, and enter's the data's text
    void paintCell (juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour (juce::ListBox::textColourId));
        g.setFont (font);
        
        if (auto* rowElement = dataList->getChildElement (rowNumber))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));
            
            g.drawText (text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        }

        g.setColour (getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
        g.fillRect (width - 1, 0, 1, height);
    }
    
    //Subsection [d] -- Sets Sizes, and Bounds
    void resized() override {
        table.setBounds(0, 25, 1200, 450);
        
        newProject.setBounds(0, 0, 100, 25);
        openProject.setBounds(100, 0, 100, 25);
        
        addAudioCueButton.setBounds(300, 0, 100, 25);
        addGroupButton.setBounds(450, 0, 100, 25);
        addFadeButton.setBounds(600, 0, 100, 25);
        addPlayCueButton.setBounds(750, 0, 100, 25);
        addStopCueButton.setBounds(850, 0, 100, 25);
        addPauseCueButton.setBounds(950, 0, 100, 25);
        
        number.setBounds(25, 500, 250, 25);
        name.setBounds(25, 525, 250, 25);
        target.setBounds(25, 550, 250, 25);
        
        updateCueButton.setBounds(25, 575, 250, 25);
    }
    //=======================================================================================================================================

    
    
private:
    
    
    
    //=======================================================================================================================================
    //Section [5] -- Private State Helper Methods
    
    //Subsection [a] -- Declare State Values
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };
    
    //Subsection [b] -- Changes the Current State, and Starts Sound
    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:
                    transportSource.setPosition (0.0);
                    break;

                case Starting:
                    transportSource.start();
                    break;

                case Playing:
                    break;

                case Stopping:
                    transportSource.stop();
                    break;
            }
        }
    }
    //=======================================================================================================================================

    
    
    //=======================================================================================================================================
    //Section [6] - Private Variables
    
    //Subsection [a] -- Audio Helper Variables
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;
    
    //Subsection [b] -- GUI Variables
    juce::TableListBox table  { {}, this };
    juce::TextButton addGroupButton {"Group"};
    juce::TextButton addAudioCueButton {"Audio"};
    juce::TextButton addFadeButton {"Fade"};
    juce::TextButton addPlayCueButton {"Play"};
    juce::TextButton addStopCueButton {"Stop"};
    juce::TextButton addPauseCueButton {"Pause"};

    juce::TextEditor number;
    juce::TextEditor name;
    juce::TextEditor target;
    juce::TextButton updateCueButton {"Update Cue"};
    juce::TextButton newProject {"New"};
    juce::TextButton openProject {"Open"};
    juce::Font font           { 14.0f };
    
    //Subsection [c] -- Data Loading Variables
    std::unique_ptr<juce::XmlElement> data;
    juce::XmlElement* columnList = nullptr;
    juce::XmlElement* dataList = nullptr;
    int numRows = 0;
    int numCols = 0;
    int rowNumSelected = 0;
    
    FileChooser chooser {"Choose a Project to open ...", juce::File::getCurrentWorkingDirectory(), "", false, this};
    FileChooser dirChooser {"Choose a Directory to start your new project ...", juce::File::getCurrentWorkingDirectory(), "", false, this};
    
    //Change to your resources path for now, will fix
    juce::File currentFile;
    
    //=======================================================================================================================================
    
    
    
    //=======================================================================================================================================
    //Section [7] -- Load Data Helper Methods
    void loadData(juce::File file)
    {
        if (file.exists())
        {
            data = juce::XmlDocument::parse (file);

            dataList   = data->getChildByName ("DATA");
            columnList = data->getChildByName ("HEADERS");     

            numRows = dataList->getNumChildElements();                 
        }
    }

    juce::String getAttributeNameForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*columnList, columnXml)
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("name");
        }

        return {};
    }
    
    juce::String getAttributeNumberForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*columnList, columnXml)
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("columnId");
        }

        return {};
    }
    
    juce::String getAttributeFileForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*columnList, columnXml)
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("target");
        }

        return {};
    }
    
    juce::String getAttributeFileForRowId (const int rowId) const
    {
        if (auto* rowElement = dataList->getChildElement(rowId))
        {
            auto file = rowElement->getStringAttribute (getAttributeNameForColumnId (3));
            
            return file;
        }
    }
    //=======================================================================================================================================
};



//=======================================================================================================================================
//Section [8] -- Main Component Class with Sub-components to be run by the Main.cpp file
class MainComponent : public juce::Component
{
public:
    MainComponent() : juce::Component()
    {
        addAndMakeVisible (table);
        
        setSize (1200, 625);
    }
    
    void resized() override
    {
        table.setBounds (getLocalBounds());
    }

private:
    TableComponent table;
};
//======================================================================================================================================= 

//=======================================================================================================================================
