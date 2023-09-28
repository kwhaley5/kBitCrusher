/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BitCrusherAudioProcessorEditor::BitCrusherAudioProcessorEditor (BitCrusherAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    bitDepthAT(audioProcessor.apvts, "bitDepth", bitDepth),
    bitRateAT(audioProcessor.apvts, "bitRate", bitRate),
    mixAT(audioProcessor.apvts, "mix", mix),
    cutoffAT(audioProcessor.apvts, "cutoff", cutoff)
{
    setRotarySlider(bitDepth);
    setRotarySlider(bitRate);
    setRotarySlider(mix);
    setRotarySlider(cutoff);
    
    setSize (800, 250);
}

BitCrusherAudioProcessorEditor::~BitCrusherAudioProcessorEditor()
{
}

//==============================================================================
void BitCrusherAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void BitCrusherAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto depthBounds = bounds.removeFromLeft(bounds.getWidth() * .25);
    bitDepth.setBounds(depthBounds);

    auto rateBounds = bounds.removeFromLeft(bounds.getWidth() * .33);
    bitRate.setBounds(rateBounds);

    auto mixBounds = bounds.removeFromLeft(bounds.getWidth() * .5);
    mix.setBounds(mixBounds);

    auto cutoffBounds = bounds.removeFromLeft(bounds.getWidth());
    cutoff.setBounds(cutoffBounds);
}

void BitCrusherAudioProcessorEditor::setRotarySlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 1, 1);
    slider.setComponentID("Filter");
    addAndMakeVisible(slider);
}
