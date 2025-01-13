#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), modelChooser(p.state)
{
    juce::ignoreUnused (processorRef);
    webViewHolder = choc::ui::createJUCEWebViewHolder(webView); 

    webView.navigate("localhost:5173");

    addAndMakeVisible(*webViewHolder); 
    addAndMakeVisible(modelChooser);
    setSize (640, 360);
    setResizable(true, true); 
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{

    auto area = getLocalBounds(); 

    auto modelChooserArea = area.removeFromBottom((int) (area.toFloat().getHeight() / 6.0f));

    modelChooser.setBounds(modelChooserArea);
    webViewHolder->setBounds(area); 
}
