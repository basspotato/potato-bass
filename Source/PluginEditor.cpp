#include "PluginEditor.h"

PotatoBassEditor::PotatoBassEditor(PotatoBassProcessor& p)
    : AudioProcessorEditor(&p),
      osc1(p, "OSC 1", ID::osc1Table, ID::osc1Oct, ID::osc1Fine, ID::osc1Level),
      osc2(p, "OSC 2", ID::osc2Table, ID::osc2Oct, ID::osc2Fine, ID::osc2Level),
      env(p), filter(p), dist(p), comp(p)
{
    setLookAndFeel(&laf);

    addAndMakeVisible(osc1);
    addAndMakeVisible(osc2);
    addAndMakeVisible(env);
    addAndMakeVisible(filter);
    addAndMakeVisible(dist);
    addAndMakeVisible(comp);
    setSize(900, 770);
}

void PotatoBassEditor::paint(juce::Graphics& g)
{
    g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff0a1a24), 0.f, 0.f,
                                           theme::bg, (float) getWidth(), (float) getHeight(), false));
    g.fillAll();
    g.setColour(theme::accent);
    g.setFont(juce::Font(juce::FontOptions(22.f, juce::Font::bold)));
    g.drawText("POTATO BASS", 16, 8, 300, 28, juce::Justification::left);
    g.setColour(theme::cyan.withAlpha(0.4f));
    g.drawLine(16.f, 38.f, (float) getWidth() - 16.f, 38.f, 1.f);
}

void PotatoBassEditor::resized()
{
    auto b = getLocalBounds().reduced(10);
    b.removeFromTop(34);

    auto row1 = b.removeFromTop(150);
    osc1.setBounds(row1.removeFromLeft(row1.getWidth() / 2 - 4));
    row1.removeFromLeft(8);
    osc2.setBounds(row1);

    b.removeFromTop(8);
    auto row2 = b.removeFromTop(300);
    env.setBounds(row2.removeFromLeft(row2.getWidth() / 2 - 4));
    row2.removeFromLeft(8);
    filter.setBounds(row2);

    b.removeFromTop(8);
    dist.setBounds(b.removeFromLeft(b.getWidth() / 2 - 4));
    b.removeFromLeft(8);
    comp.setBounds(b);
}
