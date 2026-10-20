#include "PluginEditor.h"

using namespace look;

//==============================================================================
// The whole fixed-size hardware panel (scaled by the editor).
class PanelContent : public juce::Component
{
public:
    explicit PanelContent(PotatoBassProcessor& p)
        : proc(p),
          wave1(p.wavetables, p.apvts, ID::osc1Table), wave2(p.wavetables, p.apvts, ID::osc2Table),
          ampD(p.apvts), filtD(p.apvts), distD(p.apvts), compD(p.apvts, p.grMeter), lfoD(p.apvts),
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
        auto power = [&](const char* id, const char* title, juce::Rectangle<int> tile, bool seqStyle = false)
        {
            auto* l = leds.add(new PowerToggle(prm(id), title, seqStyle));
            l->setBounds(tile.getRight() - 112, tile.getY() + 5, 100, 26);
            l->onChange = [this] { repaint(); };
            addAndMakeVisible(l);
            dims.push_back({ tile, l });
        };
        auto place = [this](juce::Component& comp, juce::Rectangle<int> r) { addAndMakeVisible(comp); comp.setBounds(r); bezels.push_back(r); };

        // row 1: oscillators and output
        for (int o = 0; o < 2; ++o)
        {
            const int ox = o == 0 ? 12 : 400;
            choice(o == 0 ? ID::osc1Table : ID::osc2Table, Wavetables::names(), o == 0 ? "Oscillator 1 waveform" : "Oscillator 2 waveform", ox + 12, 74, 356, 28);
            place(o == 0 ? (juce::Component&) wave1 : (juce::Component&) wave2, { ox + 12, 108, 356, 84 });
            knob(o == 0 ? ID::osc1Oct : ID::osc2Oct, "OCTAVE", 84, ox + 71, 200);
            knob(o == 0 ? ID::osc1Fine : ID::osc2Fine, "FINE", 84, ox + 190, 200);
            knob(o == 0 ? ID::osc1Level : ID::osc2Level, "LEVEL", 84, ox + 309, 200);
            power(o == 0 ? ID::o1On : ID::o2On, o == 0 ? "Oscillator 1 on" : "Oscillator 2 on", { ox, 44, 380, 276 });
        }
        addAndMakeVisible(vu);
        vu.setBounds(800, 94, 250, 150);
        knob(ID::master, "MASTER", 100, 1118, 112);

        // row 2: amp envelope, filter, distortion
        place(ampD, { 24, 362, 362, 138 });
        knob(ID::attack, "ATTACK", 70, 69, 508);
        knob(ID::decay, "DECAY", 70, 159, 508);
        knob(ID::sustain, "SUSTAIN", 70, 249, 508);
        knob(ID::release, "RELEASE", 70, 339, 508);
        power(ID::eOn, "Amp envelope on", { 12, 328, 386, 290 });

        choice(ID::fType, filterTypeNames, "Filter type", 418, 362, 362, 28);
        place(filtD, { 418, 396, 362, 104 });
        knob(ID::fCutoff, "CUTOFF", 70, 463, 508);
        knob(ID::fRes, "RESONANCE", 70, 553, 508);
        knob(ID::fVel, "VELOCITY", 70, 643, 508);
        knob(ID::fMix, "MIX", 70, 733, 508);
        power(ID::fOn, "Filter on", { 406, 328, 386, 290 });

        choice(ID::dType, distTypeNames, "Distortion type", 812, 362, 364, 28);
        place(distD, { 812, 396, 364, 104 });
        knob(ID::dDrive, "DRIVE", 70, 873, 508);
        knob(ID::dGain, "GAIN", 70, 994, 508);
        knob(ID::dRelease, "RELEASE", 70, 1115, 508);
        power(ID::dOn, "Distortion on", { 800, 328, 388, 290 });

        // row 3: compressor and LFO
        place(compD, { 24, 660, 556, 128 });
        const char* cid[7] { ID::cThresh, ID::cRatio, ID::cAttack, ID::cRelease, ID::cKnee, ID::cMakeup, ID::cMix };
        const char* cnm[7] { "THRESH", "RATIO", "ATTACK", "RELEASE", "KNEE", "MAKEUP", "MIX" };
        for (int i = 0; i < 7; ++i)
            knob(cid[i], cnm[i], 64, 64 + (int) (79.4f * (float) i), 796);
        power(ID::cOn, "Compressor on", { 12, 626, 580, 270 });

        place(lfoD, { 612, 660, 330, 226 });
        choice(ID::lShape, lfoShapeNames, "LFO shape", 954, 660, 222, 28);
        choice(ID::lRate, syncNames, "LFO rate", 954, 694, 222, 28);
        choice(ID::lTarget, lfoTargetNames, "LFO target", 954, 728, 222, 28);
        knob(ID::lDepth, "DEPTH", 64, 1012, 764);
        knob(ID::lCurve, "CURVE", 64, 1118, 764);
        power(ID::lOn, "LFO on", { 600, 626, 588, 270 });

