#pragma once
#include <torch/torch.h>
#include <torch/script.h>
#include <JuceHeader.h>
#include "./CircularBuffer.h"
#include "./Parameters.h"

struct RaveProcessor : private juce::ValueTree::Listener
    , public juce::Thread
    , private juce::AudioProcessorParameter::Listener
{
public:
    RaveProcessor (const NeuralParameters& params, juce::ValueTree& state);
    ~RaveProcessor() override;

    void run() override;
    void process (juce::dsp::ProcessContextReplacing<float> context);
    void prepare (const juce::dsp::ProcessSpec& context);
    void reset();
    bool loadModel();
    torch::Tensor sample_prior (const int n_steps, const float temperature);
    torch::Tensor encode (const torch::Tensor input);

    std::vector<torch::Tensor> encode_amortized (const torch::Tensor input);

    torch::Tensor decode (const torch::Tensor input);
    juce::Range<float> getValidBufferSizes();
    unsigned int getLatentDimensions();
    unsigned int getEncodeChannels();
    unsigned int getDecodeChannels();
    int getModelRatio();
    float zPerSeconds();
    int getFullLatentDimensions();
    int getInputBatches();
    int getOutputBatches();
    void resetLatentBuffer();
    void writeLatentBuffer (at::Tensor latent);
    bool hasPrior();
    bool isStereo() const;
    at::Tensor getLatentBuffer();
    bool hasMethod (const std::string& method_name) const;
    void modelPerform();
    void raveProcess();
    std::atomic<float> dryWetChanged = false;
    std::atomic<float> rmsLevel = 0.0f;

private:
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property);

    void valueTreeChildAdded (juce::ValueTree&, juce::ValueTree&) {}

    void valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree&, int) {}

    void valueTreeChildOrderChanged (juce::ValueTree&, int, int) {}

    void valueTreeParentChanged (juce::ValueTree&) {}

    void valueTreeRedirected (juce::ValueTree&) {}

    void parameterValueChanged (int, float) override {}

    void parameterGestureChanged (int, bool) override {}

    const NeuralParameters& parameters;
    juce::ValueTree state;

    torch::jit::Module model;
    int modelSampleRate;
    int latent_size;
    bool has_prior = false;
    bool stereo = false;
    std::atomic<bool> isLoaded = false;

    at::Tensor encode_params;
    at::Tensor decode_params;
    at::Tensor prior_params;
    at::Tensor latent_buffer;
    juce::Range<float> validBufferSizeRange;

    std::vector<torch::IValue> inputs_rave;
    int engineSampleRate = 44100;
    int engineBlockSize = 2048;
    int engineNumChannels;

    CircularBuffer<float> inputSamples;
    CircularBuffer<float> outputSamples;
    juce::AudioBuffer<float> monoBuffer { 1, 2048 };
    juce::AudioBuffer<float> raveInputBuffer { 1, 4096 };
    juce::AudioBuffer<float> raveOutputBuffer { 1, 4096 };
    juce::dsp::DryWetMixer<float> dryWetMixer;
};
