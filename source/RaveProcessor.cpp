#include "./RaveProcessor.h"
#include "./Identifiers.h"

#define MAX_LATENT_BUFFER_SIZE 32
#define BUFFER_LENGTH 32768

// using namespace torch::indexing;

// void RaveProcessor::modelPerform() {

//     c10::InferenceMode guard(true);
//     // encode
//     auto _latencyMode =  2048;
//     int input_size = static_cast<int>(pow(2, _latencyMode));

//     at::Tensor latent_traj;
//     at::Tensor latent_traj_mean;

// #if DEBUG_PERFORM
//     std::cout << "exp: " << *_latencyMode << " value: " << input_size << '\n';
//     std::cout << "has prior : " << _rave->hasPrior()
//               << "; use prior : " << *_usePrior << std::endl;
//     std::cout << "temperature : " << *_priorTemperature << std::endl;
// #endif

//     int64_t sizes = {input_size};
//     at::Tensor frame = torch::from_blob(_inModel[0].get(), sizes);
//     frame = torch::reshape(frame, {1, 1, input_size});
// #if DEBUG_PERFORM
//     std::cout << "Current input size : " << frame.sizes() << std::endl;
// #endif DEBUG_PERFORM

//     if (hasMethod("encode_amortized")) {
//         std::vector<torch::Tensor> latent_probs = encode_amortized(frame);
//         latent_traj_mean = latent_probs[0];
//         at::Tensor latent_traj_std = latent_probs[1];

//     #if DEBUG_PERFORM
//         std::cout << "mean shape" << latent_traj_mean.sizes() << std::endl;
//         std::cout << "std shape" << latent_traj_std.sizes() << std::endl;
//     #endif

//         latent_traj = latent_traj_mean +
//                         latent_traj_std * torch::randn_like(latent_traj_mean);
//         } else {
//         latent_traj = encode(frame);
//         latent_traj_mean = latent_traj;
//         }

// #if DEBUG_PERFORM
//     std::cout << "latent traj shape" << latent_traj.sizes() << std::endl;
// #endif

//     // Latent modifications
//     // apply scale and bias
//     int64_t n_dimensions =
//         std::min((int)latent_traj.size(1), (int)AVAILABLE_DIMS);
//     for (int i = 0; i < n_dimensions; i++) {
//       // The assert and casting here is needed as I got a:
//       // warning: conversion to ‘std::array<std::atomic<float>*,
//       // 8>::size_type’ {aka ‘long unsigned int’} from ‘int’ may change the
//       // sign of the result [-Wsign-conversion]
//       // Whatever AVAILABLE_DIMS type I defined
//       assert(i >= 0);
//       auto i2 = (long unsigned int)i;
//       float scale = _latentScale->at(i2)->load();
//       float bias = _latentBias->at(i2)->load();
//       latent_traj.index_put_({0, i},
//                              (latent_traj.index({0, i}) * scale + bias));
//       latent_traj_mean.index_put_(
//           {0, i}, (latent_traj_mean.index({0, i}) * scale + bias));
//     }
//     writeLatentBuffer(latent_traj_mean);

// #if DEBUG_PERFORM
//     std::cout << "scale & bias applied" << std::endl;
// #endif

//     // adding latent jitter on meaningful dimensions
//     float jitter_amount = _latentJitterValue->load();
//     latent_traj = latent_traj + jitter_amount * torch::randn_like(latent_traj);

// #if DEBUG_PERFORM
//     std::cout << "jitter applied" << std::endl;
// #endif

//     // filling missing dimensions with width parameter
//     int missing_dims = getFullLatentDimensions() - latent_traj.size(1);

// //     if (_rave->isStereo() && missing_dims > 0) {
// //       torch::Tensor latent_trajL = latent_traj,
// //                     latent_trajR = latent_traj.clone();
// //       int missing_dims = _rave->getFullLatentDimensions() - latent_trajL.size(1);
// //       float width = _widthValue->load() / 100.f;
// //       at::Tensor latent_noiseL =
// //           torch::randn({1, missing_dims, latent_trajL.size(2)});
// //       at::Tensor latent_noiseR =
// //           (1 - width) * latent_noiseL +
// //           width * torch::randn({1, missing_dims, latent_trajL.size(2)});

