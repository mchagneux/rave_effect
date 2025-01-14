#pragma once

#include "PluginProcessor.h"
#include "./ModelChooser.h"
#include <JuceHeader.h>
#include "../choc/gui/choc_WebView.h"
#include "./PostProcessor.h"




struct RaveControls final : public juce::Component
{
    explicit RaveControls (juce::AudioProcessorEditor& editorIn, const NeuralParameters& parameters)
        : sliderAttachment (parameters.neuralDryWet, dryWetSlider, nullptr)

    {
        sliderLabel.attachToComponent (&dryWetSlider, false);
        dryWetSlider.setSliderStyle (juce::Slider::SliderStyle::LinearBar);
        addAndMakeVisible (dryWetSlider);
        addAndMakeVisible (sliderLabel);
    }

    void resized()
    {
        auto r = getLocalBounds();
        dryWetSlider.setBounds (r);
    }

    // AttachedCombo backendType;
    // AttachedSlider dryWet;
    juce::Slider dryWetSlider;
    juce::Label sliderLabel { "Dry/Wet" };
    juce::SliderParameterAttachment sliderAttachment;
};


//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::AudioProcessorParameter::Listener, private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


    void parameterValueChanged(int, float) override; 
    void parameterGestureChanged(int, bool) override; 
    void timerCallback() override; 
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    ModelChooser modelChooser; 
    std::unique_ptr<choc::ui::WebView> webView; 
    std::unique_ptr<juce::Component> webViewHolder; 
    PostProcessorControls postProcessorControls;
    RaveControls raveControls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
