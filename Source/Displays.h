#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include "Params.h"
#include "Wavetables.h"

namespace theme
{
    inline const juce::Colour bg      { (juce::uint32) 0xff04090f };
    inline const juce::Colour panel   { (juce::uint32) 0xff0c1822 };
    inline const juce::Colour display { (juce::uint32) 0xff050a10 };
    inline const juce::Colour grid    { (juce::uint32) 0xff12303a };
    inline const juce::Colour border  { (juce::uint32) 0xff1a4a5a };
    inline const juce::Colour accent  { (juce::uint32) 0xffffb020 };
    inline const juce::Colour yellow  { (juce::uint32) 0xffffd23a };
    inline const juce::Colour hot     { (juce::uint32) 0xffff5a2a };
    inline const juce::Colour cyan    { (juce::uint32) 0xff2fd6e0 };
    inline const juce::Colour text    { (juce::uint32) 0xffbfe9ee };
    inline const juce::Colour metalHi { (juce::uint32) 0xff4a6272 };
    inline const juce::Colour metalLo { (juce::uint32) 0xff070f16 };

    using Rect = juce::Rectangle<float>;

    // Soft neon glow: wide faint strokes under a bright core.
    inline void glow(juce::Graphics& g, const juce::Path& p, float w, juce::Colour c)
    {
        const float widths[3] { w + 6.f, w + 3.5f, w + 1.5f };
        const float alphas[3] { 0.07f, 0.13f, 0.24f };
        for (int i = 0; i < 3; ++i)
        {
            g.setColour(c.withAlpha(alphas[i]));
            g.strokePath(p, juce::PathStrokeType(widths[i], juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour(c.brighter(0.25f));
        g.strokePath(p, juce::PathStrokeType(w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    inline void screw(juce::Graphics& g, float cx, float cy, float r, float slotAngle)
    {
        g.setGradientFill(juce::ColourGradient(metalHi, cx - r, cy - r, metalLo, cx + r, cy + r, false));
        g.fillEllipse(cx - r, cy - r, 2.f * r, 2.f * r);
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.drawEllipse(cx - r, cy - r, 2.f * r, 2.f * r, 0.8f);
        g.drawLine(cx - r * 0.65f * std::cos(slotAngle), cy - r * 0.65f * std::sin(slotAngle),
                   cx + r * 0.65f * std::cos(slotAngle), cy + r * 0.65f * std::sin(slotAngle), 1.2f);
    }

    // Raised metal-and-glass module with corner screws and a glowing title LED.
    inline void paintPanel(juce::Graphics& g, Rect b, const juce::String& title)
    {
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(b, 8.f);
        const auto f = b.reduced(1.f);
        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff16303f), 0.f, f.getY(),
                                               juce::Colour((juce::uint32) 0xff08121a), 0.f, f.getBottom(), false));
        g.fillRoundedRectangle(f, 7.f);

        g.setColour(cyan.withAlpha(0.22f));
        g.drawLine(f.getX() + 8.f, f.getY() + 0.5f, f.getRight() - 8.f, f.getY() + 0.5f, 1.f);
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawLine(f.getX() + 8.f, f.getBottom() - 0.5f, f.getRight() - 8.f, f.getBottom() - 0.5f, 1.f);
        g.setColour(border.withAlpha(0.8f));
        g.drawRoundedRectangle(f.reduced(0.5f), 7.f, 1.f);

        screw(g, f.getX() + 7.f, f.getY() + 7.f, 3.f, 0.6f);
        screw(g, f.getRight() - 7.f, f.getY() + 7.f, 3.f, 2.1f);
        screw(g, f.getX() + 7.f, f.getBottom() - 7.f, 3.f, 1.3f);
        screw(g, f.getRight() - 7.f, f.getBottom() - 7.f, 3.f, 0.2f);

        const float ly = f.getY() + 15.f;
        g.setColour(accent.withAlpha(0.16f));
        g.fillEllipse(f.getX() + 14.f, ly - 6.f, 12.f, 12.f);
        g.setGradientFill(juce::ColourGradient(yellow, f.getX() + 18.f, ly - 2.f, hot, f.getX() + 22.f, ly + 3.f, true));
        g.fillEllipse(f.getX() + 17.f, ly - 3.f, 6.f, 6.f);

        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.setFont(juce::Font(juce::FontOptions(13.f, juce::Font::bold)));
        g.drawText(title, (int) f.getX() + 33, (int) f.getY() + 7, 220, 18, juce::Justification::left);
        g.setColour(cyan);
        g.drawText(title, (int) f.getX() + 32, (int) f.getY() + 6, 220, 18, juce::Justification::left);
    }

    // Recessed glass screen: bevelled rim, inner shadow, scanlines and a faint sheen.
    inline void paintDisplayBg(juce::Graphics& g, Rect b)
    {
        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff020509), 0.f, b.getY(),
                                               juce::Colour((juce::uint32) 0xff2a4a5a), 0.f, b.getBottom(), false));
        g.fillRoundedRectangle(b, 6.f);

        const auto in = b.reduced(2.f);
        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff03070c), 0.f, in.getY(),
                                               juce::Colour((juce::uint32) 0xff0a1b26), 0.f, in.getBottom(), false));
        g.fillRoundedRectangle(in, 4.5f);

        juce::Graphics::ScopedSaveState ss(g);
        juce::Path clip;
        clip.addRoundedRectangle(in, 4.5f);
        g.reduceClipRegion(clip);

        g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(0.65f), 0.f, in.getY(),
                                               juce::Colours::transparentBlack, 0.f, in.getY() + 16.f, false));
        g.fillRect(in.withHeight(16.f));
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.06f), 0.f, in.getY() + 2.f,
                                               juce::Colours::transparentWhite, 0.f, in.getY() + in.getHeight() * 0.45f, false));
        g.fillRect(in.withHeight(in.getHeight() * 0.45f));

        g.setColour(juce::Colours::black.withAlpha(0.18f));
        for (float y = in.getY() + 1.f; y < in.getBottom(); y += 3.f)
            g.fillRect(in.getX(), y, in.getWidth(), 1.f);

        g.setColour(cyan.withAlpha(0.28f));
        g.drawRoundedRectangle(in.reduced(0.5f), 4.5f, 1.f);
    }
}

