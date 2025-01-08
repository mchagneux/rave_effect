#include "./RaveProcessor.h"
#include "./Identifiers.h"

#define MAX_LATENT_BUFFER_SIZE 32
#define BUFFER_LENGTH 32768
// using namespace torch::indexing;


RaveProcessor::RaveProcessor(const juce::ValueTree& s): state(s)
{

    torch::jit::getProfilingMode() = false;
    c10::InferenceMode guard;
    torch::jit::setGraphExecutorOptimize(true);
    state.addListener(this);
  
}


RaveProcessor::~RaveProcessor()
{
    state.removeListener(this);
}


void RaveProcessor::valueTreePropertyChanged (juce::ValueTree &, const juce::Identifier &property) 
{
    if (property == fullModelPath)
    {
        juce::MessageManager::callAsync([&] {loadModel();});
    }
}


void RaveProcessor::process(juce::dsp::ProcessContextReplacing<float> context)
{
    auto buffer = context.getOutputBlock(); 

    auto torchInputs = torch::from_blob(buffer.getChannelPointer(0), buffer.getNumSamples()); 

    // module.forward(torchInputs);

}

void RaveProcessor::reset()
{

}

void RaveProcessor::prepare(const juce::dsp::ProcessSpec & spec)
{

}

bool RaveProcessor::loadModel()
{
    auto rave_model_file = state.getProperty(fullModelPath).toString().toStdString();
    try {
        c10::InferenceMode guard;
        model = torch::jit::load(rave_model_file);
    } 
    catch (const c10::Error &e) {
        std::cerr << e.what();
        std::cerr << e.msg();
        std::cerr << "Error loading the model\n";
        return false;
    }

    auto named_buffers = model.named_buffers();
    auto named_attributes = model.named_attributes();
    has_prior = false;
    prior_params = torch::zeros({0});

    std::cout << "[ ] RAVE - Model successfully loaded: " << rave_model_file
              << std::endl;

    bool found_model_as_attribute = false;
    bool found_stereo_attribute = false;
    for (auto const& attr : named_attributes) {
      if (attr.name == "_rave") {
        found_model_as_attribute = true;
        std::cout << "Found _rave model as named attribute" << std::endl;
      }
      else if (attr.name == "stereo" || attr.name == "_rave.stereo") {
        found_stereo_attribute = true;
        stereo = attr.value.toBool();
        std::cout << "Stereo?" << (stereo ? "true" : "false") << std::endl;
      }
    }

    if (!found_stereo_attribute) {
      stereo = false;
    }

    if (found_model_as_attribute) {
      // Use named buffers within _rave
      for (auto const& buf : named_buffers) {
        if (buf.name == "_rave.sampling_rate") {
          modelSampleRate = buf.value.item<int>();
          std::cout << "\tSampling rate: " << modelSampleRate << std::endl;
        }
        else if (buf.name == "_rave.latent_size") {
          latent_size = buf.value.item<int>();
          std::cout << "\tLatent size: " << latent_size << std::endl;
        }
        else if (buf.name == "encode_params") {
          encode_params = buf.value;
          std::cout << "\tEncode parameters: " << encode_params
                    << std::endl;
        }
        else if (buf.name == "decode_params") {
          decode_params = buf.value;
          std::cout << "\tDecode parameters: " << decode_params
                    << std::endl;
        }
        else if (buf.name == "prior_params") {
          prior_params = buf.value;
          has_prior = true;
          std::cout << "\tPrior parameters: " << prior_params << std::endl;
        }
      }
    }
    else {
      // Use top-level named attributes
      for (auto const& attr : named_attributes) {
        if (attr.name == "sampling_rate") {
          modelSampleRate = attr.value.toInt();
          std::cout << "\tSampling rate: " << modelSampleRate << std::endl;
        }
        else if (attr.name == "full_latent_size") {
          latent_size = attr.value.toInt();
          std::cout << "\tLatent size: " << latent_size << std::endl;
        }
        else if (attr.name == "encode_params") {
          encode_params = attr.value.toTensor();
          std::cout << "\tEncode parameters: " << encode_params
            << std::endl;
        }
        else if (attr.name == "decode_params") {
          decode_params = attr.value.toTensor();
          std::cout << "\tDecode parameters: " << decode_params
            << std::endl;
        }
        else if (attr.name == "prior_params") {
          prior_params = attr.value.toTensor();
          has_prior = true;
          std::cout << "\tPrior parameters: " << prior_params << std::endl;
        }
      }
    }

    std::cout << "\tFull latent size: " << getFullLatentDimensions()
              << std::endl;
    std::cout << "\tRatio: " << getModelRatio() << std::endl;
    c10::InferenceMode guard;
    inputs_rave.clear();
    inputs_rave.push_back(torch::ones({1, 1, getModelRatio()}));
    resetLatentBuffer();
    return true; 
}



