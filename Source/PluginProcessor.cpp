/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BitCrusherAudioProcessor::BitCrusherAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    bitDepth = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter("bitDepth"));
    bitRate = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter("bitRate"));
    mix = dynamic_cast<juce::AudioParameterFloat*> (apvts.getParameter("mix"));
    cutoff = dynamic_cast<juce::AudioParameterFloat*> (apvts.getParameter("cutoff"));
}

BitCrusherAudioProcessor::~BitCrusherAudioProcessor()
{
}

//==============================================================================
const juce::String BitCrusherAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BitCrusherAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BitCrusherAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BitCrusherAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BitCrusherAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BitCrusherAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BitCrusherAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BitCrusherAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BitCrusherAudioProcessor::getProgramName (int index)
{
    return {};
}

void BitCrusherAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BitCrusherAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    for (auto& f : filters)
    {
        f.reset();
        f.prepare(spec);
    }
}

void BitCrusherAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BitCrusherAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BitCrusherAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    processBuffer.setSize(2, buffer.getNumSamples(), false, false, true);
    processBuffer.clear();

    auto block = juce::dsp::AudioBlock<float>(processBuffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        float* processData = processBuffer.getWritePointer(ch);
        const float* data = buffer.getReadPointer(ch);
        updateFilter(ch);

        for (int s = 0; s < processBuffer.getNumSamples(); ++s)
        {
            auto rawData = data[s];
            auto crusher = pow(2, bitDepth->get());
            auto crushedData = floor(crusher * rawData) / crusher;
            processData[s] = (filters[ch].processSample(crushedData) * mix->get());

            if (bitRate->get() >= 1)
            {
                if (s % bitRate->get() != 0)
                {
                    auto redux = processData[s - s % bitRate->get()];
                    processData[s] = (redux * mix->get());
                }
            }
        }
        buffer.addFrom(ch, 0, processBuffer, ch, 0, processBuffer.getNumSamples());
    }

}

//==============================================================================
bool BitCrusherAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BitCrusherAudioProcessor::createEditor()
{
    return new BitCrusherAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BitCrusherAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void BitCrusherAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
}

void BitCrusherAudioProcessor::updateFilter(int channel)
{
    auto coeff = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(getSampleRate(), cutoff->get());
    filters[channel].coefficients = coeff;
}

juce::AudioProcessorValueTreeState::ParameterLayout BitCrusherAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto mixRange = NormalisableRange<float>(0, 1, .01);
    auto cutoffRange = NormalisableRange<float>(100, 20000, 1);

    layout.add(std::make_unique<AudioParameterInt>("bitDepth", "bitDepth", 1, 16, 16));
    layout.add(std::make_unique<AudioParameterInt>("bitRate", "Bit Rate", 0, 50, 0));
    layout.add(std::make_unique<AudioParameterFloat>("mix", "Dry/Wet", mixRange, 1));
    layout.add(std::make_unique<AudioParameterFloat>("cutoff", "Cutoff Frequency", cutoffRange, 20000));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitCrusherAudioProcessor();
}
