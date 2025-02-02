/*
  ==============================================================================

	SoutLookAndFeel.cpp
	Created: 31 Jan 2025 5:35:04pm
	Author:  sout

  ==============================================================================
*/


#include <JuceHeader.h>

class SoutLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        // 计算控件中心点
        const juce::Point<float> center(x + width * 0.5f, y + height * 0.5f);

        // 根据控件尺寸动态计算thumb参数
        const float baseSize = juce::jmin(width, height); // 取控件短边为基准
        const float thumbWidth = baseSize * 0.03f;  // 宽度为控件尺寸的3%
        const float thumbLength = baseSize * 0.08f; // 长度为控件尺寸的15%
        const float thumbOffset = baseSize * 0.25f; // 距离中心的偏移量

        // 计算旋转角度
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // 创建长方形Thumb路径（从中心向外延伸）
        juce::Path thumb;
        thumb.addRectangle(
            -thumbWidth * 0.5f,   // X居中
            -thumbOffset,         // 起点位置（中心上方偏移）
            thumbWidth,
            thumbLength           // 向边缘延伸的长度
        );

        // 设置颜色（增加透明度）
        g.setColour(slider.findColour(juce::Slider::thumbColourId).withAlpha(0.9f));

        // 应用变换（先平移后旋转）
        g.addTransform(juce::AffineTransform::rotation(angle, center.x, center.y));
        g.fillPath(thumb, juce::AffineTransform::translation(center.x, center.y));
    }

    SoutLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
    }
};