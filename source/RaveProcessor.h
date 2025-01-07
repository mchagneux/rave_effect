#pragma once 
#include <torch/torch.h>
#include <torch/script.h>
#include <juce_dsp/juce_dsp.h>


struct RaveProcessor 
{
public:
    RaveProcessor(); 
    ~RaveProcessor();

    void process(juce::dsp::ProcessContextReplacing<float> context);
    void prepare(const juce::dsp::ProcessSpec& context); 
    void reset();
    bool loadModel(const std::string& rave_model_file); 
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
    torch::jit::Module model; 
    int modelSampleRate;
    int latent_size;
    bool has_prior = false;
    bool stereo = false;
    juce::String model_path;
    at::Tensor encode_params;
    at::Tensor decode_params;
    at::Tensor prior_params;
    at::Tensor latent_buffer;
    std::vector<torch::jit::IValue> inputs_rave;
    juce::Range<float> validBufferSizeRange;
};


