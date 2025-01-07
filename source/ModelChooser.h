#pragma once 
#include <juce_gui_basics/juce_gui_basics.h>



struct ModelChooser : public juce::Component
{
public: 
    ModelChooser(); 
    void resized() override; 
    std::function<void(const std::string&)> loadModelCallback;  

private: 
    juce::TextButton loadButton; 
    std::unique_ptr<juce::FileChooser> fileChooser; 

};


