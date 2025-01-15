#pragma once
#include "./Misc.h"

template <typename Param>
void add (juce::AudioProcessorParameterGroup& group,
          std::unique_ptr<Param> param)
{
    group.addChild (std::move (param));
}

template <typename Param>
void add (juce::AudioProcessorValueTreeState::ParameterLayout& group,
          std::unique_ptr<Param> param)
{
    group.add (std::move (param));
}

template <typename Param, typename Group, typename... Ts>
Param& addToLayout (Group& layout, Ts&&... ts)
{
    auto param = new Param (std::forward<Ts> (ts)...);
    auto& ref = *param;
    add (layout, juce::rawToUniquePtr (param));
    return ref;
}

namespace ID
{
#define PARAMETER_ID(str) constexpr const char* str { #str };

// PARAMETER_ID (neuralBackend)
PARAMETER_ID (neuralDryWet)

PARAMETER_ID (eqFilterCutoff0)
PARAMETER_ID (eqFilterQ0)
PARAMETER_ID (eqFilterGain0)
PARAMETER_ID (eqFilterType0)

PARAMETER_ID (eqFilterCutoff1)
PARAMETER_ID (eqFilterQ1)
PARAMETER_ID (eqFilterGain1)
PARAMETER_ID (eqFilterType1)

PARAMETER_ID (eqFilterCutoff2)
PARAMETER_ID (eqFilterQ2)
PARAMETER_ID (eqFilterGain2)
PARAMETER_ID (eqFilterType2)
PARAMETER_ID (gain)
PARAMETER_ID (reverbDryWet)
PARAMETER_ID (phaserWet)
PARAMETER_ID (neuralEnabled)

#undef PARAMETER_ID

} // namespace ID

using Parameter = juce::AudioProcessorValueTreeState::Parameter;

struct NeuralParameters
{
    // inline static juce::StringArray backendTypes { "TFLITE", "LIBTORCH", "ONNXRUNTIME", "NONE" };
    // inline static juce::String defaultBackend { backendTypes[2] };

    template <typename T>
    explicit NeuralParameters (T& layout)
        : // : neuralBackend (addToLayout<juce::AudioParameterChoice> (
        //     layout,
        //     ID::neuralBackend,
        //     "Backend type",
        //     backendTypes,
        //     backendTypes.indexOf (defaultBackend)))

        neuralDryWet (addToLayout<Parameter> (
            layout,
            ID::neuralDryWet,
            "Dry / Wet",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            1.0f))
        , neuralEnabled (addToLayout<juce::AudioParameterBool> (layout, ID::neuralEnabled, "Neural Enabled", false))
    {
    }

    // juce::AudioParameterChoice& neuralBackend;
    Parameter& neuralDryWet;
    juce::AudioParameterBool& neuralEnabled;
};

struct FilterParameters
{
    inline static juce::StringArray filterTypes {
        "Lowpass",
        "Bandpass",
        "Highpass",
        "All-pass",
        "Notch",
        "Low shelf",
        "High shelf",
        "Bell"
    };

    // inline static juce::String defaultFilterType { filterTypes[0] };

    template <typename T>
    explicit FilterParameters (T& layout, std::vector<const char*> parameterIDs, float startFreq)
        : type (addToLayout<juce::AudioParameterChoice> (
            layout,
            juce::ParameterID { parameterIDs[0], 1 },
            "Filter Type",
            filterTypes,
            7))
        , gain (addToLayout<Parameter> (
              layout,
              juce::ParameterID { parameterIDs[1], 1 },
              "Filter Gain",
              juce::NormalisableRange<float> (-10.0f, 10.0f),
              0.0f,
              getDbAttributes()))
        , cutoff (addToLayout<Parameter> (
              layout,
              juce::ParameterID { parameterIDs[2], 1 },
              "Filter Cutoff",
              juce::NormalisableRange<float> (20.0f, 22000.0f, 0.0f, 0.25f),
              startFreq,
              getHzAttributes()))
        , Q (addToLayout<Parameter> (
              layout,
              juce::ParameterID { parameterIDs[3], 1 },
              "Filter Resonance",
              juce::NormalisableRange<float> (0.1f, 5.0f, 0.0f, 0.25f),
              0.7f))
    {
    }

