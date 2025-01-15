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
        enabledLabel.attachToComponent(&enabledButton, false);
        enabledButton.setClickingTogglesState(true);
        enabledButton.onClick = [&] 
        {
            parameters.neuralEnabled.setValueNotifyingHost(enabledButton.getToggleState());
        }; 
        addAndMakeVisible(enabledButton);
        addAndMakeVisible(enabledLabel);
    }

    void resized()
    {
        auto r = getLocalBounds();
        auto dryWetSliderArea = r.removeFromLeft((int)(getWidth() / 2)); 

        dryWetSlider.setBounds (dryWetSliderArea.reduced((int) (dryWetSliderArea.getWidth() / 4), (int) (dryWetSliderArea.getHeight() / 4)));
        enabledButton.setBounds(r.reduced((int) (r.getWidth() / 4), (int) (r.getHeight() / 4))); 
    }

    // AttachedCombo backendType;
    // AttachedSlider dryWet;
    juce::Slider dryWetSlider;
    juce::Label sliderLabel { "Dry/Wet" };
    juce::SliderParameterAttachment sliderAttachment;

    juce::ToggleButton enabledButton;
    juce::Label enabledLabel {"Enabled"}; 
    bool lastClickedValue = false; 
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
    // RaveControls raveControls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
