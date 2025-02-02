/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/HorizontalMeter.h"
#include "SoutLookAndFeel.cpp"

//==============================================================================
/**
*/
class SoutWaverTestAudioProcessorEditor : public juce::AudioProcessorEditor, public Timer
{
public:
	SoutWaverTestAudioProcessorEditor(SoutWaverTestAudioProcessor&);
	~SoutWaverTestAudioProcessorEditor() override;

	//==============================================================================
	void timerCallback() override;
	void paint(juce::Graphics&) override;
	void resized() override;

private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	SoutWaverTestAudioProcessor& audioProcessor;
	Image backgroundImage;

	Gui::HorizontalMeter horizontalMeterLeft, horizontalMeterRight;

	Slider distortionKnob;
	Slider outputGainKnob;
	Slider mixKnob;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> distortionSliderAttachment;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> outputGainSliderAttachment;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mixSliderAttachment;

	float guiScaleFactor = 0.5f;

	SoutLookAndFeel sliderLookAndFeel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoutWaverTestAudioProcessorEditor)
};
