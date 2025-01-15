#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
    , modelChooser (p.apvts.state)
    , postProcessorControls (*this, p.postProcessor) //, raveControls(*this, p.parameters.neural)

{
    juce::ignoreUnused (processorRef);
    choc::ui::WebView::Options options;
    options.enableDebugMode = true;
    options.enableDebugInspector = true;
    options.transparentBackground = true;

    webView = std::make_unique<choc::ui::WebView> (options);

    webViewHolder = choc::ui::createJUCEWebViewHolder (*webView);

    p.parameters.neural.neuralDryWet.addListener (this);
    webView->navigate ("localhost:5173");

    webView->bind ("gainUpdated", [&p] (const choc::value::ValueView& args) -> choc::value::Value
    {
        p.parameters.gain.setValueNotifyingHost (args[0].get<double>());
        return {};
    });

    webView->bind ("reverbUpdated", [&p] (const choc::value::ValueView& args) -> choc::value::Value
    {
        p.parameters.reverbWet.setValueNotifyingHost (args[0].get<double>());
        return {};
    });

    webView->bind ("phaserUpdated", [&p] (const choc::value::ValueView& args) -> choc::value::Value
    {
        p.parameters.phaserWet.setValueNotifyingHost (args[0].get<double>());
        return {};
    });

    webView->bind ("enabledUpdated", [&p] (const choc::value::ValueView& args) -> choc::value::Value
    {
        p.parameters.neural.neuralEnabled.setValueNotifyingHost (args[0].get<bool>());
        return {};
    });

    // webView->evaluateJavascript("updateValue(" + juce::String((int) (100 * (p.parameters.neural.neuralDryWet.get()))).toStdString() + ");");

    postProcessorControls.eq.handle0.sendInitialUpdates();
    postProcessorControls.eq.handle1.sendInitialUpdates();
    postProcessorControls.eq.handle2.sendInitialUpdates();

    addAndMakeVisible (*webViewHolder);
    addAndMakeVisible (modelChooser);
    addAndMakeVisible (postProcessorControls);
    // addAndMakeVisible(raveControls);
    setSize (640, 360);
    setResizable (true, true);

    startTimer (20);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    auto newValue = processorRef.raveProcessor.rmsLevel.load();
    webView->evaluateJavascript ("updateValue(" + juce::String ((int) 100 * newValue).toStdString() + ");");
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    processorRef.parameters.neural.neuralDryWet.removeListener (this);
}

void AudioPluginAudioProcessorEditor::parameterValueChanged (int parameterIndex, float newValue)
{
    // if (parameterIndex == processorRef.parameters.neural.neuralDryWet.getParameterIndex())
    // {
    //     webView->evaluateJavascript("updateValue(" + juce::String((int) 100*newValue).toStdString() + ");");
    // }
}

void AudioPluginAudioProcessorEditor::parameterGestureChanged (int, bool)
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // auto topArea = area.removeFromTop((int) (getLocalBounds().toFloat().getHeight() / 6.0f));
    auto areaLeft = area.removeFromLeft ((int) (area.getWidth() / 2));

    auto topLeft = areaLeft.removeFromTop ((int) (getLocalBounds().toFloat().getHeight() / 6.0f));

    modelChooser.setBounds (topLeft.reduced ((int) (topLeft.getWidth() / 4), (int) (topLeft.getHeight() / 6)));
    postProcessorControls.setBounds (areaLeft);

    // raveControls.setBounds(topArea);

    // auto postProcessorArea = areaLeft

    webViewHolder->setBounds (area);

    // auto modelChooserArea = raveArea.removeFromBottom((int) (area.toFloat().getHeight() / 6.0f));
    // modelChooser.setBounds(modelChooserArea);

    // auto raveDryWetSliderArea = raveArea.removeFromBottom((int) (raveArea.toFloat().getHeight() / 6.0f));
}
