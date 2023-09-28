/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "KiTiKLNF.h"

//==============================================================================
/**
*/
class BitCrusherAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    BitCrusherAudioProcessorEditor (BitCrusherAudioProcessor&);
    ~BitCrusherAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void setRotarySlider(juce::Slider&);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BitCrusherAudioProcessor& audioProcessor;

    Laf lnf;
    juce::Image logo;
    juce::Font newFont;

    std::array<Laf::LevelMeter, 2> meter;
    std::array<Laf::LevelMeter, 2> outMeter;

    juce::Slider bitDepth { "Depth" },
                 bitRate  { "Rate" },
                 mix      { "Dry/Wet" },
                 cutoff   { "Cutoff Frequency" };

    juce::AudioProcessorValueTreeState::SliderAttachment bitDepthAT, bitRateAT, mixAT, cutoffAT;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitCrusherAudioProcessorEditor)
};
