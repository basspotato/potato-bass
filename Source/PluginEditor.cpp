#include "PluginEditor.h"

using namespace look;

// The whole fixed-size hardware panel (scaled by the editor).
class PanelContent : public juce::Component
{
public:
    explicit PanelContent(PotatoBassProcessor& p)
        : proc(p),
          osc1(p, "OSC 1", ID::osc1Table, ID::osc1Oct, ID::osc1Fine, ID::osc1Level, ID::o1On),
          osc2(p, "OSC 2", ID::osc2Table, ID::osc2Oct, ID::osc2Fine, ID::osc2Level, ID::o2On),
          env(p), filter(p), dist(p), comp(p), lfo(p), seq(p),
          master(p.apvts, ID::master, "MASTER")
    {
        for (auto* c : std::initializer_list<juce::Component*> { &osc1, &osc2, &env, &filter, &dist, &comp, &lfo, &seq, &vu, &master })
            addAndMakeVisible(c);

        osc1.setBounds(12, 44, 400, 156);
        osc2.setBounds(420, 44, 400, 156);
        vu.setBounds(832, 47, 250, 150);
        master.setBounds(1086, 50, 98, 146);
        env.setBounds(12, 208, 576, 270);
        filter.setBounds(596, 208, 592, 270);
        dist.setBounds(12, 486, 576, 250);
        comp.setBounds(596, 486, 592, 250);
        lfo.setBounds(12, 744, 1176, 130);
        seq.setBounds(12, 882, 1176, 150);

        const int sy = 1044;
        for (auto* b : { &gPrev, &gNext, &pPrev, &pNext }) addAndMakeVisible(b);
        gPrev.setBounds(40, sy + 28, 30, 30);  genreWin.setBounds(76, sy + 26, 150, 34); gNext.setBounds(232, sy + 28, 30, 30);
        pPrev.setBounds(290, sy + 31, 30, 30); segs.setBounds(326, sy + 26, 60, 40);     pNext.setBounds(392, sy + 31, 30, 30);
        nameWin.setBounds(458, sy + 26, 270, 34);
        midiLed.setBounds(764, sy + 30, 27, 27);
        trigger.setBounds(978, sy + 14, 62, 62);
        for (juce::Component* c : { (juce::Component*) &genreWin, (juce::Component*) &segs, (juce::Component*) &nameWin,
                                    (juce::Component*) &midiLed, (juce::Component*) &trigger })
            addAndMakeVisible(c);
        genreWin.setTitle("Genre - open preset browser");
        nameWin.setTitle("Preset name - open preset browser");
        trigger.setTitle("Trigger C1");

        gPrev.onClick = [this] { pick(((current() / 10) + 9) % 10 * 10, true); };
        gNext.onClick = [this] { pick(((current() / 10) + 1) % 10 * 10, true); };
        pPrev.onClick = [this] { pick(current() / 10 * 10 + (current() % 10 + 9) % 10, true); };
        pNext.onClick = [this] { pick(current() / 10 * 10 + (current() % 10 + 1) % 10, true); };
        genreWin.onClick = [this] { browser.show(current()); };
        nameWin.onClick  = [this] { browser.show(current()); };

        trigger.onPress = [this](bool down)
        {
            if (down) proc.keyboardState.noteOn(1, 36, 110.0f / 127.0f);
            else      proc.keyboardState.noteOff(1, 36, 0.0f);
        };

        addAndMakeVisible(levelLights);
        levelLights.setBounds(40, sy + 5, 1000, 12);
        levelLights.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(badge);
        badge.setBounds(1058, sy, 126, 134);
        badge.setInterceptsMouseClicks(false, false);

        addChildComponent(browser);
        browser.setBounds(150, 300, 900, 520);
        browser.onPick = [this](int i) { pick(i, true); };

        refresh();
    }

