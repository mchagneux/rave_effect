#pragma once 
#include <JuceHeader.h>


struct ModelChooser : public juce::Component
{
public: 
    ModelChooser(const juce::ValueTree& s); 
    void resized() override; 

private: 
    juce::ValueTree state; 
    juce::TextButton loadButton; 
    std::unique_ptr<juce::FileChooser> fileChooser; 

};


