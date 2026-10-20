#include "PluginEditor.h"

using namespace look;

//==============================================================================
// Centre screen: Kicker-style bezel with tabs; each tab hosts one editable display.
class TabScreen : public juce::Component
{
public:
    explicit TabScreen(PotatoBassProcessor& p)
        : proc(p), amp(p.apvts), filt(p.apvts), dist(p.apvts), comp(p.apvts, p.grMeter), lfo(p.apvts)
    {
        const std::initializer_list<juce::Component*> screens { &amp, &filt, &dist, &comp, &lfo };
        for (auto* c : screens)
        {
            addChildComponent(c);
            c->setBounds(scr);
        }
        show(0);
    }

    void show(int v)
    {
        view = juce::jlimit(0, 4, v);
        int i = 0;
        for (auto* c : std::initializer_list<juce::Component*> { &amp, &filt, &dist, &comp, &lfo })
            c->setVisible(i++ == view);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        theme::paintBezel(g, getLocalBounds().toFloat());

        const char* names[5] { "AMP", "FILTER", "DIST", "COMP", "LFO" };
        for (int i = 0; i < 5; ++i)
        {
            const auto r = tabRect(i);
            if (i == view)
            {
                g.setColour(col::amber.withAlpha(0.28f)); g.fillRoundedRectangle(r.expanded(3), 5);
                g.setGradientFill(juce::ColourGradient(col::yellow.brighter(0.2f), 0, r.getY(), col::orange, 0, r.getBottom(), false));
                g.fillRoundedRectangle(r, 3);
                g.setColour(juce::Colour((juce::uint32) 0xff1a0b02));
            }
            else
            {
                g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff1d1713), 0, r.getY(), juce::Colour((juce::uint32) 0xff0e0b09), 0, r.getBottom(), false));
                g.fillRoundedRectangle(r, 3);
                g.setColour(col::line.withAlpha(0.4f)); g.drawRoundedRectangle(r.reduced(0.5f), 3, 1);
                g.setColour(juce::Colour((juce::uint32) 0xffc0824e));
            }
            g.setFont(label(14.5f, 0.12f));
            g.drawText(names[i], r, juce::Justification::centred, false);
        }

        g.setFont(mono(14.0f));
        glowText(g, juce::String(pb::presets[proc.currentPreset.load()].name).toUpperCase(),
                 juce::Rectangle<float>(340, 12, 202, 22), juce::Justification::centredRight, col::yellow, 0.22f);

        const char* foot[5] { "DRAG ATTACK, DECAY/SUSTAIN AND RELEASE POINTS", "DRAG THE POINT: LEFT/RIGHT = CUTOFF, UP/DOWN = RESONANCE",
                              "DRAG THE POINT: LEFT/RIGHT = DRIVE, UP/DOWN = GAIN", "DRAG THRESHOLD/MAKEUP AND RATIO POINTS", "LFO / DUCK SHAPE, SYNCED TO HOST TEMPO" };
        g.setColour(juce::Colour((juce::uint32) 0xffc48a4c)); g.setFont(mono(12.5f));
        g.drawText(foot[view], juce::Rectangle<float>(22, 312, 520, 16), juce::Justification::centredLeft, false);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        for (int i = 0; i < 5; ++i)
            if (tabRect(i).contains(e.position)) { show(i); return; }
    }

private:
    juce::Rectangle<float> tabRect(int i) const
    {
        const float w[5] { 54, 66, 60, 64, 56 };
        float x = 22;
        for (int j = 0; j < i; ++j) x += w[j] + 8;
        return { x, 14, w[i], 22 };
    }

    const juce::Rectangle<int> scr { 22, 44, 520, 262 };
    PotatoBassProcessor& proc;
    ADSRDisplay amp;
    FilterDisplay filt;
    DistortionDisplay dist;
    CompressorDisplay comp;
    LfoDisplay lfo;
    int view = 0;
};