    void refresh()
    {
        const int c = current();
        genreWin.setText(juce::String(pb::genreNames[c / 10]).toUpperCase());
        segs.setNumber(c % 10 + 1);
        nameWin.setText(juce::String(pb::presets[c].name) + (proc.presetIsModified() ? juce::String(juce::CharPointer_UTF8(" \xe2\x80\xa2")) : juce::String()));
        if (proc.noteOnCounter.load() != lastCounter) { lastCounter = proc.noteOnCounter.load(); lastNoteSeen = juce::Time::getMillisecondCounterHiRes(); }
        const bool active = proc.heldNotes.load() > 0 || juce::Time::getMillisecondCounterHiRes() - lastNoteSeen < 90.0;
        midiLed.setOn(active);
        badgeLevel = active ? 1.0f : badgeLevel * 0.88f;
        badge.setActivity(badgeLevel);
        trigger.setLit(active);
        vu.setLevel(proc.meterRms.load());
        levelLights.setLevel(vu.getLevel());
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(col::page);
        g.setGradientFill(juce::ColourGradient(col::amber.withAlpha(0.07f), 600, 0, col::amber.withAlpha(0.0f), 600, 520, true));
        g.fillRect(0, 0, 1200, 1030);

        theme::drawTile(g, { 824, 44, 364, 156 });   // output meter + master
        theme::drawTile(g, { 12, 1040, 1176, 140 }); // preset strip

        g.setFont(label(29.0f, 0.38f, true));
        glowTextRamp(g, "POTATO BASS", juce::Rectangle<float>(380, 4, 440, 32), juce::Justification::centred, 0.35f);
        g.setFont(mono(12.0f)); g.setColour(juce::Colour((juce::uint32) 0xffb98c62));
        g.drawText(juce::CharPointer_UTF8("EDM BASS SYNTH  \xc2\xb7  100 PRESETS"), juce::Rectangle<float>(16, 12, 300, 16), juce::Justification::left, false);
        g.drawText(juce::CharPointer_UTF8("VST3  \xc2\xb7  WIN"), juce::Rectangle<float>(884, 12, 300, 16), juce::Justification::right, false);

        g.setFont(label(15.0f, 0.14f));
        auto cap = [&](const juce::String& s, float cx, float y) { glowText(g, s, juce::Rectangle<float>(cx - 120, y, 240, 17), juce::Justification::centred, col::ink, 0.22f); };
        const float sy = 1044.f;
        cap("GENRE", 151, sy + 68);
        cap("PRESET", 356, sy + 74);
        cap("CLICK NAME TO BROWSE ALL 100", 593, sy + 68);
        cap("MIDI IN", 777, sy + 68);
        cap("TRIGGER", 1009, sy + 84);
    }

private:
    int current() const { return proc.currentPreset.load(); }

    void pick(int index, bool audition)
    {
        proc.loadPreset(index);
        refresh();
        if (audition)
        {
            proc.keyboardState.noteOn(1, 36, 110.0f / 127.0f);
            juce::Component::SafePointer<PanelContent> safe(this);
            juce::Timer::callAfterDelay(450, [safe] { if (safe != nullptr) safe->proc.keyboardState.noteOff(1, 36, 0.0f); });
        }
    }

    PotatoBassProcessor& proc;
    OscPanel osc1, osc2;
    EnvPanel env;
    FilterPanel filter;
    DistPanel dist;
    CompPanel comp;
    LfoPanel lfo;
    SeqPanel seq;
    Knob master;
    VuMeter vu;
    SquareButton gPrev { SquareButton::arrowLeft, "Previous genre" }, gNext { SquareButton::arrowRight, "Next genre" },
                 pPrev { SquareButton::arrowLeft, "Previous preset" }, pNext { SquareButton::arrowRight, "Next preset" };
    LedWindow genreWin { true }, nameWin { false };
    SevenSeg segs;
    LedDot midiLed;
    TriggerButton trigger;
    PotatoBadge badge;
    LevelLights levelLights;
    float badgeLevel = 0.0f;
    PresetBrowser browser;
    unsigned lastCounter = 0;
    double lastNoteSeen = 0.0;
};

//==============================================================================
PotatoBassEditor::PotatoBassEditor(PotatoBassProcessor& p)
    : AudioProcessorEditor(&p)
{
    setLookAndFeel(&laf);
    panel = std::make_unique<PanelContent>(p);
    addAndMakeVisible(*panel);
    panel->setBounds(0, 0, panelW, panelH);

    setResizable(true, true);
    if (auto* c = getConstrainer())
    {
        c->setFixedAspectRatio((double) panelW / panelH);
        c->setSizeLimits(panelW * 5 / 10, panelH * 5 / 10, panelW * 3 / 2, panelH * 3 / 2);
    }
    setSize(panelW * 72 / 100, panelH * 72 / 100);
    startTimerHz(30);
}

PotatoBassEditor::~PotatoBassEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PotatoBassEditor::paint(juce::Graphics& g) { g.fillAll(col::page); }

void PotatoBassEditor::resized()
{
    panel->setTransform(juce::AffineTransform::scale((float) getWidth() / (float) panelW));
}

void PotatoBassEditor::timerCallback() { panel->refresh(); }
