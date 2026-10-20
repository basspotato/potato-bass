#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include "Params.h"
#include "Wavetables.h"

namespace theme
{
    inline const juce::Colour bg      { (juce::uint32) 0xff070d14 };
    inline const juce::Colour panel   { (juce::uint32) 0xff0c1822 };
    inline const juce::Colour display { (juce::uint32) 0xff050a10 };
    inline const juce::Colour grid    { (juce::uint32) 0xff12303a };
    inline const juce::Colour border  { (juce::uint32) 0xff1a4a5a };
    inline const juce::Colour accent  { (juce::uint32) 0xffffb020 };
    inline const juce::Colour hot     { (juce::uint32) 0xffff5a2a };
    inline const juce::Colour cyan    { (juce::uint32) 0xff2fd6e0 };
    inline const juce::Colour text    { (juce::uint32) 0xffbfe9ee };

    inline void paintDisplayBg(juce::Graphics& g, juce::Rectangle<float> b)
    {
        g.setColour(display);
        g.fillRoundedRectangle(b, 6.f);
        g.setColour(border);
        g.drawRoundedRectangle(b.reduced(0.5f), 6.f, 1.f);
    }
}

// Dark teal HUD style: segmented amber-to-orange rings around a cyan-outlined hub.
class PotatoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PotatoLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, theme::accent);
        setColour(juce::Slider::trackColourId, theme::accent);
        setColour(juce::Slider::backgroundColourId, theme::grid);
        setColour(juce::Slider::textBoxTextColourId, theme::text);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, theme::text);
        setColour(juce::ComboBox::backgroundColourId, theme::display);
        setColour(juce::ComboBox::outlineColourId, theme::border);
        setColour(juce::ComboBox::textColourId, theme::cyan);
        setColour(juce::ComboBox::arrowColourId, theme::cyan);
        setColour(juce::PopupMenu::backgroundColourId, theme::panel);
        setColour(juce::PopupMenu::textColourId, theme::text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, theme::accent);
        setColour(juce::PopupMenu::highlightedTextColourId, theme::bg);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos,
                          float startA, float endA, juce::Slider&) override
    {
        const auto b = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(3.f);
        const float rOut = juce::jmin(b.getWidth(), b.getHeight()) * 0.5f;
        const float rIn = rOut * 0.72f;
        const auto c = b.getCentre();
        constexpr int segs = 24;

        for (int i = 0; i < segs; ++i)
        {
            const float t = (float) i / (float) (segs - 1);
            const float a = startA + t * (endA - startA);
            g.setColour(t <= pos + 0.001f ? theme::accent.interpolatedWith(theme::hot, t) : theme::grid);
            g.drawLine(c.x + rIn * std::sin(a), c.y - rIn * std::cos(a),
                       c.x + rOut * std::sin(a), c.y - rOut * std::cos(a), 2.4f);
        }

        const float rC = rIn * 0.8f;
        g.setColour(theme::display);
        g.fillEllipse(c.x - rC, c.y - rC, 2.f * rC, 2.f * rC);
        g.setColour(theme::cyan.withAlpha(0.55f));
        g.drawEllipse(c.x - rC, c.y - rC, 2.f * rC, 2.f * rC, 1.2f);

        const float ang = startA + pos * (endA - startA);
        g.setColour(theme::cyan);
        g.drawLine(c.x + rC * 0.35f * std::sin(ang), c.y - rC * 0.35f * std::cos(ang),
                   c.x + rC * 0.9f * std::sin(ang), c.y - rC * 0.9f * std::cos(ang), 2.f);
    }
};

class Knob : public juce::Component
{
public:
    Knob(juce::AudioProcessorValueTreeState& s, const char* id, const juce::String& name)
        : attachment(s, id, slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 16);
        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(juce::FontOptions(13.f)));
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds(b.removeFromTop(16));
        slider.setBounds(b);
    }

private:
    juce::Slider slider;
    juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

