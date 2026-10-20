#include "PluginEditor.h"

PotatoBassEditor::PotatoBassEditor(PotatoBassProcessor& p)
    : AudioProcessorEditor(&p),
      master(p.apvts, ID::master, "Master"),
      osc1(p, "OSC 1", ID::osc1Table, ID::osc1Oct, ID::osc1Fine, ID::osc1Level, ID::o1On),
      osc2(p, "OSC 2", ID::osc2Table, ID::osc2Oct, ID::osc2Fine, ID::osc2Level, ID::o2On),
      env(p), filter(p), dist(p), comp(p), lfo(p), seq(p)
{
    setLookAndFeel(&laf);

    addAndMakeVisible(master);
    addAndMakeVisible(osc1);
    addAndMakeVisible(osc2);
    addAndMakeVisible(env);
    addAndMakeVisible(filter);
    addAndMakeVisible(dist);
    addAndMakeVisible(comp);
    addAndMakeVisible(lfo);
    addAndMakeVisible(seq);
    setSize(900, 1010);
}

void PotatoBassEditor::paint(juce::Graphics& g)
{
    const float W = (float) getWidth(), H = (float) getHeight();
    g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff0a1822), 0.f, 0.f, theme::bg, W, H, false));
    g.fillAll();
    g.setGradientFill(juce::ColourGradient(theme::cyan.withAlpha(0.12f), 90.f, 40.f, juce::Colours::transparentBlack, 90.f, 460.f, true));
    g.fillRect(0.f, 0.f, W, 520.f);
    g.setGradientFill(juce::ColourGradient(theme::hot.withAlpha(0.10f), W - 80.f, H - 40.f, juce::Colours::transparentBlack, W - 80.f, H - 520.f, true));
    g.fillRect(0.f, H - 560.f, W, 560.f);

    g.setColour(theme::cyan.withAlpha(0.035f));
    for (float x = 0.f; x < W; x += 24.f) g.drawVerticalLine((int) x, 0.f, H);
    for (float y = 0.f; y < H; y += 24.f) g.drawHorizontalLine((int) y, 0.f, W);

    const auto title = juce::Font(juce::FontOptions(24.f, juce::Font::bold));
    g.setFont(title);
    g.setColour(theme::hot.withAlpha(0.25f));
    g.drawText("POTATO BASS", 17, 9, 300, 28, juce::Justification::left);
    g.setColour(theme::accent);
    g.drawText("POTATO BASS", 16, 8, 300, 28, juce::Justification::left);
    g.setColour(theme::cyan.withAlpha(0.8f));
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText("EDM BASS SYNTH", 202, 22, 120, 12, juce::Justification::left);

    for (int i = 0; i < 12; ++i)
    {
        const float t = (float) i / 11.f;
        const float bh = 5.f + 15.f * std::abs(std::sin(t * 5.1f + 0.7f));
        const auto col = PotatoLookAndFeel::ledColour(t);
        g.setColour(col.withAlpha(0.25f));
        g.fillRect(340.f + (float) i * 7.f - 1.f, 30.f - bh - 1.f, 6.f, bh + 2.f);
        g.setColour(col);
        g.fillRect(340.f + (float) i * 7.f, 30.f - bh, 4.f, bh);
    }

    g.setGradientFill(juce::ColourGradient(theme::cyan.withAlpha(0.0f), 16.f, 0.f, theme::cyan.withAlpha(0.6f), W * 0.5f, 0.f, false));
    g.fillRect(16.f, 38.f, W * 0.5f - 16.f, 1.f);
    g.setGradientFill(juce::ColourGradient(theme::cyan.withAlpha(0.6f), W * 0.5f, 0.f, theme::cyan.withAlpha(0.f), W - 16.f, 0.f, false));
    g.fillRect(W * 0.5f, 38.f, W * 0.5f - 16.f, 1.f);
}

void PotatoBassEditor::resized()
{
    auto b = getLocalBounds().reduced(10);
    master.setBounds(getWidth() - 300, 8, 284, 26);
    b.removeFromTop(34);

    auto row1 = b.removeFromTop(150);
    osc1.setBounds(row1.removeFromLeft(row1.getWidth() / 2 - 4));
    row1.removeFromLeft(8);
    osc2.setBounds(row1);

    b.removeFromTop(8);
    auto row2 = b.removeFromTop(270);
    env.setBounds(row2.removeFromLeft(row2.getWidth() / 2 - 4));
    row2.removeFromLeft(8);
    filter.setBounds(row2);

    b.removeFromTop(8);
    auto row3 = b.removeFromTop(b.getHeight() - 276);
    dist.setBounds(row3.removeFromLeft(row3.getWidth() / 2 - 4));
    row3.removeFromLeft(8);
    comp.setBounds(row3);

    b.removeFromTop(8);
    lfo.setBounds(b.removeFromTop(100));
    b.removeFromTop(8);
    seq.setBounds(b);
}
