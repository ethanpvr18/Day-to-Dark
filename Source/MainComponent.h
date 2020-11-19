#pragma once

#include <JuceHeader.h>

class TableComponent: public juce::Component, public juce::TableListBoxModel {
public:
    TableComponent()
    {
        loadData();                                                              

        addAndMakeVisible (table);                                               

        table.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);      
        table.setOutlineThickness (1);
        
        if (columnList != nullptr)
        {
            forEachXmlChildElement (*columnList, columnXml)
            {
                table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                             columnXml->getIntAttribute ("columnId"),
                                             columnXml->getIntAttribute ("width"),
                                             columnXml->getIntAttribute ("width"),
                                             columnXml->getIntAttribute ("width"),
                                             18,
                                             0);
            }
        }
                        
        table.getHeader().setSortColumnId (1, true);                               

        table.setMultipleSelectionEnabled (true);                                   
    }

    int getNumRows() override
    {
        return numRows;
    }

    void paintRowBackground (juce::Graphics& g, int rowNumber, bool rowIsSelected)
    {
        auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
                                               .interpolatedWith(getLookAndFeel()
                                               .findColour(juce::ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll (juce::Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll (alternateColour);
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

    int getSelection (const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute ("Select");
    }

    void setSelection (const int rowNumber, const int newSelection)
    {
        dataList->getChildElement (rowNumber)->setAttribute ("Select", newSelection);
    }

    juce::String getText (const int columnNumber, const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getStringAttribute (getAttributeNameForColumnId (columnNumber));
    }

    void setText (const int columnNumber, const int rowNumber, const juce::String& newText)
    {
        const auto& columnName = table.getHeader().getColumnName (columnNumber);
        dataList->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }

    void resized() override
    {
        table.setBoundsInset (juce::BorderSize<int> (8));
    }

private:
    juce::TableListBox table  { {}, this };
    juce::Font font           { 14.0f };

    std::unique_ptr<juce::XmlElement> data;
    juce::XmlElement* columnList = nullptr;
    juce::XmlElement* dataList = nullptr;
    int numRows = 0;

    class EditableTextCustomComponent  : public juce::Label
    {
    public:
        EditableTextCustomComponent (TableComponent& td)
            : owner (td)
        {
            setEditable (false, true, false);
        }

        void mouseDown (const juce::MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }

        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }

        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), juce::dontSendNotification);
        }

    private:
        TableComponent& owner;
        int row, columnId;
        juce::Colour textColour;
    };

    class SelectionColumnCustomComponent    : public Component
    {
    public:
        SelectionColumnCustomComponent (TableComponent& td)
            : owner (td)
        {
            addAndMakeVisible (toggleButton);

            toggleButton.onClick = [this] { owner.setSelection (row, (int) toggleButton.getToggleState()); };
        }

        void resized() override
        {
            toggleButton.setBoundsInset (juce::BorderSize<int> (2));
        }

        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            toggleButton.setToggleState ((bool) owner.getSelection (row), juce::dontSendNotification);
        }

    private:
        TableComponent& owner;
        juce::ToggleButton toggleButton;
        int row, columnId;
    };

    class DataSorter
    {
    public:
        DataSorter (const juce::String& attributeToSortBy, bool forwards)
            : attributeToSort (attributeToSortBy),
              direction (forwards ? 1 : -1)
        {}

        int compareElements (juce::XmlElement* first, juce::XmlElement* second) const
        {
            auto result = first->getStringAttribute (attributeToSort)
                                .compareNatural (second->getStringAttribute (attributeToSort));

            if (result == 0)
                result = first->getStringAttribute ("ID")
                               .compareNatural (second->getStringAttribute ("ID"));

            return direction * result;
        }

    private:
        juce::String attributeToSort;
        int direction;
    };

    void loadData()
    {
        //Fix: juce::File::getCurrentWorkingDirectory()
        
        juce::File dir = juce::File("../TFX");
        
        int numTries = 0;

        while (! dir.getChildFile ("Resources").exists() && numTries++ < 15)
            dir = dir.getParentDirectory();

        auto tableFile = dir.getChildFile ("Resources").getChildFile ("TableData.xml");

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableComponent)
};

class MainComponent : public juce::Component
{
public:
    MainComponent() : juce::Component()
    {
        addAndMakeVisible(table);
        
        setSize(1200, 600);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        table.setBounds(getLocalBounds());
    }

private:
    TableComponent::Component table;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
