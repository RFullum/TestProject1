/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TestProject1AudioProcessor::TestProject1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif

playbackFreq(0.0f)

{
}

TestProject1AudioProcessor::~TestProject1AudioProcessor()
{
}

//==============================================================================
const juce::String TestProject1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TestProject1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TestProject1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TestProject1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TestProject1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TestProject1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TestProject1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void TestProject1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TestProject1AudioProcessor::getProgramName (int index)
{
    return {};
}

void TestProject1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TestProject1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    saw1.setSampleRate      ( sampleRate );
    phasor.setSampleRate    ( sampleRate );
    sparseLFO.setSampleRate ( sampleRate );
}

void TestProject1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TestProject1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TestProject1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    int numSamples     = buffer.getNumSamples();
    auto* leftChannel  = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // Block Variables
    float phasorFreq = 0.05f;
    float baseFreq   = 55.0f;
    float sparseFreq = 1.0f;
    float pw         = 0.33f;

    playbackFreq = baseFreq;

    phasor.setFrequency     ( phasorFreq );
    sparseLFO.setFrequency  ( sparseFreq );
    sparseLFO.setPulseWidth ( pw );

    // DSP!!!
    for (int i = 0; i < numSamples; i++)
    {
        float freqMultSample = phasor.process();

        playbackFreq += freqMultSample;

        if (playbackFreq > playbackFreq * 4.0f)
        {
            playbackFreq = baseFreq;
        }

        saw1.setFrequency ( playbackFreq );

        // Process saw1 wave and gainLFO
        float sample  = saw1.process();
        float gainLFO = sparseLFO.process();

        // Output
        leftChannel[i]  = sample * gainLFO;
        rightChannel[i] = sample * gainLFO;
    }
}

//==============================================================================
bool TestProject1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TestProject1AudioProcessor::createEditor()
{
    return new TestProject1AudioProcessorEditor (*this);
}

//==============================================================================
void TestProject1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TestProject1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TestProject1AudioProcessor();
}