// //   #if DEBUG_PERFORM
// //       std::cout << "after width : " << latent_noiseL.sizes() << ";"
// //                 << latent_trajL.sizes() << std::endl;
// //   #endif

// //       latent_trajL = torch::cat({latent_trajL, latent_noiseL}, 1);
// //       latent_trajR = torch::cat({latent_trajR, latent_noiseR}, 1);

// //   #if DEBUG_PERFORM
// //       std::cout << "latent processed" << std::endl;
// //   #endif

// //       latent_traj = torch::cat({latent_trajL, latent_trajR}, 0);
// //     }

//     // Decode
//     at::Tensor out = decode(latent_traj);
//     // On windows, I don't get why, but the two first dims are swapped (compared
//     // to macOS / UNIX) with the same torch version
//     if (out.sizes()[0] == 2) {
//       out = out.transpose(0, 1);
//     }

//     const int outIndexR = (out.sizes()[1] > 1 ? 1 : 0);
//     at::Tensor outL = out.index({0, 0, at::indexing::Slice()});
//     at::Tensor outR = out.index({0, outIndexR, at::indexing::Slice()});

// #if DEBUG_PERFORM
//     std::cout << "latent decoded" << std::endl;
// #endif

//     float *outputDataPtrL, *outputDataPtrR;
//     outputDataPtrL = outL.data_ptr<float>();
//     outputDataPtrR = outR.data_ptr<float>();

//     // Write in buffers
//     assert(input_size >= 0);
//     for (size_t i = 0; i < (size_t)input_size; i++) {
//       _outModel[0][i] = outputDataPtrL[i];
//       _outModel[1][i] = outputDataPtrR[i];
//     }

//     // if (_smoothedFadeInOut.getCurrentValue() < EPSILON) {
//     //   _isMuted.store(true);
//     // }
// }

RaveProcessor::RaveProcessor (const NeuralParameters& params, juce::ValueTree& s)
    : juce::Thread ("Rave Thread")
    , parameters (params)
    , state (s)
{
    torch::jit::getProfilingMode() = false;

    c10::InferenceMode guard;
    torch::jit::setGraphExecutorOptimize (true);
    state.addListener (this);
    startThread (juce::Thread::Priority::highest);
    parameters.neuralDryWet.addListener (this);
}

void RaveProcessor::run()

{
    while (! threadShouldExit())
    {
        auto raveInputBufferView = juce::dsp::AudioBlock<float> (raveInputBuffer);
        dryWetMixer.setWetMixProportion (parameters.neuralDryWet.get());

        if (inputSamples.copyAvailableData (raveInputBufferView))
        {
            if (! isLoaded.load())
                outputSamples.addAudioData (raveInputBufferView);
            else
            {
                c10::InferenceMode guard (true);
                dryWetMixer.pushDrySamples (raveInputBufferView);

                inputs_rave[0] = torch::from_blob (raveInputBufferView.getChannelPointer (0), { 1, 1, raveInputBuffer.getNumSamples() });
                // inputs_rave.clear();
                // inputs_rave.push_back(torch::from_blob(raveInputBufferView.getChannelPointer(0), {1, 1, getModelRatio()}));

                auto output = model.forward (inputs_rave).toTensor();

                raveOutputBuffer.copyFrom (0, 0, output.data_ptr<float>(), raveOutputBuffer.getNumSamples());
                auto outputAsBufferView = juce::dsp::AudioBlock<float> (raveOutputBuffer);
                dryWetMixer.mixWetSamples (outputAsBufferView);

                outputSamples.addAudioData (outputAsBufferView);
            }
        }
    }
}

RaveProcessor::~RaveProcessor()
{
    state.removeListener (this);
    parameters.neuralDryWet.removeListener (this);
}

void RaveProcessor::valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& property)
{
    if (property == fullModelPath)
    {
        juce::MessageManager::callAsync ([&]
        {
            loadModel();
        });
    }
}

