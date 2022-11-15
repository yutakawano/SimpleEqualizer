/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEqualizerAudioProcessor::SimpleEqualizerAudioProcessor()
{
    UserParams[MasterBypass] = 0.0f;
    UserParams[Frequency] = 0.5664f;
    UserParams[BandWidth] = 0.393f;
    UserParams[Gain] = 0.5f;

//    UIUpdateFlag = true;//Request UI update
}

SimpleEqualizerAudioProcessor::~SimpleEqualizerAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEqualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEqualizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEqualizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEqualizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEqualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEqualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEqualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEqualizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEqualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEqualizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEqualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SimpleEqualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEqualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleEqualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // EQ Process
        float _sampleRate = getSampleRate();
        UserParams[SampleRate] = _sampleRate;

        float _frequency = 20.0f * pow(1000.0f, UserParams[Frequency]);
        float _bandWidth = 0.1f * pow(60.0f, UserParams[BandWidth]);
        float _q = 0.2f * pow(100.0, UserParams[BandWidth]);
        float _gain = 48.0f * (UserParams[Gain] - 0.5f);
        if (channel < 2) {
            //iirFilter[channel].setCoefficients(IIRCoefficients::makeLowPass(_sampleRate, _frequency));
            //iirFilter[channel].processSamples(buffer.getWritePointer(channel), buffer.getNumSamples());

            parametricEQ[channel].SetParameter(_sampleRate, _frequency, _bandWidth, _gain);
            parametricEQ[channel].DoProcess(buffer.getWritePointer(channel), buffer.getNumSamples());
        }
    }
}

//==============================================================================
bool SimpleEqualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEqualizerAudioProcessor::createEditor()
{
    return new SimpleEqualizerAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleEqualizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    //Save UserParams/Data to file
    XmlElement root("Root");
    XmlElement *el;
    el = root.createNewChildElement("Bypass");
    el->addTextElement(String(UserParams[Parameters::MasterBypass]));
    el = root.createNewChildElement("Frequency");
    el->addTextElement(String(UserParams[Parameters::Frequency]));
    el = root.createNewChildElement("BandWidth");
    el->addTextElement(String(UserParams[Parameters::BandWidth]));
    el = root.createNewChildElement("Gain");
    el->addTextElement(String(UserParams[Parameters::Gain]));
    copyXmlToBinary(root, destData);
}

void SimpleEqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    //Load UserParams/Data from file
    std::unique_ptr<XmlElement> pRoot = getXmlFromBinary(data, sizeInBytes);
    if (pRoot != NULL)
    {
        forEachXmlChildElement((*pRoot), pChild)
        {
            if (pChild->hasTagName("Bypass"))
            {
                String text = pChild->getAllSubText();
                setParameter(Parameters::MasterBypass, text.getFloatValue());
            }
            else if (pChild->hasTagName("Frequency"))
            {
                String text = pChild->getAllSubText();
                setParameter(Parameters::Frequency, text.getFloatValue());
            }
            else if (pChild->hasTagName("BandWidth"))
            {
                String text = pChild->getAllSubText();
                setParameter(Parameters::BandWidth, text.getFloatValue());
            }
            else if (pChild->hasTagName("Gain"))
            {
                String text = pChild->getAllSubText();
                setParameter(Parameters::Gain, text.getFloatValue());
            }
        }
        std::unique_ptr<XmlElement> pRoot;
    }

//    UIUpdateFlag = true;//Request UI update
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEqualizerAudioProcessor();
}

int SimpleEqualizerAudioProcessor::getNumParameters()
{
    return Parameters::totalNumParam;
}

//Get Return Filter Parameter range 0~1.0
float SimpleEqualizerAudioProcessor::getParameter(int index)
{
    if (index >= 0 && index < totalNumParam)
        return UserParams[index];
    else return 0;
}

//return parameter Name as string to HOST.
const String SimpleEqualizerAudioProcessor::getParameterName(int index)
{
    switch (index)
    {
    case MasterBypass: return "Master Bypass";
    case Frequency: return "Frequency";
    case BandWidth: return "Band Width";
    case Gain: return "Gain";
    case SampleRate: return "SampleRate";
    default:return String();
    }
}

//return parameter value as string to HOST.
const String SimpleEqualizerAudioProcessor::getParameterText(int index)
{
    switch (index)
    {
            //index == 0 文字列"EFFECT"または"BYPASS"を返す
    case Parameters::MasterBypass:
        return UserParams[MasterBypass] != 1.0f ? "EFFECT" : "BYPASS";
            //index == 1 "Frequency"の周波数値を文字列として返す 20~20000Hz
    case Parameters::Frequency:
        return String((int)(pow(1000.0f, UserParams[Frequency]) * 20.0f)) + String(" Hz");
            //index == 2 "BandWidth"の範囲値を文字列として返す "0.1~6.0octave"
    case Parameters::BandWidth:
        return String(pow(60.0f, UserParams[BandWidth]) * 0.1f, 2) + String(" Octave");
            //index == 3 "Gain"のdB値を文字列として返す "-24~24dB"
    case Parameters::Gain:
        return String((48.0f * (UserParams[Gain] - 0.5f)), 1) + String(" dB");
            //index == 4 "Samplerate"
    case Parameters::SampleRate:
        return String(UserParams[SampleRate]) + String(" Hz");

    default:return String();
    }
}

void SimpleEqualizerAudioProcessor::setParameter(int index, float newValue)
{
    if (index >= 0 && index < totalNumParam)
        UserParams[index] = newValue;
    else return;

//    UIUpdateFlag = true;//Request UI update -- Some OSX hosts use alternate editors, this updates ours
}