class WaveDisplay : public juce::Component
{
public:
    WaveDisplay(const Wavetables& w, juce::AudioProcessorValueTreeState& s, const char* id)
        : wt(w), param(*s.getParameter(id)),
          att(param, [this](float v) { table = juce::jlimit(0, Wavetables::numTables - 1, juce::roundToInt(v)); repaint(); })
    {
        att.sendInitialUpdate();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        theme::paintDisplayBg(g, b);
        auto r = b.reduced(6.f);
        g.setColour(theme::grid);
        g.drawHorizontalLine((int) r.getCentreY(), r.getX(), r.getRight());

        juce::Path p;
        const int w = juce::jmax(2, (int) r.getWidth());
        for (int x = 0; x <= w; ++x)
        {
            const float phase = juce::jmin(0.9999f, (float) x / (float) w);
            const float y = r.getCentreY() - wt.readDisplay(table, phase) * r.getHeight() * 0.5f;
            if (x == 0) p.startNewSubPath(r.getX(), y); else p.lineTo(r.getX() + (float) x * r.getWidth() / (float) w, y);
        }
        g.setColour(theme::accent);
        g.strokePath(p, juce::PathStrokeType(1.8f));
    }

private:
    const Wavetables& wt;
    juce::RangedAudioParameter& param;
    juce::ParameterAttachment att;
    int table = 0;
};

// Editable ADSR: drag the attack peak (time), decay/sustain point (time + level) and release end (time).
class ADSRDisplay : public juce::Component
{
public:
    explicit ADSRDisplay(juce::AudioProcessorValueTreeState& s)
        : aP(*s.getParameter(ID::attack)), dP(*s.getParameter(ID::decay)),
          sP(*s.getParameter(ID::sustain)), rP(*s.getParameter(ID::release)),
          attA(aP, [this](float v) { na = aP.convertTo0to1(v); repaint(); }),
          attD(dP, [this](float v) { nd = dP.convertTo0to1(v); repaint(); }),
          attS(sP, [this](float v) { ns = sP.convertTo0to1(v); repaint(); }),
          attR(rP, [this](float v) { nr = rP.convertTo0to1(v); repaint(); })
    {
        attA.sendInitialUpdate(); attD.sendInitialUpdate();
        attS.sendInitialUpdate(); attR.sendInitialUpdate();
    }