void RaveProcessor::process (juce::dsp::ProcessContextReplacing<float> context)
{
    // if(!isLoaded.load()) return;
    auto leftChannelBuffer = context.getOutputBlock().getSingleChannelBlock (0);

    inputSamples.addAudioData (leftChannelBuffer);

    auto monoBufferView = juce::dsp::AudioBlock<float> (monoBuffer);

    if (outputSamples.copyAvailableData (monoBufferView))
    {
        context.getOutputBlock().getSingleChannelBlock (0).copyFrom (monoBuffer);
        context.getOutputBlock().getSingleChannelBlock (1).copyFrom (monoBuffer);
    }

    rmsLevel.store (monoBuffer.getMagnitude (0, monoBuffer.getNumSamples()));
}

void RaveProcessor::reset()
{
    dryWetMixer.reset();
}

void RaveProcessor::prepare (const juce::dsp::ProcessSpec& spec)
{
    engineSampleRate = (int) spec.sampleRate;
    engineBlockSize = (int) spec.maximumBlockSize;
    engineNumChannels = (int) spec.numChannels;
    monoBuffer.setSize (1, engineBlockSize, false, true, true);
    inputSamples.setup (2 * engineSampleRate);
    outputSamples.setup (2 * engineSampleRate);
    dryWetMixer.prepare (spec);
    dryWetMixer.setWetLatency (4096);
}

