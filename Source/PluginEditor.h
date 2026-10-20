#pragma once
#include "PluginProcessor.h"
#include "Displays.h"
#include "Hud.h"

class PanelContent;

// The editor hosts one fixed-size hardware panel and scales it to the window.
class PotatoBassEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PotatoBassEditor(PotatoBassProcessor&);
    ~PotatoBassEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int panelW = 1200, panelH = 1184;

private:
    void timerCallback() override;

    PotatoLookAndFeel laf;
    std::unique_ptr<PanelContent> panel;
};