    void paint(juce::Graphics& g) override
    {
        theme::paintDisplayBg(g, getLocalBounds().toFloat());

        const auto r = area();
        g.setColour(theme::grid);
        for (int i = 0; i <= 4; ++i)
        {
            const float y = r.getY() + r.getHeight() * (float) i / 4.f;
            g.drawLine(r.getX(), y, r.getRight(), y, 1.f);
        }

        const auto p = points();
        juce::Path path;
        path.startNewSubPath(p.p0);
        path.lineTo(p.p1); path.lineTo(p.p2); path.lineTo(p.p3); path.lineTo(p.p4);

        g.setColour(theme::accent.withAlpha(0.18f));
        g.fillPath(path);
        g.setColour(theme::accent);
        g.strokePath(path, juce::PathStrokeType(2.f));

        for (auto pt : { p.p1, p.p2, p.p4 })
        {
            g.setColour(theme::bg);
            g.fillEllipse(pt.x - 6.f, pt.y - 6.f, 12.f, 12.f);
            g.setColour(theme::accent);
            g.drawEllipse(pt.x - 6.f, pt.y - 6.f, 12.f, 12.f, 2.f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const auto p = points();
        float best = 18.f;
        drag = -1;
        auto test = [&](int idx, juce::Point<float> pt)
        {
            const float d = e.position.getDistanceFrom(pt);
            if (d < best) { best = d; drag = idx; }
        };
        test(0, p.p1); test(1, p.p2); test(2, p.p4);

        if (drag == 0) attA.beginGesture();
        else if (drag == 1) { attD.beginGesture(); attS.beginGesture(); }
        else if (drag == 2) attR.beginGesture();
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (drag < 0) return;
        const auto r = area();
        const auto p = points();
        auto n01 = [](float v) { return juce::jlimit(0.f, 1.f, v); };

        if (drag == 0)
            attA.setValueAsPartOfGesture(aP.convertFrom0to1(n01((e.position.x - r.getX()) / aMax(r))));
        else if (drag == 1)
        {
            attD.setValueAsPartOfGesture(dP.convertFrom0to1(n01((e.position.x - p.p1.x) / dMax(r))));
            attS.setValueAsPartOfGesture(sP.convertFrom0to1(n01((r.getBottom() - e.position.y) / r.getHeight())));
        }
        else
            attR.setValueAsPartOfGesture(rP.convertFrom0to1(n01((e.position.x - p.p3.x) / rMax(r))));
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        if (drag == 0) attA.endGesture();
        else if (drag == 1) { attD.endGesture(); attS.endGesture(); }
        else if (drag == 2) attR.endGesture();
        drag = -1;
    }

private:
    struct Pts { juce::Point<float> p0, p1, p2, p3, p4; };

    juce::Rectangle<float> area() const { return getLocalBounds().toFloat().reduced(14.f); }
    static float aMax(juce::Rectangle<float> r) { return r.getWidth() * 0.28f; }
    static float dMax(juce::Rectangle<float> r) { return r.getWidth() * 0.28f; }
    static float rMax(juce::Rectangle<float> r) { return r.getWidth() * 0.28f; }

    Pts points() const
    {
        const auto r = area();
        Pts p;
        p.p0 = { r.getX(), r.getBottom() };
        p.p1 = { r.getX() + na * aMax(r), r.getY() };
        p.p2 = { p.p1.x + nd * dMax(r), r.getBottom() - ns * r.getHeight() };
        p.p3 = { r.getX() + aMax(r) + dMax(r) + r.getWidth() * 0.16f, p.p2.y };
        p.p4 = { p.p3.x + nr * rMax(r), r.getBottom() };
        return p;
    }

    float na = 0.f, nd = 0.f, ns = 0.f, nr = 0.f;
    int drag = -1;
    juce::RangedAudioParameter &aP, &dP, &sP, &rP;
    juce::ParameterAttachment attA, attD, attS, attR;
};

// Filter response display: drag horizontally for cutoff, vertically for resonance.
class FilterDisplay : public juce::Component
{
public:
    explicit FilterDisplay(juce::AudioProcessorValueTreeState& s)
        : cP(*s.getParameter(ID::fCutoff)), rP(*s.getParameter(ID::fRes)), tP(*s.getParameter(ID::fType)),
          attC(cP, [this](float v) { cutoff = v; repaint(); }),
          attR(rP, [this](float v) { res = v; repaint(); }),
          attT(tP, [this](float v) { type = juce::roundToInt(v); repaint(); })
    {
        attC.sendInitialUpdate(); attR.sendInitialUpdate(); attT.sendInitialUpdate();
    }

    void paint(juce::Graphics& g) override
    {
        theme::paintDisplayBg(g, getLocalBounds().toFloat());
        const auto r = area();

        g.setColour(theme::grid);
        for (float f : { 100.f, 1000.f, 10000.f })
        {
            const float x = r.getX() + r.getWidth() * std::log10(f / 20.f) / 3.f;
            g.drawLine(x, r.getY(), x, r.getBottom(), 1.f);
        }
        g.drawLine(r.getX(), dbToY(0.f, r), r.getRight(), dbToY(0.f, r), 1.f);

        juce::Path path;
        const int w = juce::jmax(2, (int) r.getWidth());
        for (int x = 0; x <= w; ++x)
        {
            const float f = 20.f * std::pow(1000.f, (float) x / (float) w);
            const float db = 20.f * std::log10(juce::jmax(1.0e-5f, magnitude(f)));
            const float px = r.getX() + (float) x;
            const float py = dbToY(db, r);
            if (x == 0) path.startNewSubPath(px, py); else path.lineTo(px, py);
        }

        juce::Path fill(path);
        fill.lineTo(r.getRight(), r.getBottom());
        fill.lineTo(r.getX(), r.getBottom());
        fill.closeSubPath();
        g.setColour(theme::accent.withAlpha(0.18f));
        g.fillPath(fill);
        g.setColour(theme::accent);
        g.strokePath(path, juce::PathStrokeType(2.f));

        const float cx = r.getX() + r.getWidth() * std::log10(cutoff / 20.f) / 3.f;
        const float cy = dbToY(20.f * std::log10(juce::jmax(1.0e-5f, magnitude(cutoff))), r);
        g.setColour(theme::bg);
        g.fillEllipse(cx - 6.f, cy - 6.f, 12.f, 12.f);
        g.setColour(theme::accent);
        g.drawEllipse(cx - 6.f, cy - 6.f, 12.f, 12.f, 2.f);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        attC.beginGesture();
        attR.beginGesture();
        startRes = res;
        startY = e.position.y;
        mouseDrag(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        const auto r = area();
        const float xn = juce::jlimit(0.f, 1.f, (e.position.x - r.getX()) / r.getWidth());
        attC.setValueAsPartOfGesture(juce::jlimit(20.f, 20000.f, 20.f * std::pow(1000.f, xn)));
        attR.setValueAsPartOfGesture(juce::jlimit(0.f, 1.f, startRes + (startY - e.position.y) / (0.6f * r.getHeight())));
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        attC.endGesture();
        attR.endGesture();
    }

private:
    juce::Rectangle<float> area() const { return getLocalBounds().toFloat().reduced(12.f); }

    static float dbToY(float db, juce::Rectangle<float> r)
    {
        const float n = juce::jlimit(0.f, 1.f, (db + 36.f) / 60.f);
        return r.getBottom() - n * r.getHeight();
    }

    float magnitude(float f) const
    {
        const float w = f / cutoff;
        const float k = resToK(res);
        const float a = 1.f - w * w, b = k * w;
        const float den = std::sqrt(a * a + b * b);
        float m = 0.f;
        switch (filterMode(type))
        {
            case 0:  m = 1.f / den; break;
            case 1:  m = (w * w) / den; break;
            case 2:  m = b / den; break;
            default: m = std::abs(a) / den; break;
        }
        return filterTwoStage(type) ? m * m : m;
    }

    float cutoff = 1200.f, res = 0.2f, startRes = 0.f, startY = 0.f;
    int type = 1;
    juce::RangedAudioParameter &cP, &rP, &tP;
    juce::ParameterAttachment attC, attR, attT;
};

class HSlider : public juce::Component
{
public:
    HSlider(juce::AudioProcessorValueTreeState& s, const char* id, const juce::String& name)
        : attachment(s, id, slider)
    {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 18);
        label.setText(name, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions(13.f)));
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds(b.removeFromLeft(54));
        slider.setBounds(b);
    }

private:
    juce::Slider slider;
    juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

// Compressor transfer curve (input dB -> output dB) with live gain-reduction meter.
// Drag the threshold point left/right; drag the point at 0 dB input up/down for ratio.
class CompressorDisplay : public juce::Component, private juce::Timer
{
public:
    CompressorDisplay(juce::AudioProcessorValueTreeState& s, const std::atomic<float>& m)
        : meter(m), tP(*s.getParameter(ID::cThresh)), rP(*s.getParameter(ID::cRatio)), kP(*s.getParameter(ID::cKnee)),
          attT(tP, [this](float v) { thr = v; repaint(); }),
          attR(rP, [this](float v) { ratio = v; repaint(); }),
          attK(kP, [this](float v) { knee = v; repaint(); })
    {
        attT.sendInitialUpdate(); attR.sendInitialUpdate(); attK.sendInitialUpdate();
        startTimerHz(30);
    }

    ~CompressorDisplay() override { stopTimer(); }

    void paint(juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        theme::paintDisplayBg(g, b);
        const auto r = area();

        g.setColour(theme::grid);
        for (float db : { -40.f, -20.f, 0.f })
        {
            g.drawLine(xOf(db, r), r.getY(), xOf(db, r), r.getBottom(), 1.f);
            g.drawLine(r.getX(), yOf(db, r), r.getRight(), yOf(db, r), 1.f);
        }
        g.drawLine(r.getX(), r.getBottom(), r.getRight(), r.getY(), 1.f);

        juce::Path path;
        const int w = juce::jmax(2, (int) r.getWidth());
        for (int x = 0; x <= w; ++x)
        {
            const float inDb = -60.f + 60.f * (float) x / (float) w;
            const float px = r.getX() + (float) x;
            const float py = yOf(compCurve(inDb, thr, ratio, knee), r);
            if (x == 0) path.startNewSubPath(px, py); else path.lineTo(px, py);
        }
        juce::Path fill(path);
        fill.lineTo(r.getRight(), r.getBottom());
        fill.lineTo(r.getX(), r.getBottom());
        fill.closeSubPath();
        g.setColour(theme::accent.withAlpha(0.18f));
        g.fillPath(fill);
        g.setColour(theme::accent);
        g.strokePath(path, juce::PathStrokeType(2.f));

        for (auto pt : { handleThr(r), handleRatio(r) })
        {
            g.setColour(theme::bg);
            g.fillEllipse(pt.x - 6.f, pt.y - 6.f, 12.f, 12.f);
            g.setColour(theme::accent);
            g.drawEllipse(pt.x - 6.f, pt.y - 6.f, 12.f, 12.f, 2.f);
        }

        const float mx = b.getRight() - 14.f;
        g.setColour(theme::grid);
        g.drawRect(mx, r.getY(), 6.f, r.getHeight(), 1.f);
        g.setColour(theme::accent);
        g.fillRect(mx, r.getY(), 6.f, r.getHeight() * juce::jlimit(0.f, 1.f, shownGr / 24.f));
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const auto r = area();
        float best = 18.f;
        drag = -1;
        auto test = [&](int idx, juce::Point<float> pt)
        {
            const float d = e.position.getDistanceFrom(pt);
            if (d < best) { best = d; drag = idx; }
        };
        test(0, handleThr(r));
        test(1, handleRatio(r));
        if (drag == 0) attT.beginGesture();
        else if (drag == 1) attR.beginGesture();
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (drag < 0) return;
        const auto r = area();
        if (drag == 0)
            attT.setValueAsPartOfGesture(juce::jlimit(-60.f, 0.f, -60.f + 60.f * (e.position.x - r.getX()) / r.getWidth()));
        else if (thr < -0.5f)
        {
            const float outDb = -60.f + 60.f * (r.getBottom() - e.position.y) / r.getHeight();
            const float d = juce::jlimit(thr, 0.f, outDb) - thr;
            attR.setValueAsPartOfGesture(d < 0.05f ? 20.f : juce::jlimit(1.f, 20.f, -thr / d));
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        if (drag == 0) attT.endGesture();
        else if (drag == 1) attR.endGesture();
        drag = -1;
    }

private:
    void timerCallback() override
    {
        const float gr = meter.load();
        if (std::abs(gr - shownGr) > 0.05f) { shownGr = gr; repaint(); }
    }

    juce::Rectangle<float> area() const { return getLocalBounds().toFloat().reduced(12.f).withTrimmedRight(12.f); }
    static float xOf(float db, juce::Rectangle<float> r) { return r.getX() + (juce::jlimit(-60.f, 0.f, db) + 60.f) / 60.f * r.getWidth(); }
    static float yOf(float db, juce::Rectangle<float> r) { return r.getBottom() - (juce::jlimit(-60.f, 0.f, db) + 60.f) / 60.f * r.getHeight(); }
    juce::Point<float> handleThr(juce::Rectangle<float> r) const { return { xOf(thr, r), yOf(thr, r) }; }
    juce::Point<float> handleRatio(juce::Rectangle<float> r) const { return { xOf(0.f, r), yOf(compCurve(0.f, thr, ratio, knee), r) }; }

    const std::atomic<float>& meter;
    float thr = -12.f, ratio = 4.f, knee = 6.f, shownGr = 0.f;
    int drag = -1;
    juce::RangedAudioParameter &tP, &rP, &kP;
    juce::ParameterAttachment attT, attR, attK;
};

// Distortion transfer curve. Drag left/right for drive, up/down for output gain.
class DistortionDisplay : public juce::Component
{
public:
    explicit DistortionDisplay(juce::AudioProcessorValueTreeState& s)
        : tP(*s.getParameter(ID::dType)), dP(*s.getParameter(ID::dDrive)), gP(*s.getParameter(ID::dGain)),
          attT(tP, [this](float v) { type = juce::roundToInt(v); repaint(); }),
          attD(dP, [this](float v) { drive = v; repaint(); }),
          attG(gP, [this](float v) { gain = v; repaint(); })
    {
        attT.sendInitialUpdate(); attD.sendInitialUpdate(); attG.sendInitialUpdate();
    }

    void paint(juce::Graphics& g) override
    {
        theme::paintDisplayBg(g, getLocalBounds().toFloat());
        const auto r = area();

        g.setColour(theme::grid);
        g.drawLine(r.getX(), r.getCentreY(), r.getRight(), r.getCentreY(), 1.f);
        g.drawLine(r.getCentreX(), r.getY(), r.getCentreX(), r.getBottom(), 1.f);
        g.drawLine(r.getX(), r.getBottom(), r.getRight(), r.getY(), 1.f);

        const float dg = driveGain(drive);
        const float outLin = juce::Decibels::decibelsToGain(gain);
        juce::Path path;
        const int w = juce::jmax(2, (int) r.getWidth());
        for (int px = 0; px <= w; ++px)
        {
            const float x = -1.f + 2.f * (float) px / (float) w;
            const float y = type == 0 ? x : juce::jlimit(-1.f, 1.f, shape(type, x * dg) * outLin);
            const float sx = r.getX() + (float) px;
            const float sy = r.getCentreY() - y * r.getHeight() * 0.5f;
            if (px == 0) path.startNewSubPath(sx, sy); else path.lineTo(sx, sy);
        }
        g.setColour(theme::accent);
        g.strokePath(path, juce::PathStrokeType(2.f));

        const auto h = handle(r);
        g.setColour(theme::bg);
        g.fillEllipse(h.x - 6.f, h.y - 6.f, 12.f, 12.f);
        g.setColour(theme::accent);
        g.drawEllipse(h.x - 6.f, h.y - 6.f, 12.f, 12.f, 2.f);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        attD.beginGesture();
        attG.beginGesture();
        mouseDrag(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        const auto r = area();
        attD.setValueAsPartOfGesture(juce::jlimit(0.f, 1.f, (e.position.x - r.getX()) / r.getWidth()));
        attG.setValueAsPartOfGesture(juce::jlimit(-24.f, 12.f, -24.f + 36.f * (r.getBottom() - e.position.y) / r.getHeight()));
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        attD.endGesture();
        attG.endGesture();
    }

private:
    juce::Rectangle<float> area() const { return getLocalBounds().toFloat().reduced(12.f); }
    juce::Point<float> handle(juce::Rectangle<float> r) const
    {
        return { r.getX() + drive * r.getWidth(), r.getBottom() - (gain + 24.f) / 36.f * r.getHeight() };
    }

    float drive = 0.3f, gain = 0.f;
    int type = 0;
    juce::RangedAudioParameter &tP, &dP, &gP;
    juce::ParameterAttachment attT, attD, attG;
};