// Hardware style: glowing segmented LED rings, machined metal knobs, recessed screens and LED buttons,
// in the dark-teal / amber-orange palette of the reference artwork.
class PotatoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PotatoLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, theme::accent);
        setColour(juce::Slider::trackColourId, theme::accent);
        setColour(juce::Slider::backgroundColourId, theme::grid);
        setColour(juce::Slider::textBoxTextColourId, theme::yellow);
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

    static juce::Colour ledColour(float t)
    {
        return t < 0.5f ? theme::yellow.interpolatedWith(theme::accent, t * 2.f)
                        : theme::accent.interpolatedWith(theme::hot, (t - 0.5f) * 2.f);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos,
                          float startA, float endA, juce::Slider&) override
    {
        const auto b = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(2.f);
        const float rOut = juce::jmin(b.getWidth(), b.getHeight()) * 0.5f;
        const float rIn = rOut * 0.76f;
        const float rBez = rIn * 0.93f;
        const float rCap = rBez * 0.86f;
        const float rTop = rCap * 0.84f;
        const auto c = b.getCentre();
        constexpr int segs = 27;

        for (int i = 0; i < segs; ++i)
        {
            const float t = (float) i / (float) (segs - 1);
            const float a = startA + t * (endA - startA);
            const float sx = std::sin(a), sy = -std::cos(a);
            if (t <= pos + 0.001f)
            {
                const auto col = ledColour(t);
                g.setColour(col.withAlpha(0.2f));
                g.drawLine(c.x + rIn * sx, c.y + rIn * sy, c.x + rOut * sx, c.y + rOut * sy, 5.f);
                g.setColour(col);
                g.drawLine(c.x + rIn * sx, c.y + rIn * sy, c.x + rOut * sx, c.y + rOut * sy, 2.4f);
            }
            else
            {
                g.setColour(juce::Colour((juce::uint32) 0xff0d242d));
                g.drawLine(c.x + rIn * sx, c.y + rIn * sy, c.x + rOut * sx, c.y + rOut * sy, 2.4f);
            }
        }

        g.setColour(theme::cyan.withAlpha(0.25f));
        g.drawEllipse(c.x - rIn * 0.97f, c.y - rIn * 0.97f, rIn * 1.94f, rIn * 1.94f, 0.8f);

        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillEllipse(c.x - rBez, c.y - rBez + 3.f, 2.f * rBez, 2.f * rBez);

        g.setGradientFill(juce::ColourGradient(theme::metalHi, c.x - rBez * 0.7f, c.y - rBez * 0.8f,
                                               theme::metalLo, c.x + rBez * 0.7f, c.y + rBez * 0.9f, false));
        g.fillEllipse(c.x - rBez, c.y - rBez, 2.f * rBez, 2.f * rBez);
        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.drawEllipse(c.x - rBez, c.y - rBez, 2.f * rBez, 2.f * rBez, 1.f);

        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff34495a), c.x - rCap * 0.4f, c.y - rCap * 0.5f,
                                               juce::Colour((juce::uint32) 0xff070e14), c.x + rCap * 0.2f, c.y + rCap * 1.1f, true));
        g.fillEllipse(c.x - rCap, c.y - rCap, 2.f * rCap, 2.f * rCap);

        const float ang = startA + pos * (endA - startA);
        constexpr int knurl = 36;
        for (int i = 0; i < knurl; ++i)
        {
            const float a = ang + (float) i * juce::MathConstants<float>::twoPi / (float) knurl;
            const float sx = std::sin(a), sy = -std::cos(a);
            g.setColour(juce::Colours::black.withAlpha(0.55f));
            g.drawLine(c.x + rCap * 0.9f * sx, c.y + rCap * 0.9f * sy, c.x + rCap * sx, c.y + rCap * sy, 1.2f);
            g.setColour(juce::Colours::white.withAlpha(0.05f));
            const float a2 = a + 0.045f;
            g.drawLine(c.x + rCap * 0.9f * std::sin(a2), c.y - rCap * 0.9f * std::cos(a2),
                       c.x + rCap * std::sin(a2), c.y - rCap * std::cos(a2), 0.8f);
        }
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.drawEllipse(c.x - rCap, c.y - rCap, 2.f * rCap, 2.f * rCap, 1.f);

        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff263744), c.x, c.y - rTop,
                                               juce::Colour((juce::uint32) 0xff0a131a), c.x, c.y + rTop, false));
        g.fillEllipse(c.x - rTop, c.y - rTop, 2.f * rTop, 2.f * rTop);
        g.setColour(theme::cyan.withAlpha(0.3f));
        g.drawEllipse(c.x - rTop, c.y - rTop, 2.f * rTop, 2.f * rTop, 1.f);

        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.2f), c.x, c.y - rTop * 0.9f,
                                               juce::Colours::transparentWhite, c.x, c.y - rTop * 0.1f, false));
        g.fillEllipse(c.x - rTop * 0.72f, c.y - rTop * 0.9f, rTop * 1.44f, rTop * 0.8f);

        const float sx = std::sin(ang), sy = -std::cos(ang);
        g.setColour(theme::cyan.withAlpha(0.25f));
        g.drawLine(c.x + rTop * 0.3f * sx, c.y + rTop * 0.3f * sy, c.x + rTop * 0.92f * sx, c.y + rTop * 0.92f * sy, 6.f);
        g.setColour(theme::cyan.brighter(0.3f));
        g.drawLine(c.x + rTop * 0.3f * sx, c.y + rTop * 0.3f * sy, c.x + rTop * 0.92f * sx, c.y + rTop * 0.92f * sy, 2.4f);

        const float rh = rTop * 0.13f;
        g.setColour(juce::Colours::black);
        g.fillEllipse(c.x - rh * 1.5f, c.y - rh * 1.5f, rh * 3.f, rh * 3.f);
        g.setColour(theme::cyan);
        g.drawEllipse(c.x - rh * 1.5f, c.y - rh * 1.5f, rh * 3.f, rh * 3.f, 1.f);
        g.fillEllipse(c.x - rh * 0.6f, c.y - rh * 0.6f, rh * 1.2f, rh * 1.2f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float minPos, float maxPos,
                          juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, sliderPos, minPos, maxPos, style, s);
            return;
        }

        const float cy = (float) y + (float) h * 0.5f;
        const float x0 = (float) x + 8.f, x1 = (float) x + (float) w - 8.f;
        const float tx = juce::jlimit(x0, x1, sliderPos);

        g.setColour(theme::cyan.withAlpha(0.18f));
        for (int i = 0; i <= 10; ++i)
        {
            const float tx2 = x0 + (x1 - x0) * (float) i / 10.f;
            g.drawLine(tx2, cy - 9.f, tx2, cy - 6.f, 1.f);
        }

        const juce::Rectangle<float> groove(x0 - 4.f, cy - 3.5f, x1 - x0 + 8.f, 7.f);
        g.setGradientFill(juce::ColourGradient(juce::Colours::black, 0.f, groove.getY(),
                                               juce::Colour((juce::uint32) 0xff1a3744), 0.f, groove.getBottom(), false));
        g.fillRoundedRectangle(groove, 3.5f);
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.drawRoundedRectangle(groove, 3.5f, 1.f);

        if (tx > x0)
        {
            const juce::Rectangle<float> fill(x0 - 3.f, cy - 2.f, tx - x0 + 3.f, 4.f);
            g.setColour(theme::accent.withAlpha(0.22f));
            g.fillRoundedRectangle(fill.expanded(0.f, 3.f), 4.f);
            g.setGradientFill(juce::ColourGradient(theme::yellow, fill.getX(), 0.f, theme::hot, x1, 0.f, false));
            g.fillRoundedRectangle(fill, 2.f);
        }

        const juce::Rectangle<float> thumb(tx - 7.f, cy - 10.f, 14.f, 20.f);
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.fillRoundedRectangle(thumb.translated(0.f, 2.f), 3.f);
        g.setGradientFill(juce::ColourGradient(theme::metalHi, thumb.getX(), thumb.getY(),
                                               theme::metalLo, thumb.getRight(), thumb.getBottom(), false));
        g.fillRoundedRectangle(thumb, 3.f);
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.2f), 0.f, thumb.getY(),
                                               juce::Colours::transparentWhite, 0.f, thumb.getCentreY(), false));
        g.fillRoundedRectangle(thumb.reduced(1.f).withHeight(9.f), 2.f);
        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.drawRoundedRectangle(thumb, 3.f, 1.f);
        g.setColour(theme::cyan);
        g.fillRect(tx - 0.75f, thumb.getY() + 3.f, 1.5f, thumb.getHeight() - 6.f);
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox&) override
    {
        const auto b = juce::Rectangle<float>(0.f, 0.f, (float) w, (float) h);
        g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff02060a), 0.f, 0.f,
                                               juce::Colour((juce::uint32) 0xff0f2431), 0.f, (float) h, false));
        g.fillRoundedRectangle(b, 4.f);
        g.setColour(down ? theme::accent : theme::cyan.withAlpha(0.45f));
        g.drawRoundedRectangle(b.reduced(0.5f), 4.f, 1.f);
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.drawLine(4.f, 1.5f, (float) w - 4.f, 1.5f, 1.f);

        juce::Path arrow;
        const float ax = (float) w - 14.f, ay = (float) h * 0.5f;
        arrow.addTriangle(ax - 4.f, ay - 2.f, ax + 4.f, ay - 2.f, ax, ay + 3.f);
        g.setColour(theme::cyan);
        g.fillPath(arrow);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool, bool down) override
    {
        const auto b = btn.getLocalBounds().toFloat().reduced(1.f);
        const bool on = btn.getToggleState();
        if (on)
        {
            g.setColour(theme::accent.withAlpha(0.25f));
            g.fillRoundedRectangle(b.expanded(1.f), 6.f);
            g.setGradientFill(juce::ColourGradient(theme::yellow, 0.f, b.getY(), theme::hot, 0.f, b.getBottom(), false));
        }
        else
            g.setGradientFill(juce::ColourGradient(juce::Colour((juce::uint32) 0xff1b2f3b), 0.f, b.getY(),
                                                   juce::Colour((juce::uint32) 0xff070f16), 0.f, b.getBottom(), false));
        g.fillRoundedRectangle(b, 5.f);
        g.setColour(juce::Colours::white.withAlpha(down ? 0.05f : 0.14f));
        g.fillRoundedRectangle(b.reduced(2.f).withHeight(b.getHeight() * 0.4f), 3.f);
        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.drawRoundedRectangle(b, 5.f, 1.f);
        g.setColour(on ? theme::bg : theme::text.withAlpha(0.7f));
        g.setFont(juce::Font(juce::FontOptions(13.f, juce::Font::bold)));
        g.drawText(btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred);
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
        theme::glow(g, path, 2.f, theme::accent);

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
        theme::glow(g, path, 2.f, theme::accent);

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
        theme::glow(g, path, 2.f, theme::accent);

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
        theme::paintDisplayBg(g, getLocalBounds().toFloat());
        const int cur = curStep.load();
        const float cw = (float) getWidth() / 16.f;

        for (int i = 0; i < 16; ++i)
        {
            const bool on = pOn[i]->getValue() > 0.5f;
            const float x = (float) i * cw;
            const auto pit = pitchArea().withX(x).withWidth(cw).reduced(3.f, 0.f);
            const auto vel = velArea().withX(x).withWidth(cw).reduced(3.f, 0.f);
            const auto tog = toggleArea().withX(x).withWidth(cw).reduced(3.f, 2.f);

            if (i == cur)
            {
                g.setColour(theme::cyan.withAlpha(0.14f));
                g.fillRect(x, 0.f, cw, (float) getHeight());
            }

            g.setColour(theme::grid);
            g.fillRect(pit.getX(), pit.getCentreY() - 0.5f, pit.getWidth(), 1.f);
            g.drawRect(pit, 1.f);
            g.drawRect(vel, 1.f);

            const float pv = pPit[i]->getValue();
            const float yv = pit.getBottom() - pv * pit.getHeight();
            g.setColour(on ? theme::accent : theme::accent.withAlpha(0.3f));
            const float y0 = std::min(yv, pit.getCentreY()), y1 = std::max(yv, pit.getCentreY());
            g.fillRect(pit.getX() + 2.f, y0, pit.getWidth() - 4.f, std::max(2.f, y1 - y0));

            const float vv = pVel[i]->getValue();
            g.setColour(on ? theme::cyan : theme::cyan.withAlpha(0.3f));
            g.fillRect(vel.getX() + 2.f, vel.getBottom() - vv * vel.getHeight(), vel.getWidth() - 4.f, vv * vel.getHeight());

            g.setColour(on ? theme::hot : theme::grid);
            g.fillRoundedRectangle(tog, 3.f);
            g.setColour(on ? theme::bg : theme::text.withAlpha(0.6f));
            g.setFont(juce::Font(juce::FontOptions(11.f)));
            g.drawText(juce::String(i + 1), tog.toNearestInt(), juce::Justification::centred);
        }
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
        const auto b = getLocalBounds().toFloat().reduced(2.f);
        const auto c = b.getCentre();
        const float r = juce::jmin(b.getWidth(), b.getHeight()) * 0.5f;
        const bool on = getToggleState();

        if (on)
        {
            g.setColour(theme::accent.withAlpha(0.28f));
            g.fillEllipse(c.x - r - 3.f, c.y - r - 3.f, 2.f * r + 6.f, 2.f * r + 6.f);
        }
        g.setGradientFill(juce::ColourGradient(on ? theme::yellow : juce::Colour((juce::uint32) 0xff2b3f4c), c.x, c.y - r,
                                               on ? theme::hot : juce::Colour((juce::uint32) 0xff070f16), c.x, c.y + r, false));
        g.fillEllipse(c.x - r, c.y - r, 2.f * r, 2.f * r);
        g.setColour(juce::Colours::black.withAlpha(0.85f));
        g.drawEllipse(c.x - r, c.y - r, 2.f * r, 2.f * r, 1.f);

        juce::Path icon;
        const float ir = r * 0.5f;
        icon.addCentredArc(c.x, c.y + 0.5f, ir, ir, 0.f, 0.65f, juce::MathConstants<float>::twoPi - 0.65f, true);
        icon.startNewSubPath(c.x, c.y - ir - 1.f);
        icon.lineTo(c.x, c.y);
        g.setColour(on ? theme::bg : (hover ? theme::text : theme::text.withAlpha(0.5f)));
        g.strokePath(icon, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
};
