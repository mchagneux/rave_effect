#pragma once

#include <JuceHeader.h>
#include "./RaveProcessor.h"
#include "./Parameters.h"
#include "./PostProcessor.h"
//==============================================================================


// static juce::Identifier stateID ("state");

class AudioPluginAudioProcessor final : public juce::AudioProcessor//, public juce::AudioProcessorParameter::Listener
{
public:
    //==============================================================================
    AudioPluginAudioProcessor(): AudioPluginAudioProcessor (juce::AudioProcessorValueTreeState::ParameterLayout {}) {}

    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    // void parameterValueChanged(int, float) override; 
    // void parameterGestureChanged(int, bool) override; 

    Parameters parameters;
    juce::AudioProcessorValueTreeState apvts;
    PostProcessor postProcessor;
    RaveProcessor raveProcessor; 
    juce::dsp::Gain<float> gainProcessor;
    juce::dsp::Reverb reverbProcessor; 
    juce::dsp::Reverb::Parameters reverbParameters;
    juce::dsp::Phaser<float> phaserProcessor;


private:

    explicit AudioPluginAudioProcessor (
        juce::AudioProcessorValueTreeState::ParameterLayout layout)
        : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
        , parameters (layout)
        , apvts (*this, nullptr, "Plugin", std::move (layout))
        , postProcessor (parameters.postProcessor)
        , raveProcessor(parameters.neural, apvts.state)

        // , neuralProcessor (parameters.neural)
    { 
        // parameters.gain.addListener(this);
        // apvts.state.addProperty()
    }


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
