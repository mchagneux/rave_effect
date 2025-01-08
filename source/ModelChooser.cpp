#include "./ModelChooser.h"
#include "./Identifiers.h"

ModelChooser::ModelChooser(const juce::ValueTree& s): state(s)
{
    loadButton.setButtonText("Load...");
    loadButton.onClick = [&] ()
    
    {     
        
        fileChooser = std::make_unique<juce::FileChooser> ("Please select the model you want to load...",
                                                juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                                                "*.ts");
    
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
        fileChooser->launchAsync (chooserFlags, [&] (const juce::FileChooser& chooser)
        {
    
            auto file = chooser.getResult();
            if (file.existsAsFile()){
                state.setProperty(fullModelPath, file.getFullPathName(), nullptr); 
            }
        });
        
    }; 
    addAndMakeVisible(loadButton); 
}

void ModelChooser::resized()
{
    loadButton.setBounds(getLocalBounds());
}
