/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
SoutWaverTestAudioProcessorEditor::SoutWaverTestAudioProcessorEditor(SoutWaverTestAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	backgroundImage = ImageCache::getFromMemory(BinaryData::FOLDER_GUI_png, BinaryData::FOLDER_GUI_pngSize);
	setSize(720 * guiScaleFactor, 1000 * guiScaleFactor);

	// 唉好不容易写的分贝条条结果用不上
	// addAndMakeVisible(horizontalMeterLeft);
	// addAndMakeVisible(horizontalMeterRight);

	// 失真旋钮
	distortionKnob.setSliderStyle(Slider::SliderStyle::Rotary);
	distortionKnob.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
	distortionKnob.setRange(0.0, 1.0);
	distortionKnob.setSkewFactor(1.0);
	distortionKnob.setVelocityBasedMode(true);
	distortionKnob.setLookAndFeel(&sliderLookAndFeel);
	distortionSliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Distortion", distortionKnob);

	outputGainKnob.setSliderStyle(Slider::SliderStyle::Rotary);
	outputGainKnob.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
	outputGainKnob.setRange(0.0, 1.0);
	outputGainKnob.setSkewFactor(1.0);
	outputGainKnob.setVelocityBasedMode(true);
	outputGainKnob.setLookAndFeel(&sliderLookAndFeel);
	outputGainSliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Output", outputGainKnob);

	mixKnob.setSliderStyle(Slider::SliderStyle::Rotary);
	mixKnob.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
	mixKnob.setRange(0.0, 1.0);
	mixKnob.setSkewFactor(1.0);
	mixKnob.setVelocityBasedMode(true);
	mixKnob.setLookAndFeel(&sliderLookAndFeel);
	mixSliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "Mix", mixKnob);

	// TODO: 删掉这依托大粪
	auto distortionKnobSize = 384.0f * guiScaleFactor;
	auto distortionKnobPosX = getWidth() / 2.0f - distortionKnobSize / 2.0f;
	auto distortionKnobPosY = 431.0f * guiScaleFactor - 173.0f / 2.0f;
	distortionKnob.setBounds(distortionKnobPosX, distortionKnobPosY,
		distortionKnobSize, distortionKnobSize);
	originalDistortionBounds = distortionKnob.getBounds();

	auto outputKnobSize = 200.0f * guiScaleFactor;
	auto outputKnobPosX = 183.0f * guiScaleFactor - outputKnobSize / 2.0f - 7.0f * guiScaleFactor;
	auto outputKnobPosY = 802.0f * guiScaleFactor - outputKnobSize / 2.0f;
	outputGainKnob.setBounds(outputKnobPosX, outputKnobPosY,
		outputKnobSize, outputKnobSize);
	originalOutputGainBounds = outputGainKnob.getBounds();

	auto mixKnobSize = 200.0f * guiScaleFactor;
	auto mixKnobPosX = 566.0f * guiScaleFactor - mixKnobSize / 2.0f - 18.0f * guiScaleFactor;
	auto mixKnobPosY = 800.0f * guiScaleFactor - mixKnobSize / 2.0f;
	mixKnob.setBounds(mixKnobPosX, mixKnobPosY,
		mixKnobSize, mixKnobSize);
	originalMixBounds = mixKnob.getBounds();

	addAndMakeVisible(distortionKnob);
	addAndMakeVisible(outputGainKnob);
	addAndMakeVisible(mixKnob);

	menu.addItem(1, "50%", true, guiScaleFactor == 0.5f);
	menu.addItem(2, "100%", true, guiScaleFactor == 1.0f);
	menu.addItem(3, "150%", true, guiScaleFactor == 1.5f);
	menu.addItem(4, "200%", true, guiScaleFactor == 2.0f);
	menu.addSeparator();
	menu.addItem(5, "Custom Scale...");

#if JUCE_VERSION >= 0x60000
	setResizable(true, true);
	getConstrainer()->setFixedAspectRatio(720.0f / 1000.0f);
#endif

	startTimerHz(24);
}

SoutWaverTestAudioProcessorEditor::~SoutWaverTestAudioProcessorEditor()
{
}

void SoutWaverTestAudioProcessorEditor::timerCallback()
{
	horizontalMeterLeft.setLevel(audioProcessor.getRMS(0));
	horizontalMeterRight.setLevel(audioProcessor.getRMS(1));

	horizontalMeterLeft.repaint();
	horizontalMeterRight.repaint();
}

//==============================================================================
void SoutWaverTestAudioProcessorEditor::paint(juce::Graphics& g)
{
	if (backgroundImage.isValid())
	{
		g.drawImageWithin(backgroundImage, 0, 0, getWidth(), getHeight(), RectanglePlacement::stretchToFit);
	}
	else
	{
		g.fillAll(Colours::darkgrey);
		DBG("Failed to load embedded image!");
	}
}

void SoutWaverTestAudioProcessorEditor::resized()
{
	horizontalMeterLeft.setBounds(getWidth() / 2 - 200, getHeight() - 40, 200, 30);
	horizontalMeterRight.setBounds(getWidth() / 2 - 200, getHeight() - 50, 200, 30);

	auto bounds = this->getBounds();
	auto windowWidth = bounds.getWidth();
	auto originalHeight = 720.0f * guiScaleFactor;
	auto originalWidth = 1000.0f * guiScaleFactor;
	auto scaleFactor = windowWidth / 360.f;

	distortionKnob.setBounds(
		originalDistortionBounds.getX() * scaleFactor, originalDistortionBounds.getY() * scaleFactor,
		384.0f * guiScaleFactor * scaleFactor, 384.0f * guiScaleFactor * scaleFactor);

	outputGainKnob.setBounds(
		originalOutputGainBounds.getX() * scaleFactor, originalOutputGainBounds.getY() * scaleFactor,
		200.f * guiScaleFactor * scaleFactor, 200.f * guiScaleFactor * scaleFactor);

	mixKnob.setBounds(
		originalMixBounds.getX() * scaleFactor, originalMixBounds.getY() * scaleFactor,
		200.f * guiScaleFactor * scaleFactor, 200.f * guiScaleFactor * scaleFactor);
}
