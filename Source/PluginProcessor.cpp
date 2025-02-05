/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <memory>
#include <vector>
#include <iostream>

//==============================================================================
SoutWaverTestAudioProcessor::SoutWaverTestAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
	#if ! JucePlugin_IsMidiEffect
	#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
	#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
	#endif
	), apvts(*this, nullptr, "Parameters", creatParameterLayout())
#endif
{
}

SoutWaverTestAudioProcessor::~SoutWaverTestAudioProcessor()
{
}

//==============================================================================
const juce::String SoutWaverTestAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool SoutWaverTestAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool SoutWaverTestAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool SoutWaverTestAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double SoutWaverTestAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int SoutWaverTestAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int SoutWaverTestAudioProcessor::getCurrentProgram()
{
	return 0;
}

void SoutWaverTestAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String SoutWaverTestAudioProcessor::getProgramName(int index)
{
	return {};
}

void SoutWaverTestAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void SoutWaverTestAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..
	rmsSmootherLeft.reset(sampleRate, 0.5);
	rmsSmootherRight.reset(sampleRate, 0.5);

	rmsSmootherLeft.setCurrentAndTargetValue(-100.0f);
	rmsSmootherRight.setCurrentAndTargetValue(-100.0f);

	highPass.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(
		sampleRate, 800.0f, 0.707f);
	resonator.state = juce::dsp::IIR::Coefficients<float>::makeBandPass(
		sampleRate, 3200.0f, 0.1f);
	resonator.prepare({ sampleRate, (uint32)samplesPerBlock, 2 });
}

void SoutWaverTestAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SoutWaverTestAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
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

void SoutWaverTestAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

	//DBG(g->load());
	auto currentDistortion = apvts.getRawParameterValue("Distortion")->load();
	auto currentOutput = apvts.getRawParameterValue("Output")->load();
	auto currentMix = apvts.getRawParameterValue("Mix")->load();

	// 失真参数设置
	const float drive = currentDistortion;										// 失真驱动度 (1.0-10.0)
	const float mix = currentMix;												// 干湿比 (0.0-1.0)
	const float outputLevel = Decibels::decibelsToGain(currentOutput);	// 输出衰减

	const float preEmphasis = 2.0f;
	const float harmonicBoost = 0.3f;
	const float attack = 0.999f, release = 0.999f;

	for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);

		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			float dry = channelData[sample];

			// 高频预加重
			float processed = dry * (1.0f + preEmphasis * highPass.processSample(dry));

			// 动态检测
			envelope = (dry * dry > envelope) ?
				attack * envelope + (1 - attack) * dry * dry :
				release * envelope;
			float dynamicDrive = drive * (1.0f + 2.0f * envelope);

			// 非线性处理
			float foldWet = metalClip(std::asin(std::sin(processed * 15.0f * dynamicDrive)), 0.6f);
			float softWet = std::tanh(processed * 30.0f * (1.0f - drive));

			// 谐波混合
			float wet = (1.0f - mix) * dry + mix * (
				foldWet * (1.0f - harmonicBoost) +
				softWet * harmonicBoost
				);

			channelData[sample] = wet;
		}
	}

	// 共振+滤波 声音跟放屁一样动听
	juce::dsp::AudioBlock<float> block(buffer);
	juce::dsp::ProcessContextReplacing<float> context(block);
	// 不太清楚是否有用
	resonator.process(context);

	// 输出衰减
	for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			channelData[sample] *= outputLevel;
		}
	}

	rmsSmootherLeft.skip(buffer.getNumSamples());
	rmsSmootherRight.skip(buffer.getNumSamples());

	{
		const auto value = Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
		if (value < rmsSmootherLeft.getCurrentValue())
		{
			rmsSmootherLeft.setTargetValue(value);
		}
		else
		{
			rmsSmootherLeft.setCurrentAndTargetValue(value);
		}
	}

	{
		const auto value = Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
		if (value < rmsSmootherRight.getCurrentValue())
		{
			rmsSmootherRight.setTargetValue(value);
		}
		else
		{
			rmsSmootherRight.setCurrentAndTargetValue(value);
		}
	}
}

//==============================================================================
bool SoutWaverTestAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SoutWaverTestAudioProcessor::createEditor()
{
	return new SoutWaverTestAudioProcessorEditor(*this);
}

//==============================================================================
void SoutWaverTestAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
	MemoryOutputStream stream(destData, false);
	apvts.state.writeToStream(stream);
}

void SoutWaverTestAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
	auto tree = ValueTree::readFromData(data, size_t(sizeInBytes));
	if (tree.isValid())
		apvts.state = tree;
}

float SoutWaverTestAudioProcessor::getRMS(const int channel) const
{
	jassert(channel == 0 || channel == 1);

	if (channel == 0)
		return rmsSmootherLeft.getCurrentValue();
	if (channel == 1)
		return rmsSmootherRight.getCurrentValue();

	return 0.0f;
}

AudioProcessorValueTreeState::ParameterLayout SoutWaverTestAudioProcessor::creatParameterLayout()
{
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

	params.push_back(std::make_unique<juce::AudioParameterFloat>("Distortion", "Distortion", 0.0f, 1.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("Output", "Output", -60.0f, 10.0f, 0.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("Mix", "Mix", 0.0f, 1.0f, 0.5f));

	return { params.begin(), params.end() };
}

float SoutWaverTestAudioProcessor::metalClip(float x, float threshold = 0.6f)
{
	return x > threshold ? threshold + (x - threshold) * 0.1f :
		x < -threshold ? -threshold + (x + threshold) * 0.1f : x;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new SoutWaverTestAudioProcessor();
}