//==============================================================================
// The whole fixed-size hardware panel (scaled by the editor).
class PanelContent : public juce::Component
{
public:
    explicit PanelContent(PotatoBassProcessor& p)
        : proc(p), screen(p),
          wave1(p.wavetables, p.apvts, ID::osc1Table), wave2(p.wavetables, p.apvts, ID::osc2Table),
          seqDisplay(p.apvts, p.seqStep)
    {
        auto& s = p.apvts;
        auto prm = [&s](const char* id) -> juce::RangedAudioParameter& { return *s.getParameter(id); };
        auto upperNames = [](const juce::StringArray& a) { juce::StringArray r; for (auto& x : a) r.add(x.toUpperCase()); return r; };

        auto knob = [&](const char* id, const char* title, float size, int cx, int top, bool seqPal = false)
        {
            auto* k = knobs.add(new KKnob(prm(id), title, size, seqPal));
            addAndMakeVisible(k);
            k->placeTop(cx, top);
        };
        auto choice = [&](const char* id, const juce::StringArray& names, const char* title, int x, int y, int w, int h, bool seqStyle = false)
        {
            auto* c = choices.add(new ChoiceWindow(prm(id), upperNames(names), title));
            c->seqStyle = seqStyle;
            addAndMakeVisible(c);
            c->setBounds(x, y, w, h);
        };
        auto power = [&](const char* id, const char* title, int x, int y, juce::Rectangle<int> dimArea)
        {
            auto* l = leds.add(new PowerLed(prm(id), title));
            l->setBounds(x, y, 20, 18);
            l->onChange = [this] { repaint(); };
            addAndMakeVisible(l);
            dims.push_back({ dimArea, l });
        };

        // left tile: oscillators + master
        choice(ID::osc1Table, Wavetables::names(), "Oscillator 1 waveform", 24, 72, 268, 28);
        wave1.setBounds(24, 104, 268, 40);
        knob(ID::osc1Oct, "OCTAVE", 84, 69, 150);
        knob(ID::osc1Fine, "FINE", 84, 158, 150);
        knob(ID::osc1Level, "LEVEL", 84, 247, 150);
        choice(ID::osc2Table, Wavetables::names(), "Oscillator 2 waveform", 24, 300, 268, 28);
        wave2.setBounds(24, 332, 268, 40);
        knob(ID::osc2Oct, "OCTAVE", 84, 69, 378);
        knob(ID::osc2Fine, "FINE", 84, 158, 378);
        knob(ID::osc2Level, "LEVEL", 84, 247, 378);
        knob(ID::master, "MASTER", 76, 158, 530);
        power(ID::o1On, "Oscillator 1 on", 270, 48, { 16, 46, 284, 226 });
        power(ID::o2On, "Oscillator 2 on", 270, 276, { 16, 274, 284, 224 });

        // centre: screen + envelope
        addAndMakeVisible(screen);
        screen.setBounds(318, 52, 564, 342);
        knob(ID::attack, "ATTACK", 100, 390, 436);
        knob(ID::decay, "DECAY", 100, 530, 436);
        knob(ID::sustain, "SUSTAIN", 100, 670, 436);
        knob(ID::release, "RELEASE", 100, 810, 436);
        power(ID::eOn, "Amp envelope on", 858, 408, { 316, 404, 572, 238 });

        // right tile: meter, LFO, distortion
        addAndMakeVisible(vu);
        vu.setBounds(915, 52, 250, 150);
        choice(ID::lShape, lfoShapeNames, "LFO shape", 908, 236, 268, 28);
        choice(ID::lRate, syncNames, "LFO rate", 908, 268, 268, 28);
        choice(ID::lTarget, lfoTargetNames, "LFO target", 908, 300, 268, 28);
        knob(ID::lDepth, "DEPTH", 80, 975, 336);
        knob(ID::lCurve, "CURVE", 80, 1105, 336);
        choice(ID::dType, distTypeNames, "Distortion type", 908, 484, 268, 28);
        knob(ID::dDrive, "DRIVE", 64, 953, 520);
        knob(ID::dGain, "GAIN", 64, 1042, 520);
        knob(ID::dRelease, "RELEASE", 64, 1131, 520);
        power(ID::lOn, "LFO on", 1160, 210, { 900, 208, 284, 244 });
        power(ID::dOn, "Distortion on", 1160, 458, { 900, 456, 284, 180 });

        // filter + compressor row
        choice(ID::fType, filterTypeNames, "Filter type", 28, 688, 140, 28);
        knob(ID::fCutoff, "CUTOFF", 64, 232, 700);
        knob(ID::fRes, "RESONANCE", 64, 332, 700);
        knob(ID::fVel, "VELOCITY", 64, 432, 700);
        knob(ID::fMix, "MIX", 64, 532, 700);
        power(ID::fOn, "Filter on", 566, 658, { 16, 656, 572, 160 });
        const char* cid[7] { ID::cThresh, ID::cRatio, ID::cAttack, ID::cRelease, ID::cKnee, ID::cMakeup, ID::cMix };
        const char* cnm[7] { "THRESH", "RATIO", "ATTACK", "RELEASE", "KNEE", "MAKEUP", "MIX" };
        for (int i = 0; i < 7; ++i)
            knob(cid[i], cnm[i], 64, 650 + 79 * i, 700);
        power(ID::cOn, "Compressor on", 1160, 658, { 604, 656, 580, 160 });

        // sequencer
        for (auto* b : { &patPrev, &patNext }) addAndMakeVisible(b);
        patPrev.setBounds(30, 868, 28, 28);
        patWin.setBounds(64, 866, 210, 32);
        patNext.setBounds(280, 868, 28, 28);
        patWin.seqStyle = true;
        patWin.getItems = [] { juce::StringArray a; for (auto& q : pb::seqPatterns) a.add(q.name); return a; };
        patWin.getCurrent = [this] { return proc.currentPattern.load(); };
        patWin.onPickItem = [this](int i) { proc.loadSeqPattern(i); patWin.repaint(); };
        patWin.setTitle("Sequencer pattern");
        addAndMakeVisible(patWin);
        patPrev.onClick = [this] { proc.loadSeqPattern((proc.currentPattern.load() + pb::numSeqPatterns - 1) % pb::numSeqPatterns); patWin.repaint(); };
        patNext.onClick = [this] { proc.loadSeqPattern((proc.currentPattern.load() + 1) % pb::numSeqPatterns); patWin.repaint(); };
        choice(ID::sRate, seqRateNames, "Sequencer rate", 340, 866, 110, 32, true);
        choice(ID::sDir, seqDirNames, "Sequencer direction", 462, 866, 130, 32, true);
        addAndMakeVisible(seqDisplay);
        seqDisplay.setBounds(24, 922, 1000, 98);
        knob(ID::sGate, "GATE", 64, 1100, 884, true);
        power(ID::sOn, "Sequencer on", 1160, 832, { 16, 830, 1168, 196 });

        // preset strip
        const int sy = 1040;
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
        nameWin.setText(juce::String(pb::presets[c].name).toUpperCase() + (proc.presetIsModified() ? juce::String(juce::CharPointer_UTF8(" \xe2\x80\xa2")) : juce::String()));
        if (proc.noteOnCounter.load() != lastCounter) { lastCounter = proc.noteOnCounter.load(); lastNoteSeen = juce::Time::getMillisecondCounterHiRes(); }
        const bool active = proc.heldNotes.load() > 0 || juce::Time::getMillisecondCounterHiRes() - lastNoteSeen < 90.0;
        midiLed.setOn(active);
        badgeLevel = active ? 1.0f : badgeLevel * 0.88f;
        badge.setActivity(badgeLevel);
        trigger.setLit(active);
        vu.setLevel(proc.meterRms.load());
        levelLights.setLevel(vu.getLevel());
        if (shownPreset != c) { shownPreset = c; screen.repaint(); }
        patWin.repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(col::page);
        g.setGradientFill(juce::ColourGradient(col::amber.withAlpha(0.07f), 600, 0, col::amber.withAlpha(0.0f), 600, 520, true));
        g.fillRect(0, 0, 1200, 1030);
        g.setGradientFill(juce::ColourGradient(col::amber.withAlpha(0.05f), 600, 1184, col::amber.withAlpha(0.0f), 600, 700, true));
        g.fillRect(0, 700, 1200, 484);

        // faint hairline frame and tick marks along the header rule
        g.setColour(col::line.withAlpha(0.10f));
        g.drawRoundedRectangle(3.5f, 3.5f, 1193.f, 1177.f, 12.f, 1.f);
        g.setColour(col::line.withAlpha(0.22f));
        for (int x = 16; x <= 1184; x += 16)
            g.fillRect((float) x, (x % 64 == 0) ? 33.f : 36.f, 1.f, (x % 64 == 0) ? 5.f : 2.f);

        theme::drawTile(g, { 12, 44, 292, 600 });    // oscillators + master
        theme::drawTile(g, { 310, 402, 580, 242 });  // amp envelope
        theme::drawTile(g, { 896, 44, 292, 600 });   // meter / LFO / distortion
        theme::drawTile(g, { 12, 652, 580, 168 });   // filter
        theme::drawTile(g, { 600, 652, 588, 168 });  // compressor
        {
            const int old = rampMode();
            rampMode() = 1;
            theme::drawTile(g, { 12, 828, 1176, 200 });  // sequencer
            theme::section(g, "SEQUENCER", 30, 834, 1140);
            g.setFont(label(15.0f, 0.14f));
            auto cap = [&](const juce::String& s, float cx, float y) { glowText(g, s, juce::Rectangle<float>(cx - 80, y, 160, 17), juce::Justification::centred, juce::Colour((juce::uint32) 0xffc2b4ff), 0.22f); };
            cap("PATTERN", 169, 900);
            cap("RATE", 395, 900);
            cap("DIRECTION", 527, 900);
            rampMode() = old;
        }
        theme::drawTile(g, { 12, 1036, 1176, 140 }); // preset strip

        g.setFont(label(29.0f, 0.38f, true));
        glowTextRamp(g, "POTATO BASS", juce::Rectangle<float>(380, 4, 440, 32), juce::Justification::centred, 0.35f);
        g.setFont(mono(12.0f)); g.setColour(juce::Colour((juce::uint32) 0xffb98c62));
        g.drawText(juce::CharPointer_UTF8("EDM BASS SYNTH  \xc2\xb7  100 PRESETS"), juce::Rectangle<float>(16, 12, 300, 16), juce::Justification::left, false);
        g.drawText(juce::CharPointer_UTF8("VST3  \xc2\xb7  WIN"), juce::Rectangle<float>(884, 12, 300, 16), juce::Justification::right, false);
        g.setFont(label(17.0f, 0.32f));

        theme::section(g, "OSCILLATOR 1", 30, 50, 256);
        theme::section(g, "OSCILLATOR 2", 30, 278, 256);
        theme::section(g, "OUTPUT", 30, 508, 256);
        theme::section(g, "AMP ENVELOPE", 330, 410, 540);
        theme::section(g, "LFO / DUCK", 914, 214, 256);
        theme::section(g, "DISTORTION", 914, 462, 256);
        theme::section(g, "FILTER", 30, 660, 544);
        theme::section(g, "COMPRESSOR", 618, 660, 552);

        g.setFont(label(15.0f, 0.14f));
        auto cap = [&](const juce::String& s, float cx, float y) { glowText(g, s, juce::Rectangle<float>(cx - 120, y, 240, 17), juce::Justification::centred, col::ink, 0.22f); };
        const float sy = 1040.f;
        cap("GENRE", 151, sy + 68);
        cap("PRESET", 356, sy + 74);
        cap("CLICK NAME TO BROWSE ALL 100", 593, sy + 68);
        cap("MIDI IN", 777, sy + 68);
        cap("TRIGGER", 1009, sy + 84);
    }

    void paintOverChildren(juce::Graphics& g) override
    {
        for (auto& d : dims)
        {
            if (d.led->isOn()) continue;
            juce::Graphics::ScopedSaveState ss(g);
            g.excludeClipRegion(d.led->getBounds());
            g.setColour(col::page.withAlpha(0.62f));
            g.fillRoundedRectangle(d.area.toFloat(), 8.f);
        }
    }

private:
    struct Dim { juce::Rectangle<int> area; PowerLed* led; };

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
    TabScreen screen;
    WaveDisplay wave1, wave2;
    SeqDisplay seqDisplay;
    juce::OwnedArray<KKnob> knobs;
    juce::OwnedArray<ChoiceWindow> choices;
    juce::OwnedArray<PowerLed> leds;
    std::vector<Dim> dims;
    VuMeter vu;
    PickWindow patWin;
    SquareButton patPrev { SquareButton::arrowLeft, "Previous pattern" }, patNext { SquareButton::arrowRight, "Next pattern" };
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
    int shownPreset = -1;
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
