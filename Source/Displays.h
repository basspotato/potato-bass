#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include <tuple>
#include "Params.h"
#include "Wavetables.h"
#include "Look.h"

namespace theme
{
    // Potato-Kicker palette: near-black panels, glowing orange accents, amber text.
    inline const juce::Colour bg      { (juce::uint32) 0xff070707 };
    inline const juce::Colour panel   { (juce::uint32) 0xff141210 };
    inline const juce::Colour display { (juce::uint32) 0xff080504 };
    inline const juce::Colour grid    { (juce::uint32) 0x1aff8a1e };
    inline const juce::Colour border  { (juce::uint32) 0xff5a3410 };
    inline const juce::Colour accent  { (juce::uint32) 0xffff8a1e };
    inline const juce::Colour yellow  { (juce::uint32) 0xffffd23f };
    inline const juce::Colour hot     { (juce::uint32) 0xffff7212 };
    inline const juce::Colour cyan    { (juce::uint32) 0xffffc27a };
    inline const juce::Colour text    { (juce::uint32) 0xffffb35e };

    using Rect = juce::Rectangle<float>;

    // Kicker-style glow: a soft yellow halo under a pale hot core.
    inline void glow(juce::Graphics& g, const juce::Path& p, float w, juce::Colour c)
    {
        g.setColour(c.withAlpha(0.14f));
        g.strokePath(p, juce::PathStrokeType(w + 7.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(c.withAlpha(0.28f));
        g.strokePath(p, juce::PathStrokeType(w + 3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(juce::Colour((juce::uint32) 0xfffff3d0));
        g.strokePath(p, juce::PathStrokeType(w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Draggable node handle, as on the Kicker screen: dark disc with amber ring, red when grabbed.
    inline void node(juce::Graphics& g, float x, float y, bool active = false)
    {
        const auto r = juce::Rectangle<float>(active ? 14.f : 11.f, active ? 14.f : 11.f).withCentre({ x, y });
        g.setColour(active ? juce::Colour((juce::uint32) 0xffff3b2c) : juce::Colour((juce::uint32) 0xff140d05)); g.fillEllipse(r);
        g.setColour(active ? juce::Colour((juce::uint32) 0xffffd2c8) : look::col::amberHi); g.drawEllipse(r, 2.f);
    }

    // Dark HUD tile with a thin orange border, a light strip on the top edge and bright corner brackets.
    inline void drawTile(juce::Graphics& g, Rect r)
    {
        using namespace look;
        const juce::Colour lineC = rampMode() == 1 ? ramp(0.5f) : col::line;
        g.setColour(juce::Colours::black.withAlpha(0.55f)); g.fillRoundedRectangle(r.translated(0, 4), 9);
        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff141210), 0, r.getY(), juce::Colour((juce::uint32) 0xff0b0a09), 0, r.getBottom(), false));
        g.fillRoundedRectangle(r, 9);
        g.setGradientFill(juce::ColourGradient(lineC.withAlpha(0.06f), 0, r.getY(), lineC.withAlpha(0.0f), 0, r.getY() + 46, false));
        g.fillRoundedRectangle(r, 9);
        g.setGradientFill(juce::ColourGradient(ramp(0.f).withAlpha(0.3f), r.getX(), 0, ramp(1.f).withAlpha(0.3f), r.getRight(), 0, false));
        g.drawRoundedRectangle(r.reduced(0.5f), 9, 1.0f);

        const float lx0 = r.getX() + 26, lx1 = r.getRight() - 26;
        auto strip = juce::ColourGradient(ramp(0.f).withAlpha(0.0f), lx0, 0, ramp(1.f).withAlpha(0.0f), lx1, 0, false);
        strip.addColour(0.15, ramp(0.f).withAlpha(0.9f));
        strip.addColour(0.85, ramp(1.f).withAlpha(0.9f));
        auto gl = strip; gl.multiplyOpacity(0.22f);
        g.setGradientFill(gl);    g.fillRoundedRectangle(lx0, r.getY() - 1.5f, lx1 - lx0, 6.0f, 3.0f);
        g.setGradientFill(strip); g.fillRect(lx0, r.getY() + 0.5f, lx1 - lx0, 1.6f);

        g.setGradientFill(juce::ColourGradient(ramp(0.f).withAlpha(0.95f), r.getX(), 0, ramp(1.f).withAlpha(0.95f), r.getRight(), 0, false));
        const float L = 16.0f, t = 2.0f, x0 = r.getX() + 3, y0 = r.getY() + 3, x1 = r.getRight() - 3, y1 = r.getBottom() - 3;
        g.fillRect(x0, y0, L, t);         g.fillRect(x0, y0, t, L);
        g.fillRect(x1 - L, y0, L, t);     g.fillRect(x1 - t, y0, t, L);
        g.fillRect(x0, y1 - t, L, t);     g.fillRect(x0, y1 - L, t, L);
        g.fillRect(x1 - L, y1 - t, L, t); g.fillRect(x1 - t, y1 - L, t, L);
    }

    // Section caption: glowing orange-to-yellow text between two fading rules with LED end caps.
    inline void section(juce::Graphics& g, const juce::String& text, float x, float y, float w)
    {
        using namespace look;
        const juce::Colour lineC = rampMode() == 1 ? ramp(0.5f) : col::line;
        const auto f = label(15.0f, 0.22f);
        const float tw = juce::GlyphArrangement::getStringWidth(f, text) + 32.0f;
        const float cy = y + 7.0f, mid = x + w * 0.5f, l = mid - tw * 0.5f, r = mid + tw * 0.5f;
        g.setGradientFill(juce::ColourGradient(lineC.withAlpha(0.0f), x, cy, lineC.withAlpha(0.45f), l, cy, false));
        g.fillRect(x, cy, l - x, 1.0f);
        g.setGradientFill(juce::ColourGradient(lineC.withAlpha(0.45f), r, cy, lineC.withAlpha(0.0f), x + w, cy, false));
        g.fillRect(r, cy, x + w - r, 1.0f);
        drawLed(g, juce::Rectangle<float>(5, 5).withCentre({ l + 5, cy + 0.5f }), true);
        drawLed(g, juce::Rectangle<float>(5, 5).withCentre({ r - 5, cy + 0.5f }), true);
        g.setFont(f);
        glowTextRamp(g, text, juce::Rectangle<float>(l + 10, y - 1, tw - 20, 17), juce::Justification::centred, 0.25f);
    }

    inline void paintPanel(juce::Graphics& g, Rect b, const juce::String& title)
    {
        drawTile(g, b);
        section(g, title, 30.f, 6.f, b.getWidth() - 60.f);
    }

    // Kicker screen: warm radial glow rising from the bottom edge, thin amber outline.
    inline void paintDisplayBg(juce::Graphics& g, Rect b, bool seqTint = false)
    {
        using namespace look;
        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) (seqTint ? 0xff141640 : 0xff1e1408)), b.getCentreX(), b.getY() + b.getHeight() * 0.9f,
                                               juce::Colour((juce::uint32) (seqTint ? 0xff05051a : 0xff0a0703)), b.getCentreX() + b.getWidth() * 0.7f, b.getY() + b.getHeight() * 0.9f, true));
        g.fillRoundedRectangle(b, 7.f);
        g.setColour((seqTint ? juce::Colour((juce::uint32) 0xff6a4cff) : col::line).withAlpha(0.25f));
        g.drawRoundedRectangle(b.reduced(0.5f), 7.f, 1.f);
    }

