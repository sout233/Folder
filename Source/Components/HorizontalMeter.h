/*
  ==============================================================================

    HorizontalMeter.h
    Created: 20 Jan 2025 12:20:43pm
    Author:  sout

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

namespace Gui
{
    class HorizontalMeter : public Component
    {
    public:
        HorizontalMeter()
        {
            glow.colour = Colours::blueviolet.withAlpha(0.7f);
            glow.radius = 12;

            setBorderSize(BorderSize<int>(glowRadius));
        }

        void paint(Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();

            auto bgRect = bounds.reduced(glowRadius);
            g.setColour(Colours::blueviolet.withBrightness(0.1f));
            g.fillRoundedRectangle(bgRect, 5.0f);

            auto meterWidth = bgRect.getWidth();
            auto scaledX = jmap(level, -60.0f, 6.0f, 0.0f, meterWidth);
            auto ledRect = bgRect.withWidth(scaledX);

            Path glowPath;
            glowPath.addRoundedRectangle(ledRect, 5.0f);

            glow.drawForPath(g, glowPath);

            g.setColour(Colours::white);
            g.fillPath(glowPath);
        }

        void setLevel(const float value) { level = value; }

        BorderSize<int> getBorderSize() const { return BorderSize<int>(glowRadius); }

    private:
        juce::DropShadow glow;
        const int glowRadius = 12;
        float level = -60.0f;

        void setBorderSize(BorderSize<int> border)
        {
            setInterceptsMouseClicks(false, false); // ·???±??à???ò×è?????÷
            auto newBounds = getLocalBounds().reduced(border.getTopAndBottom(), border.getLeftAndRight());
            setBounds(newBounds);
        }
    };
}