bool RaveProcessor::loadModel()
{
    auto rave_model_file = state.getProperty (fullModelPath).toString().toStdString();
    try
    {
        c10::InferenceMode guard;
        model = torch::jit::load (rave_model_file);
    }
    catch (const c10::Error& e)
    {
        std::cerr << e.what();
        std::cerr << e.msg();
        std::cerr << "Error loading the model\n";
        return false;
    }

    auto named_buffers = model.named_buffers();
    auto named_attributes = model.named_attributes();
    has_prior = false;
    prior_params = torch::zeros ({ 0 });

    std::cout << "[ ] RAVE - Model successfully loaded: " << rave_model_file
              << std::endl;

    bool found_model_as_attribute = false;
    bool found_stereo_attribute = false;
    for (const auto& attr : named_attributes)
    {
        if (attr.name == "_rave")
        {
            found_model_as_attribute = true;
            std::cout << "Found _rave model as named attribute" << std::endl;
        }
        else if (attr.name == "stereo" || attr.name == "_rave.stereo")
        {
            found_stereo_attribute = true;
            stereo = attr.value.toBool();
            std::cout << "Stereo?" << (stereo ? "true" : "false") << std::endl;
        }
    }

    if (! found_stereo_attribute)
    {
        stereo = false;
    }

    if (found_model_as_attribute)
    {
        // Use named buffers within _rave
        for (const auto& buf : named_buffers)
        {
            if (buf.name == "_rave.sampling_rate")
            {
                modelSampleRate = buf.value.item<int>();
                std::cout << "\tSampling rate: " << modelSampleRate << std::endl;
            }
            else if (buf.name == "_rave.latent_size")
            {
                latent_size = buf.value.item<int>();
                std::cout << "\tLatent size: " << latent_size << std::endl;
            }
            else if (buf.name == "encode_params")
            {
                encode_params = buf.value;
                std::cout << "\tEncode parameters: " << encode_params
                          << std::endl;
            }
            else if (buf.name == "decode_params")
            {
                decode_params = buf.value;
                std::cout << "\tDecode parameters: " << decode_params
                          << std::endl;
            }
            else if (buf.name == "prior_params")
            {
                prior_params = buf.value;
                has_prior = true;
                std::cout << "\tPrior parameters: " << prior_params << std::endl;
            }
        }
    }
    else
    {
        // Use top-level named attributes
        for (const auto& attr : named_attributes)
        {
            if (attr.name == "sampling_rate")
            {
                modelSampleRate = attr.value.toInt();
                std::cout << "\tSampling rate: " << modelSampleRate << std::endl;
            }
            else if (attr.name == "full_latent_size")
            {
                latent_size = attr.value.toInt();
                std::cout << "\tLatent size: " << latent_size << std::endl;
            }
            else if (attr.name == "encode_params")
            {
                encode_params = attr.value.toTensor();
                std::cout << "\tEncode parameters: " << encode_params
                          << std::endl;
            }
            else if (attr.name == "decode_params")
            {
                decode_params = attr.value.toTensor();
                std::cout << "\tDecode parameters: " << decode_params
                          << std::endl;
            }
            else if (attr.name == "prior_params")
            {
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

    raveInputBuffer.setSize (1, 4096, false, true, false);
    raveOutputBuffer.setSize (1, 4096, false, true, false);

    inputs_rave.clear();
    inputs_rave.push_back (torch::ones ({ 1, 1, 4096 }));
    resetLatentBuffer();
    isLoaded.store (true);
    return true;
}

torch::Tensor RaveProcessor::sample_prior (const int n_steps, const float temperature)
{
    c10::InferenceMode guard;
    inputs_rave[0] = torch::ones ({ 1, 1, n_steps }) * temperature;
    torch::Tensor prior =
        model.get_method ("prior") (inputs_rave).toTensor();
    return prior;
}

torch::Tensor RaveProcessor::encode (const torch::Tensor input)
{
    c10::InferenceMode guard;
    inputs_rave[0] = input;
    auto y = model.get_method ("encode") (inputs_rave).toTensor();
    return y;
}

std::vector<torch::Tensor> RaveProcessor::encode_amortized (const torch::Tensor input)
{
    c10::InferenceMode guard;
    inputs_rave[0] = input;
    auto stats = model.get_method ("encode_amortized") (inputs_rave)
                     .toTuple()
                     .get()
                     ->elements();
    torch::Tensor mean = stats[0].toTensor();
    torch::Tensor std = stats[1].toTensor();
    std::vector<torch::Tensor> mean_std = { mean, std };
    return mean_std;
}

torch::Tensor RaveProcessor::decode (const torch::Tensor input)
{
    c10::InferenceMode guard;
    inputs_rave[0] = input;
    auto y = model.get_method ("decode") (inputs_rave).toTensor();
    return y;
}

juce::Range<float> RaveProcessor::getValidBufferSizes()
{
    return juce::Range<float> (getModelRatio(), BUFFER_LENGTH);
}

unsigned int RaveProcessor::getLatentDimensions()
{
    int tmp = decode_params.index ({ 0 }).item<int>();
    assert (tmp >= 0);
    return (unsigned int) tmp;
}

unsigned int RaveProcessor::getEncodeChannels()
{
    int tmp = encode_params.index ({ 0 }).item<int>();
    assert (tmp >= 0);
    return (unsigned int) tmp;
}

unsigned int RaveProcessor::getDecodeChannels()
{
    int tmp = decode_params.index ({ 3 }).item<int>();
    assert (tmp >= 0);
    return (unsigned int) tmp;
}

int RaveProcessor::getModelRatio() { return encode_params.index ({ 3 }).item<int>(); }

float RaveProcessor::zPerSeconds() { return encode_params.index ({ 3 }).item<float>() / modelSampleRate; }

int RaveProcessor::getFullLatentDimensions() { return latent_size; }

int RaveProcessor::getInputBatches() { return encode_params.index ({ 1 }).item<int>(); }

int RaveProcessor::getOutputBatches() { return decode_params.index ({ 3 }).item<int>(); }

void RaveProcessor::resetLatentBuffer() { latent_buffer = torch::zeros ({ 0 }); }

void RaveProcessor::writeLatentBuffer (at::Tensor latent)
{
    if (latent_buffer.size (0) == 0)
    {
        latent_buffer = latent;
    }
    else
    {
        latent_buffer = torch::cat ({ latent_buffer, latent }, 2);
    }
    if (latent_buffer.size (2) > MAX_LATENT_BUFFER_SIZE)
    {
        latent_buffer = latent_buffer.index (
            { "...", torch::indexing::Slice (-MAX_LATENT_BUFFER_SIZE, torch::indexing::None, torch::indexing::None) });
    }
}

bool RaveProcessor::hasPrior() { return has_prior; }

bool RaveProcessor::isStereo() const { return stereo; }

at::Tensor RaveProcessor::getLatentBuffer() { return latent_buffer; }

bool RaveProcessor::hasMethod (const std::string& method_name) const
{
    return model.find_method (method_name).has_value();
}