    // Outer bezel with corner brackets (the frame around the tabbed centre screen).
    inline void paintBezel(juce::Graphics& g, Rect bb)
    {
        using namespace look;
        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff100d0a), 0, 0, juce::Colour((juce::uint32) 0xff060504), 0, bb.getHeight(), false));
        g.fillRoundedRectangle(bb, 10.f);
        g.setColour(col::line.withAlpha(0.30f)); g.drawRoundedRectangle(bb.reduced(0.5f), 10.f, 1.f);
        g.setColour(col::line.withAlpha(0.85f));
        for (auto [x, y, dx, dy] : { std::tuple<float, float, float, float> { 4, 4, 1, 1 }, { bb.getRight() - 4, 4, -1, 1 },
                                     { 4, bb.getBottom() - 4, 1, -1 }, { bb.getRight() - 4, bb.getBottom() - 4, -1, -1 } })
        {
            g.fillRect(dx > 0 ? x : x - 16, dy > 0 ? y : y - 2, 16.f, 2.f);
            g.fillRect(dx > 0 ? x : x - 2, dy > 0 ? y : y - 16, 2.f, 16.f);
        }
    }
}

// Potato-Kicker hardware style: glossy black knobs with a backlit orange-to-yellow ring, inset LED windows, console buttons.
class PotatoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PotatoLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, theme::accent);
        setColour(juce::Slider::trackColourId, theme::accent);
        setColour(juce::Slider::backgroundColourId, theme::grid);
        setColour(juce::Slider::textBoxTextColourId, juce::Colour((juce::uint32) 0xffb98c62));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, theme::text);
        setColour(juce::ComboBox::backgroundColourId, theme::display);
        setColour(juce::ComboBox::outlineColourId, theme::border);
        setColour(juce::ComboBox::textColourId, theme::yellow);
        setColour(juce::ComboBox::arrowColourId, theme::accent);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour((juce::uint32) 0xf7080809));
        setColour(juce::PopupMenu::textColourId, theme::text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, theme::accent);
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour((juce::uint32) 0xff1a0b02));
    }

    static juce::Colour ledColour(float t) { return look::ramp(t); }

    juce::Font getLabelFont(juce::Label& l) override
    {
        if (dynamic_cast<juce::Slider*>(l.getParentComponent()) != nullptr)
            return look::mono(12.5f);
        return l.getFont();
    }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return look::mono(14.f); }
    juce::Font getPopupMenuFont() override { return look::label(17.f, 0.05f); }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos,
                          float, float, juce::Slider&) override
    {
        const auto b = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h);
        const float size = juce::jmin(b.getWidth(), b.getHeight());
        look::drawLitKnob(g, b.getCentre(), size, pos, size >= 84.f, true);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float minPos, float maxPos,
                          juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, sliderPos, minPos, maxPos, style, s);
            return;
        }
        using namespace look;
        const float cy = (float) y + (float) h * 0.5f;
        const float x0 = (float) x + 8.f, x1 = (float) x + (float) w - 8.f;
        const float tx = juce::jlimit(x0, x1, sliderPos);

        g.setColour(col::tick.withAlpha(0.4f));
        for (int i = 0; i <= 10; ++i)
        {
            const float t = x0 + (x1 - x0) * (float) i / 10.f;
            g.drawLine(t, cy - 10.f, t, cy - (i % 5 == 0 ? 6.f : 7.5f), i % 5 == 0 ? 1.2f : 0.8f);
        }
        insetWindow(g, juce::Rectangle<float>(x0 - 6.f, cy - 4.f, x1 - x0 + 12.f, 8.f));
        if (tx > x0)
        {
            const juce::Rectangle<float> fill(x0 - 3.f, cy - 1.8f, tx - x0 + 3.f, 3.6f);
            g.setColour(col::amber.withAlpha(0.25f)); g.fillRoundedRectangle(fill.expanded(0.f, 3.f), 4.f);
            g.setGradientFill(juce::ColourGradient(col::orange, x0, 0.f, col::yellow, x1, 0.f, false));
            g.fillRoundedRectangle(fill, 1.8f);
        }
        const juce::Rectangle<float> cap(tx - 7.f, cy - 10.f, 14.f, 20.f);
        g.setColour(juce::Colours::black.withAlpha(0.6f)); g.fillRoundedRectangle(cap.translated(0.f, 2.f), 4.f);
        juce::ColourGradient body(juce::Colour((juce::uint32) 0xff4e4a46), cap.getX(), cap.getY(), juce::Colour((juce::uint32) 0xff020202), cap.getRight(), cap.getBottom(), false);
        body.addColour(0.35, juce::Colour((juce::uint32) 0xff1d1b1a));
        g.setGradientFill(body); g.fillRoundedRectangle(cap, 4.f);
        g.setColour(juce::Colours::black); g.drawRoundedRectangle(cap, 4.f, 0.8f);
        g.setColour(juce::Colours::white.withAlpha(0.18f)); g.fillRoundedRectangle(cap.reduced(2.f).withHeight(6.f), 2.f);
        g.setColour(ramp((tx - x0) / juce::jmax(1.f, x1 - x0)).withAlpha(0.3f)); g.fillRect(tx - 2.f, cap.getY() + 3.f, 4.f, cap.getHeight() - 6.f);
        g.setColour(juce::Colour((juce::uint32) 0xfffff1d8)); g.fillRect(tx - 0.8f, cap.getY() + 3.f, 1.6f, cap.getHeight() - 6.f);
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox&) override
    {
        const auto b = juce::Rectangle<float>(0.f, 0.f, (float) w, (float) h).reduced(1.f);
        look::insetWindow(g, b);
        if (down) { g.setColour(juce::Colours::white.withAlpha(0.04f)); g.fillRoundedRectangle(b, 3.f); }
        juce::Path arrow;
        const float ax = (float) w - 14.f, ay = (float) h * 0.5f;
        arrow.addTriangle(ax - 4.5f, ay - 2.5f, ax + 4.5f, ay - 2.5f, ax, ay + 3.5f);
        g.setColour(theme::accent);
        g.fillPath(arrow);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool over, bool down) override
    {
        using namespace look;
        auto r = btn.getLocalBounds().toFloat().reduced(1.f);
        if (down) r.translate(0.f, 1.f);
        juce::Colour ink;
        if (btn.getToggleState())
        {
            g.setColour(col::amber.withAlpha(0.28f)); g.fillRoundedRectangle(r.expanded(3), 5);
            g.setGradientFill(juce::ColourGradient(col::yellow.brighter(0.2f), 0, r.getY(), col::orange, 0, r.getBottom(), false));
            g.fillRoundedRectangle(r, 3);
            ink = juce::Colour((juce::uint32) 0xff1a0b02);
        }
        else
        {
            g.setGradientFill(juce::ColourGradient(juce::Colour(over ? 0xff261d16 : 0xff1d1713), 0, r.getY(), juce::Colour((juce::uint32) 0xff0e0b09), 0, r.getBottom(), false));
            g.fillRoundedRectangle(r, 3);
            g.setColour(col::line.withAlpha(over ? 0.75f : 0.45f)); g.drawRoundedRectangle(r.reduced(0.5f), 3, 1);
            ink = col::ink;
        }
        g.setColour(ink);
        g.setFont(label(14.f, 0.06f, true));
        g.drawText(btn.getButtonText(), r, juce::Justification::centred, false);
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
        label.setFont(look::label(15.f, 0.1f));
        label.setColour(juce::Label::textColourId, look::col::ink);
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds(b.removeFromTop(17));
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
        theme::glow(g, p, 1.8f, theme::accent);
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
        theme::glow(g, path, 2.f, theme::accent);

        for (auto pt : { p.p1, p.p2, p.p4 })
        {
            theme::node(g, pt.x, pt.y);
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
        theme::glow(g, path, 2.f, theme::accent);

        const float cx = r.getX() + r.getWidth() * std::log10(cutoff / 20.f) / 3.f;
        const float cy = dbToY(20.f * std::log10(juce::jmax(1.0e-5f, magnitude(cutoff))), r);
        theme::node(g, cx, cy);
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
        label.setFont(look::label(15.f, 0.1f));
        label.setColour(juce::Label::textColourId, look::col::ink);
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
        theme::glow(g, path, 2.f, theme::accent);

        for (auto pt : { handleThr(r), handleRatio(r) })
        {
            theme::node(g, pt.x, pt.y);
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
        theme::glow(g, path, 2.f, theme::accent);

        const auto h = handle(r);
        theme::node(g, h.x, h.y);
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

class LfoDisplay : public juce::Component, private juce::Timer
{
public:
    explicit LfoDisplay(juce::AudioProcessorValueTreeState& s) : apvts(s) { startTimerHz(15); }

    void paint(juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        theme::paintDisplayBg(g, b);

        const int shape = (int) apvts.getRawParameterValue(ID::lShape)->load();
        const float depth = apvts.getRawParameterValue(ID::lDepth)->load();
        const float expo = lfoCurveExp(apvts.getRawParameterValue(ID::lCurve)->load());
        const auto in = b.reduced(6.f);

        juce::Path p;
        for (int i = 0; i <= 120; ++i)
        {
            const float ph = (float) i / 120.f;
            const float v = 1.f - depth * (1.f - lfoShape(shape, ph, expo));
            const float x = in.getX() + ph * in.getWidth();
            const float y = in.getBottom() - v * in.getHeight();
            i == 0 ? p.startNewSubPath(x, y) : p.lineTo(x, y);
        }
        g.setColour(theme::accent);
        theme::glow(g, p, 1.8f, theme::accent);
    }

private:
    void timerCallback() override { repaint(); }
    juce::AudioProcessorValueTreeState& apvts;
};

class SeqDisplay : public juce::Component, private juce::Timer
{
public:
    SeqDisplay(juce::AudioProcessorValueTreeState& s, const std::atomic<int>& stepIn) : apvts(s), curStep(stepIn)
    {
        for (int i = 0; i < 16; ++i)
        {
            pOn[i] = s.getParameter(stepId("seq_on", i));
            pPit[i] = s.getParameter(stepId("seq_pit", i));
            pVel[i] = s.getParameter(stepId("seq_vel", i));
        }
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        theme::paintDisplayBg(g, getLocalBounds().toFloat(), true);
        const int oldMode = look::rampMode();
        look::rampMode() = 1;
        const int cur = curStep.load();
        const float cw = (float) getWidth() / 16.f;
        const juce::Colour deepBlue(0xff2b4cf0), deepPurple(0xff8a2fe0), deepMagenta(0xffe0259e);

        for (int i = 0; i < 16; ++i)
        {
            const bool on = pOn[i]->getValue() > 0.5f;
            const float x = (float) i * cw;
            const auto pit = pitchArea().withX(x).withWidth(cw).reduced(3.f, 0.f);
            const auto vel = velArea().withX(x).withWidth(cw).reduced(3.f, 0.f);
            const auto tog = toggleArea().withX(x).withWidth(cw).reduced(3.f, 2.f);

            if (i == cur)
            {
                g.setGradientFill(juce::ColourGradient(deepBlue.withAlpha(0.30f), 0.f, 0.f, deepMagenta.withAlpha(0.18f), 0.f, (float) getHeight(), false));
                g.fillRect(x, 0.f, cw, (float) getHeight());
                g.setColour(deepBlue.brighter(0.5f).withAlpha(0.9f));
                g.fillRect(x + 3.f, 0.f, cw - 6.f, 2.f);
            }

            g.setColour(deepPurple.withAlpha(0.28f));
            g.fillRect(pit.getX(), pit.getCentreY() - 0.5f, pit.getWidth(), 1.f);
            g.drawRect(pit, 1.f);
            g.drawRect(vel, 1.f);

            const float pv = pPit[i]->getValue();
            const float yv = pit.getBottom() - pv * pit.getHeight();
            const float y0 = std::min(yv, pit.getCentreY()), y1 = std::max(yv, pit.getCentreY());
            const juce::Rectangle<float> pbar(pit.getX() + 2.f, y0, pit.getWidth() - 4.f, std::max(2.f, y1 - y0));
            g.setGradientFill(juce::ColourGradient(look::ramp(pv).withAlpha(on ? 1.f : 0.3f), 0.f, pit.getBottom(),
                                                   look::ramp(1.f - pv * 0.3f).withAlpha(on ? 1.f : 0.3f), 0.f, pit.getY(), false));
            g.fillRect(pbar);
            if (on)
            {
                g.setColour(look::ramp(pv).withAlpha(0.25f));
                g.fillRect(pbar.expanded(2.f, 2.f));
                g.setColour(juce::Colours::white.withAlpha(0.55f));
                g.fillRect(pbar.getX(), yv - 0.5f, pbar.getWidth(), 1.5f);
            }

            const float vv = pVel[i]->getValue();
            const juce::Rectangle<float> vbar(vel.getX() + 2.f, vel.getBottom() - vv * vel.getHeight(), vel.getWidth() - 4.f, vv * vel.getHeight());
            g.setGradientFill(juce::ColourGradient(deepMagenta.withAlpha(on ? 1.f : 0.3f), 0.f, vel.getBottom(),
                                                   deepPurple.withAlpha(on ? 1.f : 0.3f), 0.f, vel.getY(), false));
            g.fillRect(vbar);

            if (on)
            {
                g.setColour(deepBlue.withAlpha(0.35f));
                g.fillRoundedRectangle(tog.expanded(2.f), 5.f);
                g.setGradientFill(juce::ColourGradient(deepBlue.brighter(0.2f), 0.f, tog.getY(), deepMagenta, 0.f, tog.getBottom(), false));
            }
            else
                g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff1a1740), 0.f, tog.getY(), juce::Colour((juce::uint32) 0xff0a0822), 0.f, tog.getBottom(), false));
            g.fillRoundedRectangle(tog, 3.f);
            if (!on)
            {
                g.setColour(deepPurple.withAlpha(0.5f));
                g.drawRoundedRectangle(tog.reduced(0.5f), 3.f, 1.f);
            }
            g.setColour(on ? juce::Colour((juce::uint32) 0xfff4ecff) : juce::Colour((juce::uint32) 0xff9a86d8));
            g.setFont(look::mono(12.f));
            g.drawText(juce::String(i + 1).paddedLeft('0', 2), tog.toNearestInt(), juce::Justification::centred);
        }
        look::rampMode() = oldMode;
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const int i = column(e.x);
        region = e.y >= toggleArea().getY() ? 2 : e.y >= velArea().getY() ? 1 : 0;
        if (region == 2)
        {
            pOn[i]->beginChangeGesture();
            pOn[i]->setValueNotifyingHost(pOn[i]->getValue() > 0.5f ? 0.f : 1.f);
            pOn[i]->endChangeGesture();
            return;
        }
        edit(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override { if (region < 2) edit(e); }
    void mouseUp(const juce::MouseEvent&) override { if (lastParam != nullptr) lastParam->endChangeGesture(); lastParam = nullptr; }

private:
    juce::Rectangle<float> pitchArea() const { return { 0.f, 4.f, (float) getWidth(), (float) getHeight() * 0.5f - 4.f }; }
    juce::Rectangle<float> velArea() const { const float y = (float) getHeight() * 0.5f + 4.f; return { 0.f, y, (float) getWidth(), (float) getHeight() * 0.5f - 30.f }; }
    juce::Rectangle<float> toggleArea() const { return { 0.f, (float) getHeight() - 24.f, (float) getWidth(), 22.f }; }
    int column(int x) const { return juce::jlimit(0, 15, x * 16 / juce::jmax(1, getWidth())); }

    void edit(const juce::MouseEvent& e)
    {
        const int i = column(e.x);
        auto* prm = region == 0 ? pPit[i] : pVel[i];
        const auto a = region == 0 ? pitchArea() : velArea();
        const float v = juce::jlimit(0.f, 1.f, 1.f - ((float) e.y - a.getY()) / a.getHeight());
        if (prm != lastParam)
        {
            if (lastParam != nullptr) lastParam->endChangeGesture();
            prm->beginChangeGesture();
            lastParam = prm;
        }
        prm->setValueNotifyingHost(prm->convertTo0to1(std::round(prm->convertFrom0to1(v) * (region == 0 ? 1.f : 100.f)) / (region == 0 ? 1.f : 100.f)));
    }

    void timerCallback() override { repaint(); }

    juce::AudioProcessorValueTreeState& apvts;
    const std::atomic<int>& curStep;
    juce::RangedAudioParameter *pOn[16], *pPit[16], *pVel[16];
    juce::RangedAudioParameter* lastParam = nullptr;
    int region = 0;
};

class ToggleBtn : public juce::Component
{
public:
    ToggleBtn(juce::AudioProcessorValueTreeState& s, const char* id, const juce::String& text)
        : attachment(s, id, button)
    {
        button.setButtonText(text);
        addAndMakeVisible(button);
    }
    void resized() override { button.setBounds(getLocalBounds()); }

private:
    juce::ToggleButton button;
    juce::AudioProcessorValueTreeState::ButtonAttachment attachment;
};

class PowerButton : public juce::ToggleButton
{
public:
    PowerButton() { setClickingTogglesState(true); setMouseCursor(juce::MouseCursor::PointingHandCursor); }

    void paintButton(juce::Graphics& g, bool hover, bool) override
    {
        const auto r = getLocalBounds().toFloat().withSizeKeepingCentre(14.f, 14.f);
        look::drawLed(g, r, getToggleState());
        if (hover)
        {
            g.setColour(look::col::amber.withAlpha(0.6f));
            g.drawEllipse(r.expanded(3.f), 1.f);
        }
    }
};