    juce::AudioParameterChoice& type;
    Parameter& gain;
    Parameter& cutoff;
    Parameter& Q;
};

struct EQParameters
{
    static auto getStartFreq (int filterNb, int numFilters)
    {
        return 20.0f + juce::mapToLog10 (((float) (filterNb + 1)) / (float) numFilters, 20.0f, 15000.0f);
    }

    static auto getFilterParamID (int filterNb) noexcept
    {
        switch (filterNb)
        {
            case 0:
                return std::vector<const char*> { ID::eqFilterType0, ID::eqFilterGain0, ID::eqFilterCutoff0, ID::eqFilterQ0 };
            case 1:
                return std::vector<const char*> { ID::eqFilterType1, ID::eqFilterGain1, ID::eqFilterCutoff1, ID::eqFilterQ1 };
            case 2:
                return std::vector<const char*> { ID::eqFilterType2, ID::eqFilterGain2, ID::eqFilterCutoff2, ID::eqFilterQ2 };

                // default : std::vector<const char *> {ID::eqFilterType0,
                // ID::eqFilterGain0, ID::eqFilterCutoff0, ID::eqFilterQ0};
        }
    }

    template <typename T>
    explicit EQParameters (T& layout)
        : filter0 (addToLayout<juce::AudioProcessorParameterGroup> (
                       layout,
                       "eqFilter0",
                       "EQ Filter 0",
                       "|"),
                   getFilterParamID (0),
                   getStartFreq (0, 3))
        , filter1 (addToLayout<juce::AudioProcessorParameterGroup> (
                       layout,
                       "eqFilter1",
                       "EQ Filter 1",
                       "|"),
                   getFilterParamID (1),
                   getStartFreq (1, 3))
        , filter2 (addToLayout<juce::AudioProcessorParameterGroup> (
                       layout,
                       "eqFilter2",
                       "EQ Filter 2",
                       "|"),
                   getFilterParamID (2),
                   getStartFreq (2, 3))
    {
    }

    FilterParameters filter0;
    FilterParameters filter1;
    FilterParameters filter2;
};

struct PostProcessorParameters
{
    template <typename T>
    explicit PostProcessorParameters (T& layout)
        : eq (addToLayout<juce::AudioProcessorParameterGroup> (layout, "eq", "EQ", "|"))
    {
    }

    EQParameters eq;
};

struct Parameters

{
    explicit Parameters (
        juce::AudioProcessorValueTreeState::ParameterLayout& layout)
        : postProcessor (addToLayout<juce::AudioProcessorParameterGroup> (
            layout,
            "postProcessor",
            "Post Processor",
            "|"))
        , neural (addToLayout<juce::AudioProcessorParameterGroup> (
              layout,
              "neural",
              "Neural Processor",
              "|"))

        , gain (addToLayout<Parameter> (
              layout,
              ID::gain,
              "Gain",
              juce::NormalisableRange<float> (-10.0f, 0.0f),
              0.0f,
              getDbAttributes()))
        , reverbWet (addToLayout<Parameter> (
              layout,
              ID::reverbDryWet,
              "Dry / Wet",
              juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
              1.0f))
        , phaserWet (addToLayout<Parameter> (
              layout,
              ID::phaserWet,
              "Dry / Wet",
              juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
              1.0f))
    {
    }

    PostProcessorParameters postProcessor;
    NeuralParameters neural;
    Parameter& gain;
    Parameter& reverbWet;
    Parameter& phaserWet;
};

// struct State
// {
//     explicit State(juce::AudioProcessorValueTreeState::ParameterLayout&
//     layout, juce::AudioProcessor& p) : parameterRefs { layout },
//       apvts (p, nullptr, "Plugin", std::move(layout)){ }

//     Parameters parameterRefs;
//     juce::AudioProcessorValueTreeState apvts;
// };