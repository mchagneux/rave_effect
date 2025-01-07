#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    modelChooser.loadModelCallback = [&p] (const std::string& modelFileName){ p.raveProcessor.loadModel(modelFileName) ; };
    addAndMakeVisible(modelChooser);
    setSize (400, 300);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{

    auto area = getLocalBounds(); 

    auto modelChooserArea = area.removeFromBottom((int) (area.toFloat().getHeight() / 6.0f));

    modelChooser.setBounds(modelChooserArea);
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