torch::Tensor RaveProcessor::sample_prior(const int n_steps, const float temperature) {
    c10::InferenceMode guard;
    inputs_rave[0] = torch::ones({1, 1, n_steps}) * temperature;
    torch::Tensor prior =
        model.get_method("prior")(inputs_rave).toTensor();
    return prior;
}

torch::Tensor RaveProcessor::encode(const torch::Tensor input) {
    c10::InferenceMode guard;
    inputs_rave[0] = input;
    auto y = model.get_method("encode")(inputs_rave).toTensor();
    return y;
}

std::vector<torch::Tensor> RaveProcessor::encode_amortized(const torch::Tensor input) {
    c10::InferenceMode guard;
    inputs_rave[0] = input;
    auto stats = model.get_method("encode_amortized")(inputs_rave)
                        .toTuple()
                        .get()
                        ->elements();
    torch::Tensor mean = stats[0].toTensor();
    torch::Tensor std = stats[1].toTensor();
    std::vector<torch::Tensor> mean_std = {mean, std};
    return mean_std;
}

torch::Tensor RaveProcessor::decode(const torch::Tensor input) {
    c10::InferenceMode guard;
    inputs_rave[0] = input;
    auto y = model.get_method("decode")(inputs_rave).toTensor();
    return y;
}

juce::Range<float> RaveProcessor::getValidBufferSizes() {
    return juce::Range<float>(getModelRatio(), BUFFER_LENGTH);
}

unsigned int RaveProcessor::getLatentDimensions() {
    int tmp = decode_params.index({0}).item<int>();
    assert(tmp >= 0);
    return (unsigned int)tmp;
}

unsigned int RaveProcessor::getEncodeChannels() {
    int tmp = encode_params.index({0}).item<int>();
    assert(tmp >= 0);
    return (unsigned int)tmp;
}

unsigned int RaveProcessor::getDecodeChannels() {
    int tmp = decode_params.index({3}).item<int>();
    assert(tmp >= 0);
    return (unsigned int)tmp;
}

int RaveProcessor::getModelRatio() { return encode_params.index({3}).item<int>(); }

float RaveProcessor::zPerSeconds() { return encode_params.index({3}).item<float>() / modelSampleRate; }

int RaveProcessor::getFullLatentDimensions() { return latent_size; }

int RaveProcessor::getInputBatches() { return encode_params.index({1}).item<int>(); }

int RaveProcessor::getOutputBatches() { return decode_params.index({3}).item<int>(); }

void RaveProcessor::resetLatentBuffer() { latent_buffer = torch::zeros({0}); }

void RaveProcessor::writeLatentBuffer(at::Tensor latent) {
    if (latent_buffer.size(0) == 0) {
        latent_buffer = latent;
    } else {
        latent_buffer = torch::cat({latent_buffer, latent}, 2);
    }
    if (latent_buffer.size(2) > MAX_LATENT_BUFFER_SIZE) {
        latent_buffer = latent_buffer.index(
            {"...", torch::indexing::Slice(-MAX_LATENT_BUFFER_SIZE, torch::indexing::None, torch::indexing::None)});
    }
}

bool RaveProcessor::hasPrior() { return has_prior; }

bool RaveProcessor::isStereo() const { return stereo; }

at::Tensor RaveProcessor::getLatentBuffer() { return latent_buffer; }

bool RaveProcessor::hasMethod(const std::string& method_name) const {
    return model.find_method(method_name).has_value();
}

