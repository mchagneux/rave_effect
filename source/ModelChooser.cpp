#pragma once 
#include "./ModelChooser.h"

ModelChooser::ModelChooser()
{
    loadButton.setButtonText("Load...");
    loadButton.onClick = [&] ()
    
    {     
        
        fileChooser = std::make_unique<juce::FileChooser> ("Please select the model you want to load...",
                                                juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                                                "*.ts");
    
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    
        fileChooser->launchAsync (folderChooserFlags, [this] (const juce::FileChooser& chooser)
        {
    
            if(loadModelCallback){
                
                loadModelCallback (chooser.getResult().getFileName().toStdString());
            } 
        });
        
    }; 
    addAndMakeVisible(loadButton); 
}

void ModelChooser::resized()
{
    loadButton.setBounds(getLocalBounds());
}
