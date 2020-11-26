#pragma once

#include <JuceHeader.h>
#include <iostream>

using namespace std;

class TableComponent: public juce::AudioAppComponent, public juce::TableListBoxModel {
public:
    TableComponent() : state(Stopped)
    {
        addAndMakeVisible (table);
        
        table.setModel(this);
        table.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
        table.setOutlineThickness(1);
                
        loadData();

        if (columnList != nullptr)
        {
            forEachXmlChildElement (*columnList, columnXml)
            {
                table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                             columnXml->getIntAttribute ("columnId"),
                                             columnXml->getIntAttribute ("width"),
                                             columnXml->getIntAttribute ("width"),
                                             columnXml->getIntAttribute ("width"),
                                             juce::TableHeaderComponent::ColumnPropertyFlags::notResizableOrSortable);
                
            }
        }
        
        formatManager.registerBasicFormats();

        if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
            && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
        {
            juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                               [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
        }
        else
        {
            // Specify the number of input and output channels that we want to open
            setAudioChannels (2, 2);
        }
    }
    
    ~TableComponent() override
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }
    
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock (bufferToFill);
    }
    
    void releaseResources() override
    {
        transportSource.releaseResources();
    }
    
//    void changeListenerCallback (juce::ChangeBroadcaster* source)
//    {
//        if (source == &transportSource)
//        {
//            if (transportSource.isPlaying())
//                changeState (Playing);
//            else
//                changeState (Stopped);
//        }
//    }
    
    void returnKeyPressed (int currentSelectedRow) override
    {
        auto file = getAttributeFileForRowId(currentSelectedRow);
        
        auto* reader = formatManager.createReaderFor (file);
        
        if (reader != nullptr)
        {
            std::unique_ptr<juce::AudioFormatReaderSource> newSource (new juce::AudioFormatReaderSource (reader, true));
            transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
            readerSource.reset (newSource.release());
        }
        
        changeState (Starting);
    }
    
    void deleteKeyPressed (int currentSelectedRow) override
    {
        changeState (Stopping);
    }

    int getNumRows() override
    {
        return numRows;
    }

    void paintRowBackground (juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
                                               .interpolatedWith(getLookAndFeel()
                                               .findColour(juce::ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll(juce::Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll(alternateColour);
    }

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
    
    void resized() override {
        table.setBounds(getLocalBounds());
    }
private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };
    
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
    
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;
    
    juce::TableListBox table  { {}, this };
    juce::Font font           { 14.0f };

    std::unique_ptr<juce::XmlElement> data;
    juce::XmlElement* columnList = nullptr;
    juce::XmlElement* dataList = nullptr;
    int numRows = 0;
    
    void loadData()
    {
        //Change to your resources path for now, will fix
        juce::File tableFile = juce::File("/Users/ethan/TFX/Resources/TableData.xml");

//        auto dir = juce::File::getCurrentWorkingDirectory();
//
//        int numTries = 0;
//
//        while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
//            dir = dir.getParentDirectory();
//
//        auto tableFile = dir.getChildFile("Resources").getChildFile("TableData.xml");
        
        if (tableFile.exists())
        {
            data = juce::XmlDocument::parse (tableFile);

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
    
    juce::String getAttributeFileForRowId (const int rowId) const
    {
        if (auto* rowElement = dataList->getChildElement(rowId))
        {
            auto file = rowElement->getStringAttribute (getAttributeNameForColumnId (3));
            
            return file;
        }
    }
};

class MainComponent : public juce::Component
{
public:
    MainComponent() : juce::Component()
    {
        addAndMakeVisible (table);
        
        setSize (1200, 600);
    }
    
    void resized() override
    {
        table.setBounds (getLocalBounds());
    }

private:
    TableComponent table;
};
