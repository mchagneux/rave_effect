#pragma once 
#include <torch/torch.h>
#include <torch/script.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_data_structures/juce_data_structures.h>
#include "./CircularBuffer.h"

struct RaveProcessor : private juce::ValueTree::Listener, public juce::Thread
{
public:
    RaveProcessor(const juce::ValueTree& state); 
    ~RaveProcessor();



    void run(); 
    void process(juce::dsp::ProcessContextReplacing<float> context);
    void prepare(const juce::dsp::ProcessSpec& context); 
    void reset();
    bool loadModel(); 
    torch::Tensor sample_prior(const int n_steps, const float temperature);
    torch::Tensor encode(const torch::Tensor input);

    std::vector<torch::Tensor> encode_amortized(const torch::Tensor input);

    torch::Tensor decode(const torch::Tensor input);
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
    void writeLatentBuffer(at::Tensor latent);
    bool hasPrior();
    bool isStereo() const;
    at::Tensor getLatentBuffer();
    bool hasMethod(const std::string& method_name) const;


private: 

    void valueTreePropertyChanged (juce::ValueTree &treeWhosePropertyHasChanged, const juce::Identifier &property); 
    void valueTreeChildAdded (juce::ValueTree &, juce::ValueTree &) {} 
    void valueTreeChildRemoved (juce::ValueTree &, juce::ValueTree &, int ) {}
    void valueTreeChildOrderChanged (juce::ValueTree &, int , int ) {}
    void valueTreeParentChanged (juce::ValueTree &) {}
    void valueTreeRedirected (juce::ValueTree &) {}

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
    juce::AudioBuffer<float> monoBuffer {1, 2048}; 
    juce::AudioBuffer<float> raveInputBuffer{1, 512}; 
    juce::AudioBuffer<float> raveOutputBuffer {1, 512}; 
};


