#pragma once
#include "PluginProcessor.h"
#include "Displays.h"

struct PanelBase : juce::Component
{
    explicit PanelBase(juce::String t) : title(std::move(t)) {}

    void paint(juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        g.setColour(theme::panel);
        g.fillRoundedRectangle(b, 8.f);
        g.setColour(theme::border);
        g.drawRoundedRectangle(b.reduced(0.5f), 8.f, 1.f);
        g.setColour(theme::cyan);
        g.setFont(juce::Font(juce::FontOptions(13.f, juce::Font::bold)));
        g.drawText(title, 12, 6, 200, 18, juce::Justification::left);
    }

    juce::String title;
};

class OscPanel : public PanelBase
{
public:
    OscPanel(PotatoBassProcessor& p, const juce::String& name, const char* tbl, const char* oct,
             const char* fine, const char* lvl)
        : PanelBase(name), wave(p.wavetables, p.apvts, tbl),
          kOct(p.apvts, oct, "Octave"), kFine(p.apvts, fine, "Fine"), kLevel(p.apvts, lvl, "Level")
    {
        combo.addItemList(Wavetables::names(), 1);
        comboAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, tbl, combo);
        for (auto* c : std::initializer_list<juce::Component*> { &combo, &wave, &kOct, &kFine, &kLevel })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(10);
        b.removeFromTop(20);
        auto left = b.removeFromLeft(190);
        combo.setBounds(left.removeFromTop(26));
        left.removeFromTop(6);
        wave.setBounds(left);
        b.removeFromLeft(10);
        const int w = b.getWidth() / 3;
        kOct.setBounds(b.removeFromLeft(w));
        kFine.setBounds(b.removeFromLeft(w));
        kLevel.setBounds(b);
    }

private:
    juce::ComboBox combo;
    WaveDisplay wave;
    Knob kOct, kFine, kLevel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAtt;
};

class EnvPanel : public PanelBase
{
public:
    explicit EnvPanel(PotatoBassProcessor& p)
        : PanelBase("AMP ENVELOPE"), display(p.apvts),
          kA(p.apvts, ID::attack, "Attack"), kD(p.apvts, ID::decay, "Decay"),
          kS(p.apvts, ID::sustain, "Sustain"), kR(p.apvts, ID::release, "Release")
    {
        for (auto* c : std::initializer_list<juce::Component*> { &display, &kA, &kD, &kS, &kR })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(10);
        b.removeFromTop(20);
        auto knobs = b.removeFromBottom(90);
        display.setBounds(b.withTrimmedBottom(6));
        const int w = knobs.getWidth() / 4;
        kA.setBounds(knobs.removeFromLeft(w));
        kD.setBounds(knobs.removeFromLeft(w));
        kS.setBounds(knobs.removeFromLeft(w));
        kR.setBounds(knobs);
    }

private:
    ADSRDisplay display;
    Knob kA, kD, kS, kR;
};

class FilterPanel : public PanelBase
{
public:
    explicit FilterPanel(PotatoBassProcessor& p)
        : PanelBase("FILTER"), display(p.apvts),
          kCutoff(p.apvts, ID::fCutoff, "Cutoff"), kRes(p.apvts, ID::fRes, "Resonance"),
          kVel(p.apvts, ID::fVel, "Velocity"), kMix(p.apvts, ID::fMix, "Mix")
    {
        combo.addItemList(filterTypeNames, 1);
        comboAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, ID::fType, combo);
        for (auto* c : std::initializer_list<juce::Component*> { &combo, &display, &kCutoff, &kRes, &kVel, &kMix })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(10);
        auto top = b.removeFromTop(24);
        combo.setBounds(top.removeFromRight(130));
        auto knobs = b.removeFromBottom(90);
        display.setBounds(b.withTrimmedBottom(6));
        const int w = knobs.getWidth() / 4;
        kCutoff.setBounds(knobs.removeFromLeft(w));
        kRes.setBounds(knobs.removeFromLeft(w));
        kVel.setBounds(knobs.removeFromLeft(w));
        kMix.setBounds(knobs);
    }

private:
    juce::ComboBox combo;
    FilterDisplay display;
    Knob kCutoff, kRes, kVel, kMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAtt;
};

class DistPanel : public PanelBase
{
public:
    explicit DistPanel(PotatoBassProcessor& p)
        : PanelBase("DISTORTION"), display(p.apvts),
          drive(p.apvts, ID::dDrive, "Drive"),
          gain(p.apvts, ID::dGain, "Gain"), release(p.apvts, ID::dRelease, "Release")
    {
        combo.addItemList(distTypeNames, 1);
        comboAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, ID::dType, combo);
        for (auto* c : std::initializer_list<juce::Component*> { &combo, &display, &drive, &gain, &release })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(10);
        auto top = b.removeFromTop(24);
        combo.setBounds(top.removeFromRight(130));
        display.setBounds(b.removeFromLeft(190));
        b.removeFromLeft(12);
        drive.setBounds(b.removeFromTop(110));
        b.removeFromTop(8);
        gain.setBounds(b.removeFromTop(34));
        release.setBounds(b.removeFromTop(34));
    }

private:
    juce::ComboBox combo;
    DistortionDisplay display;
    Knob drive;
    HSlider gain, release;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAtt;
};

class CompPanel : public PanelBase
{
public:
    explicit CompPanel(PotatoBassProcessor& p)
        : PanelBase("COMPRESSOR"), display(p.apvts, p.grMeter),
          kThresh(p.apvts, ID::cThresh, "Thresh"), kRatio(p.apvts, ID::cRatio, "Ratio"),
          kAttack(p.apvts, ID::cAttack, "Attack"), kRelease(p.apvts, ID::cRelease, "Release"),
          kKnee(p.apvts, ID::cKnee, "Knee"), kMakeup(p.apvts, ID::cMakeup, "Makeup"), kMix(p.apvts, ID::cMix, "Mix")
    {
        for (auto* c : std::initializer_list<juce::Component*> { &display, &kThresh, &kRatio, &kAttack, &kRelease, &kKnee, &kMakeup, &kMix })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(10);
        b.removeFromTop(20);
        auto knobs = b.removeFromBottom(90);
        display.setBounds(b.withTrimmedBottom(6));
        const int w = knobs.getWidth() / 7;
        for (auto* k : { &kThresh, &kRatio, &kAttack, &kRelease, &kKnee, &kMakeup })
            k->setBounds(knobs.removeFromLeft(w));
        kMix.setBounds(knobs);
    }

private:
    CompressorDisplay display;
    Knob kThresh, kRatio, kAttack, kRelease, kKnee, kMakeup, kMix;
};

class PotatoBassEditor : public juce::AudioProcessorEditor
{
public:
    explicit PotatoBassEditor(PotatoBassProcessor&);
    ~PotatoBassEditor() override { setLookAndFeel(nullptr); }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PotatoLookAndFeel laf;
    OscPanel osc1, osc2;
    EnvPanel env;
    FilterPanel filter;
    DistPanel dist;
    CompPanel comp;
};