        // sequencer
        for (auto* b : { &patPrev, &patNext }) addAndMakeVisible(b);
        patPrev.setBounds(30, 946, 28, 28);
        patWin.setBounds(64, 944, 210, 32);
        patNext.setBounds(280, 946, 28, 28);
        patWin.seqStyle = true;
        patWin.getItems = [] { juce::StringArray a; for (auto& q : pb::seqPatterns) a.add(q.name); return a; };
        patWin.getCurrent = [this] { return proc.currentPattern.load(); };
        patWin.onPickItem = [this](int i) { proc.loadSeqPattern(i); patWin.repaint(); };
        patWin.setTitle("Sequencer pattern");
        addAndMakeVisible(patWin);
        patPrev.onClick = [this] { proc.loadSeqPattern((proc.currentPattern.load() + pb::numSeqPatterns - 1) % pb::numSeqPatterns); patWin.repaint(); };
        patNext.onClick = [this] { proc.loadSeqPattern((proc.currentPattern.load() + 1) % pb::numSeqPatterns); patWin.repaint(); };
        choice(ID::sRate, seqRateNames, "Sequencer rate", 340, 944, 110, 32, true);
        choice(ID::sDir, seqDirNames, "Sequencer direction", 462, 944, 130, 32, true);
        place(seqDisplay, { 24, 1000, 1000, 96 });
        knob(ID::sGate, "GATE", 64, 1100, 960, true);
        power(ID::sOn, "Sequencer on", { 12, 904, 1176, 200 }, true);

        // preset strip
        const int sy = 1116;
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
        patWin.repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(col::page);
        g.setGradientFill(juce::ColourGradient(col::amber.withAlpha(0.07f), 600, 0, col::amber.withAlpha(0.0f), 600, 520, true));
        g.fillRect(0, 0, 1200, 1100);
        g.setGradientFill(juce::ColourGradient(col::amber.withAlpha(0.05f), 600, 1260, col::amber.withAlpha(0.0f), 600, 780, true));
        g.fillRect(0, 780, 1200, 480);

        // faint hairline frame and tick marks along the header rule
        g.setColour(col::line.withAlpha(0.10f));
        g.drawRoundedRectangle(3.5f, 3.5f, 1193.f, 1253.f, 12.f, 1.f);
        g.setColour(col::line.withAlpha(0.22f));
        for (int x = 16; x <= 1184; x += 16)
            g.fillRect((float) x, (x % 64 == 0) ? 33.f : 36.f, 1.f, (x % 64 == 0) ? 5.f : 2.f);

        struct T { juce::Rectangle<float> r; const char* name; };
        const T tiles[] {
            { { 12, 44, 380, 276 }, "OSCILLATOR 1" }, { { 400, 44, 380, 276 }, "OSCILLATOR 2" }, { { 788, 44, 400, 276 }, "OUTPUT" },
            { { 12, 328, 386, 290 }, "AMP ENVELOPE" }, { { 406, 328, 386, 290 }, "FILTER" }, { { 800, 328, 388, 290 }, "DISTORTION" },
            { { 12, 626, 580, 270 }, "COMPRESSOR" }, { { 600, 626, 588, 270 }, "LFO / DUCK" } };
        for (auto& t : tiles)
        {
            theme::drawTile(g, t.r);
            theme::section(g, t.name, t.r.getX() + 18.f, t.r.getY() + 6.f, t.r.getWidth() - 140.f);
        }

        {
            const int old = rampMode();
            rampMode() = 1;
            theme::drawTile(g, { 12, 904, 1176, 200 });
            theme::section(g, "SEQUENCER", 30, 910, 1040);
            g.setFont(label(15.0f, 0.14f));
            auto cap = [&](const juce::String& s, float cx, float y) { glowText(g, s, juce::Rectangle<float>(cx - 80, y, 160, 17), juce::Justification::centred, juce::Colour((juce::uint32) 0xffc2b4ff), 0.22f); };
            cap("PATTERN", 169, 978);
            cap("RATE", 395, 978);
            cap("DIRECTION", 527, 978);
            rampMode() = old;
        }
        theme::drawTile(g, { 12, 1112, 1176, 140 });

        for (auto& r : bezels)
            theme::paintBezel(g, r.toFloat().expanded(3.f));
        theme::paintBezel(g, juce::Rectangle<float>(797.f, 91.f, 256.f, 156.f));

        g.setFont(label(29.0f, 0.38f, true));
        glowTextRamp(g, "POTATO BASS", juce::Rectangle<float>(380, 4, 440, 32), juce::Justification::centred, 0.35f);
        g.setFont(mono(12.0f)); g.setColour(juce::Colour((juce::uint32) 0xffb98c62));
        g.drawText(juce::CharPointer_UTF8("EDM BASS SYNTH  \xc2\xb7  100 PRESETS"), juce::Rectangle<float>(16, 12, 300, 16), juce::Justification::left, false);
        g.drawText(juce::CharPointer_UTF8("VST3  \xc2\xb7  WIN"), juce::Rectangle<float>(884, 12, 300, 16), juce::Justification::right, false);

        g.setFont(label(15.0f, 0.14f));
        auto cap = [&](const juce::String& s, float cx, float y) { glowText(g, s, juce::Rectangle<float>(cx - 120, y, 240, 17), juce::Justification::centred, col::ink, 0.22f); };
        const float sy = 1116.f;
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
    struct Dim { juce::Rectangle<int> area; PowerToggle* led; };

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
    WaveDisplay wave1, wave2;
    ADSRDisplay ampD;
    FilterDisplay filtD;
    DistortionDisplay distD;
    CompressorDisplay compD;
    LfoDisplay lfoD;
    SeqDisplay seqDisplay;
    juce::OwnedArray<KKnob> knobs;
    juce::OwnedArray<ChoiceWindow> choices;
    juce::OwnedArray<PowerToggle> leds;
    std::vector<Dim> dims;
    std::vector<juce::Rectangle<int>> bezels;
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
    setSize(panelW * 68 / 100, panelH * 68 / 100);
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
