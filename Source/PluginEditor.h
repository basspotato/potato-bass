#pragma once
#include "PluginProcessor.h"
#include "Displays.h"
#include "Hud.h"

struct PanelBase : juce::Component
{
    explicit PanelBase(juce::String t, juce::AudioProcessorValueTreeState* s = nullptr, const char* onId = nullptr)
        : title(std::move(t))
    {
        if (s != nullptr)
        {
            attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(*s, onId, power);
            power.onStateChange = [this] { repaint(); };
            hasPower = true;
            addAndMakeVisible(power);
        }
    }

    void paint(juce::Graphics& g) override { theme::paintPanel(g, getLocalBounds().toFloat(), title); }

    void paintOverChildren(juce::Graphics& g) override
    {
        if (!hasPower || power.getToggleState())
            return;
        g.excludeClipRegion(power.getBounds());
        g.setColour(theme::bg.withAlpha(0.6f));
        g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(1.f), 7.f);
    }

    void resized() override { power.setBounds(getWidth() - 36, 6, 24, 20); }

    juce::String title;
    PowerButton power;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    bool hasPower = false;
};

class OscPanel : public PanelBase
{
public:
    OscPanel(PotatoBassProcessor& p, const juce::String& name, const char* tbl, const char* oct,
             const char* fine, const char* lvl, const char* on)
        : PanelBase(name, &p.apvts, on), wave(p.wavetables, p.apvts, tbl),
          kOct(p.apvts, oct, "Octave"), kFine(p.apvts, fine, "Fine"), kLevel(p.apvts, lvl, "Level")
    {
        combo.addItemList(Wavetables::names(), 1);
        comboAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, tbl, combo);
        for (auto* c : std::initializer_list<juce::Component*> { &combo, &wave, &kOct, &kFine, &kLevel })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        PanelBase::resized();
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
        : PanelBase("AMP ENVELOPE", &p.apvts, ID::eOn), display(p.apvts),
          kA(p.apvts, ID::attack, "Attack"), kD(p.apvts, ID::decay, "Decay"),
          kS(p.apvts, ID::sustain, "Sustain"), kR(p.apvts, ID::release, "Release")
    {
        for (auto* c : std::initializer_list<juce::Component*> { &display, &kA, &kD, &kS, &kR })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        PanelBase::resized();
        auto b = getLocalBounds().reduced(10);
        b.removeFromTop(20);
        auto knobs = b.removeFromBottom(104);
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
        : PanelBase("FILTER", &p.apvts, ID::fOn), display(p.apvts),
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
        PanelBase::resized();
        auto b = getLocalBounds().reduced(10);
        auto top = b.removeFromTop(24);
        top.removeFromRight(30);
        combo.setBounds(top.removeFromRight(130));
        auto knobs = b.removeFromBottom(104);
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
        : PanelBase("DISTORTION", &p.apvts, ID::dOn), display(p.apvts),
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
        PanelBase::resized();
        auto b = getLocalBounds().reduced(10);
        auto top = b.removeFromTop(24);
        top.removeFromRight(30);
        combo.setBounds(top.removeFromRight(130));
        display.setBounds(b.removeFromLeft(190));
        b.removeFromLeft(12);
        drive.setBounds(b.removeFromTop(100));
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
        : PanelBase("COMPRESSOR", &p.apvts, ID::cOn), display(p.apvts, p.grMeter),
          kThresh(p.apvts, ID::cThresh, "Thresh"), kRatio(p.apvts, ID::cRatio, "Ratio"),
          kAttack(p.apvts, ID::cAttack, "Attack"), kRelease(p.apvts, ID::cRelease, "Release"),
          kKnee(p.apvts, ID::cKnee, "Knee"), kMakeup(p.apvts, ID::cMakeup, "Makeup"), kMix(p.apvts, ID::cMix, "Mix")
    {
        for (auto* c : std::initializer_list<juce::Component*> { &display, &kThresh, &kRatio, &kAttack, &kRelease, &kKnee, &kMakeup, &kMix })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        PanelBase::resized();
        auto b = getLocalBounds().reduced(10);
        b.removeFromTop(20);
        auto knobs = b.removeFromBottom(100);
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

class LfoPanel : public PanelBase
{
public:
    explicit LfoPanel(PotatoBassProcessor& p)
        : PanelBase("LFO / DUCK", &p.apvts, ID::lOn), display(p.apvts),
          kDepth(p.apvts, ID::lDepth, "Depth"), kCurve(p.apvts, ID::lCurve, "Curve")
    {
        shape.addItemList(lfoShapeNames, 1);
        rate.addItemList(syncNames, 1);
        target.addItemList(lfoTargetNames, 1);
        shapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, ID::lShape, shape);
        rateAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, ID::lRate, rate);
        targetAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, ID::lTarget, target);
        for (auto* c : std::initializer_list<juce::Component*> { &shape, &rate, &target, &display, &kDepth, &kCurve })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        PanelBase::resized();
        auto b = getLocalBounds().reduced(10);
        b.removeFromTop(20);
        auto combos = b.removeFromLeft(130);
        shape.setBounds(combos.removeFromTop(26));
        combos.removeFromTop(5);
        rate.setBounds(combos.removeFromTop(26));
        combos.removeFromTop(5);
        target.setBounds(combos.removeFromTop(26));
        b.removeFromLeft(10);
        display.setBounds(b.removeFromLeft(b.getWidth() - 220));
        b.removeFromLeft(10);
        kDepth.setBounds(b.removeFromLeft(b.getWidth() / 2));
        kCurve.setBounds(b);
    }

private:
    juce::ComboBox shape, rate, target;
    LfoDisplay display;
    Knob kDepth, kCurve;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAtt, rateAtt, targetAtt;
};

class SeqPanel : public PanelBase
{
public:
    explicit SeqPanel(PotatoBassProcessor& p)
        : PanelBase("SEQUENCER"), on(p.apvts, ID::sOn, "On"), display(p.apvts, p.seqStep), gate(p.apvts, ID::sGate, "Gate")
    {
        rate.addItemList(seqRateNames, 1);
        rateAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.apvts, ID::sRate, rate);
        for (auto* c : std::initializer_list<juce::Component*> { &on, &rate, &gate, &display })
            addAndMakeVisible(c);
    }

    void resized() override
    {
        PanelBase::resized();
        auto b = getLocalBounds().reduced(10);
        auto top = b.removeFromTop(24);
        top.removeFromLeft(100);
        on.setBounds(top.removeFromLeft(60));
        top.removeFromLeft(10);
        rate.setBounds(top.removeFromLeft(80));
        top.removeFromLeft(20);
        gate.setBounds(top.removeFromLeft(260));
        b.removeFromTop(4);
        display.setBounds(b);
    }

private:
    ToggleBtn on;
    juce::ComboBox rate;
    SeqDisplay display;
    HSlider gate;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAtt;
};

class PanelContent;

// The editor hosts one fixed-size hardware panel and scales it to the window.
class PotatoBassEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PotatoBassEditor(PotatoBassProcessor&);
    ~PotatoBassEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int panelW = 1200, panelH = 1188;

private:
    void timerCallback() override;

    PotatoLookAndFeel laf;
    std::unique_ptr<PanelContent> panel;
};
