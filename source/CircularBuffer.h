#pragma once
#include <fstream>
#include <juce_dsp/juce_dsp.h>

template <typename Type>
struct CircularBuffer
{
public:
    CircularBuffer() {}

    ~CircularBuffer() = default;

    void addAudioData (const juce::dsp::AudioBlock<Type>& bufferToCopyFrom)
    {
        if (abstractFifo.getFreeSpace() < bufferToCopyFrom.getNumSamples())
            return;

        int start1, block1, start2, block2;
        abstractFifo.prepareToWrite (bufferToCopyFrom.getNumSamples(), start1, block1, start2, block2);
        
        
        internalBuffer.copyFrom (0, start1, bufferToCopyFrom.getChannelPointer (0), block1);
        if (block2 > 0)
            internalBuffer.copyFrom (0, start2, bufferToCopyFrom.getSubBlock(block1).getChannelPointer(0), block2);
        abstractFifo.finishedWrite (block1 + block2);
    }


    bool copyAvailableData (juce::dsp::AudioBlock<Type>& bufferToCopyTo)
    {
        if (abstractFifo.getNumReady() >= bufferToCopyTo.getNumSamples())
        {
            int start1, block1, start2, block2;
            abstractFifo.prepareToRead (bufferToCopyTo.getNumSamples(), start1, block1, start2, block2);
            if (block1 > 0)
                bufferToCopyTo.copyFrom (internalBuffer, start1, 0, block1); // 0, 0, internalBuffer.getReadPointer (0, start1), block1);
            if (block2 > 0)
                bufferToCopyTo.copyFrom (internalBuffer, start2, block1, block2); //, internalBuffer.getReadPointer (0, start2), block2);
            abstractFifo.finishedRead (bufferToCopyTo.getNumSamples());
            return true;
        }

        return false;
    }
    void setup (int fifoSize)
    {
        internalBuffer.setSize (1, fifoSize);
        abstractFifo.setTotalSize (fifoSize);
    }



private:
    juce::AbstractFifo abstractFifo { 48000 };
    juce::AudioBuffer<float> internalBuffer; 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularBuffer)
};