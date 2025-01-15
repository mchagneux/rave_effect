#pragma once
#include <JuceHeader.h>
#include "./Parameters.h"
#include "./Misc.h"
#include "./EQ.h"

struct PostProcessor
{
public:
    PostProcessor (const PostProcessorParameters& p)
        : parameters (p)
        , eq (parameters.eq)
    {
    }

    ~PostProcessor() {}

    //==============================================================================
    void prepare (juce::dsp::ProcessSpec& spec)
    {
        // Use this method as the place to do any pre-playback
        // initialisation that you need..
        // const auto channels = juce::jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());

        if (spec.numChannels == 0)
            return;
        eq.prepare (spec);
    }

    void reset()
    {
        eq.reset();
    }

    void process (juce::dsp::ProcessContextReplacing<SampleType>& context)
    {
        eq.process (context);

        auto numChannels = context.getOutputBlock().getNumChannels();
        auto numSamples = context.getOutputBlock().getNumSamples();

        float* dataToReferTo[numChannels];

        for (unsigned int idx = 0; idx < numChannels; ++idx)
        {
            dataToReferTo[idx] = context.getOutputBlock().getChannelPointer (idx);
        }

        eq.postAnalyzer.addAudioData (juce::AudioBuffer<SampleType> (dataToReferTo, numChannels, numSamples), 0, numChannels);
    }

    const PostProcessorParameters& parameters;

    auto& getEQ()
    {
        return eq;
    }

private:
    EQ eq;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PostProcessor)
};

struct PostProcessorControls final : public juce::Component
{
    explicit PostProcessorControls (juce::AudioProcessorEditor& editor, PostProcessor& pp)
        : eq (editor, pp.getEQ())
    {
        addAllAndMakeVisible (*this, eq);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        // distortion.setBounds(r.removeFromTop((int) (getHeight() / 2)));
        eq.setBounds (r);
    }

    EQControls eq;
};
