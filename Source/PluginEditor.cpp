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
    setLookAndFeel(&lnf);

    addAndMakeVisible(meter[0]);
    addAndMakeVisible(meter[1]);
    addAndMakeVisible(outMeter[0]);
    addAndMakeVisible(outMeter[1]);

    setRotarySlider(bitDepth);
    setRotarySlider(bitRate);
    setRotarySlider(mix);
    setRotarySlider(cutoff);
    
    setSize (800, 250);

    startTimerHz(24);
}

BitCrusherAudioProcessorEditor::~BitCrusherAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void BitCrusherAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    auto grad = juce::ColourGradient::ColourGradient(juce::Colour(186u, 34u, 34u), bounds.toFloat().getBottomLeft(), juce::Colour(186u, 34u, 34u), bounds.toFloat().getTopRight(), false);
    grad.addColour(.5f, juce::Colours::transparentBlack);

    g.setGradientFill(grad);
    g.fillAll();

    auto inputMeter = bounds.removeFromLeft(bounds.getWidth() * .125);
    auto meterLSide = inputMeter.removeFromLeft(inputMeter.getWidth() * .5);
    meter[0].setBounds(meterLSide);
    meter[1].setBounds(inputMeter);

    auto outputMeter = bounds.removeFromRight(bounds.getWidth() * .14);
    auto outMeterLSide = outputMeter.removeFromLeft(outputMeter.getWidth() * .5);
    outMeter[0].setBounds(outMeterLSide);
    outMeter[1].setBounds(outputMeter);

    g.setColour(juce::Colours::white);

    auto infoSpace = bounds.removeFromTop(bounds.getHeight() * .2);
    infoSpace.removeFromRight(infoSpace.getWidth() * .1);
    
    auto logoSpace = infoSpace.removeFromLeft(bounds.getWidth() * .4);
    auto textSpace = infoSpace.removeFromRight(bounds.getWidth() * .4);

    //add logo
    logo = juce::ImageCache::getFromMemory(BinaryData::KITIK_LOGO_NO_BKGD_png, BinaryData::KITIK_LOGO_NO_BKGD_pngSize);
    g.drawImage(logo, infoSpace.toFloat(), juce::RectanglePlacement::fillDestination);

    //Add Text
    auto newFont = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::offshore_ttf, BinaryData::offshore_ttfSize));
    g.setColour(juce::Colours::whitesmoke);
    g.setFont(newFont);
    g.setFont(30.f);
    g.drawFittedText("Simple", logoSpace, juce::Justification::centredRight, 1);
    g.drawFittedText("BitCrusher", textSpace, juce::Justification::centredLeft, 1);
}

void BitCrusherAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto inputMeter = bounds.removeFromLeft(bounds.getWidth() * .125);
    auto meterLSide = inputMeter.removeFromLeft(inputMeter.getWidth() * .5);
    meter[0].setBounds(meterLSide);
    meter[1].setBounds(inputMeter);

    auto outputMeter = bounds.removeFromRight(bounds.getWidth() * .14);
    auto outMeterLSide = outputMeter.removeFromLeft(outputMeter.getWidth() * .5);
    outMeter[0].setBounds(outMeterLSide);
    outMeter[1].setBounds(outputMeter);

    auto logoSpace = bounds.removeFromTop(bounds.getHeight() * .2);

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

void BitCrusherAudioProcessorEditor::timerCallback()
{
    //these get our rms level, and the set level function tells you how much of the rect you want
    for (auto channel = 0; channel < audioProcessor.getTotalNumInputChannels(); channel++) {
        meter[channel].setLevel(audioProcessor.getRMS(channel));
        meter[channel].repaint();

        outMeter[channel].setLevel(audioProcessor.getOutRMS(channel));
        outMeter[channel].repaint();
    }
}