#pragma once
#include <functional>
#include "Look.h"
#include "Params.h"
#include "Presets.h"
#include "Wavetables.h"

// Console widgets ported from Potato-Kicker: VU meter, strip buttons, 7-segment, preset browser, level lights, potato badge.
namespace look
{
using namespace juce;

class VuMeter : public Component
{
public:
    void setLevel (float rms)
    {
        const float db = 20.0f * std::log10 (rms + 1.0e-6f) + 10.0f;   // 0 VU = -10 dBFS RMS
        level += (db - level) * (db > level ? 0.35f : 0.06f);
        repaint();
    }

    void paint (Graphics& g) override
    {
        const Point<float> pivot (125, 176);
        const float R = 118.0f;
        g.setColour (Colour (0xff060504)); g.fillRoundedRectangle (1, 1, 248, 148, 12);
        g.setColour (col::line.withAlpha (0.35f)); g.drawRoundedRectangle (1, 1, 248, 148, 12, 1.2f);
        const Rectangle<float> face (13, 13, 224, 102);
        {
            Graphics::ScopedSaveState s (g);
            Path clip; clip.addRoundedRectangle (face, 7.0f);
            g.reduceClipRegion (clip);
            g.setGradientFill (ColourGradient (Colour (0xff160d06), 0, face.getY(), Colour (0xff050302), 0, face.getBottom(), false));
            g.fillRect (face);
            g.setGradientFill (ColourGradient (Colour (0x38ff8a1e), 125, 122, Colour (0x00ff8a1e), 125 + 150, 122, true));
            g.fillRect (face);
            g.setColour (Colour (0x10ff8a1e));
            for (float x = face.getX() + 8; x < face.getRight(); x += 12) g.drawVerticalLine ((int) x, face.getY(), face.getBottom());
            for (float y = face.getY() + 6; y < face.getBottom(); y += 12) g.drawHorizontalLine ((int) y, face.getX(), face.getRight());

            Path arc;  arc.addCentredArc (pivot.x, pivot.y, R, R, 0, degToRad (ang (-20)), degToRad (ang (0)), true);
            g.setGradientFill (ColourGradient (col::orange, 20, 0, col::yellow, 230, 0, false)); g.strokePath (arc, PathStrokeType (1.6f));
            Path red;  red.addCentredArc (pivot.x, pivot.y, R, R, 0, degToRad (ang (0)), degToRad (ang (3)), true);
            g.setColour (col::faceRed); g.strokePath (red, PathStrokeType (4.0f));
            g.setFont (label (11.5f));
            for (int v : { -20, -10, -7, -5, -3, -2, -1, 0, 1, 2, 3 })
            {
                const float a = ang ((float) v);
                const auto p1 = polar (pivot, R - 9, a), p2 = polar (pivot, R, a), pt = polar (pivot, R + 11, a);
                g.setColour (v > 0 ? col::faceRed : ramp ((v + 20.0f) / 20.0f));
                g.drawLine (p1.x, p1.y, p2.x, p2.y, 1.4f);
                g.drawText (v > 0 ? "+" + String (v) : String (v), Rectangle<float> (24, 12).withCentre (pt), Justification::centred, false);
            }
            g.setFont (label (13.5f, 0.0f, true));
            g.setColour (col::faceInk); g.drawText ("VU", Rectangle<float> (30, 22, 30, 14), Justification::left, false);
            g.setColour (col::faceRed); g.drawText ("VU", Rectangle<float> (190, 22, 30, 14), Justification::right, false);
            g.setColour (col::amber.withAlpha (0.45f)); g.setFont (label (9.0f, 0.25f));
            g.drawText ("POTATO BASS", Rectangle<float> (60, 90, 130, 12), Justification::centred, false);
            const auto tip = polar (pivot, R + 4, ang (jlimit (-20.0f, 3.5f, level)));
            g.setColour (col::amber.withAlpha (0.30f)); g.drawLine (pivot.x, pivot.y, tip.x, tip.y, 5.0f);
            g.setColour (col::yellow.brighter (0.3f)); g.drawLine (pivot.x, pivot.y, tip.x, tip.y, 1.6f);
        }
        g.setFont (label (14.5f, 0.3f));
        glowText (g, "OUTPUT", Rectangle<float> (60, 125, 130, 17), Justification::centred, col::ink, 0.22f);
        drawLed (g, Rectangle<float> (11, 11).withCentre ({ 236.5f, 126.5f }), level > 0.0f);
    }

public:
    float getLevel() const noexcept { return level; }

private:
    static float ang (float v)
    {
        const float p = (std::pow (10.0f, v / 20.0f) - 0.1f) / (1.41254f - 0.1f);
        return -46.0f + 92.0f * jlimit (0.0f, 1.0f, p);
    }
    float level = -25.0f;
};

class SquareButton : public Button
{
public:
    enum Kind { arrowLeft, arrowRight, text };
    SquareButton (Kind k, String t = {}) : Button (t), kind (k) { setTitle (t); }

    void paintButton (Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        if (down) r.translate (0, 1);
        Colour ink;
        if (getToggleState())
        {
            g.setColour (col::amber.withAlpha (0.28f)); g.fillRoundedRectangle (r.expanded (3), 5);
            g.setGradientFill (ColourGradient (col::yellow.brighter (0.2f), 0, r.getY(), col::orange, 0, r.getBottom(), false));
            g.fillRoundedRectangle (r, 3);
            ink = Colour (0xff1a0b02);
        }
        else
        {
            g.setGradientFill (ColourGradient (Colour (over ? 0xff261d16 : 0xff1d1713), 0, r.getY(), Colour (0xff0e0b09), 0, r.getBottom(), false));
            g.fillRoundedRectangle (r, 3);
            g.setColour (col::line.withAlpha (over ? 0.75f : 0.45f)); g.drawRoundedRectangle (r.reduced (0.5f), 3, 1);
            ink = col::ink;
        }
        g.setColour (ink);
        if (kind == text)
        {
            g.setFont (label (14.0f, 0.06f, true));
            g.drawText (getButtonText(), r, Justification::centred, false);
        }
        else
        {
            const auto c = r.getCentre();
            Path tri;
            if (kind == arrowLeft) tri.addTriangle (c.x - 5, c.y, c.x + 4, c.y - 5.5f, c.x + 4, c.y + 5.5f);
            else                   tri.addTriangle (c.x + 5, c.y, c.x - 4, c.y - 5.5f, c.x - 4, c.y + 5.5f);
            g.fillPath (tri);
        }
    }

private:
    Kind kind;
};

class LedWindow : public Button
{
public:
    LedWindow (bool redStyle) : Button ("window"), red (redStyle) {}
    void setText (const String& s) { if (s != txt) { txt = s; repaint(); } }
    void paintButton (Graphics& g, bool over, bool) override
    {
        const auto r = getLocalBounds().toFloat().reduced (1.0f);
        insetWindow (g, r);
        if (over) { g.setColour (Colours::white.withAlpha (0.03f)); g.fillRoundedRectangle (r, 3); }
        if (red)
        {
            g.setFont (mono (19.0f));
            glowTextRamp (g, txt, r.reduced (14.0f, 0.0f), Justification::centred, 0.35f);
        }
        else
        {
            g.setFont (mono (18.0f));
            glowText (g, txt, r.withTrimmedLeft (12).withTrimmedRight (10), Justification::centredLeft, col::yellow, 0.25f);
        }
    }
private:
    bool red;
    String txt;
};

class SevenSeg : public Component
{
public:
    void setNumber (int n) { if (n != num) { num = n; repaint(); } }
    void paint (Graphics& g) override
    {
        insetWindow (g, getLocalBounds().toFloat().reduced (1.0f));
        static const char* segs[10] { "abcdef", "bc", "abged", "abgcd", "fgbc", "afgcd", "afgedc", "abc", "abcdefg", "abfgcd" };
        struct R { char k; float x, y, w, h; };
        const R rs[7] { { 'a', 5, 1, 13, 3.2f }, { 'b', 18.5f, 4, 3.2f, 13 }, { 'c', 18.5f, 21, 3.2f, 13 }, { 'd', 5, 34.8f, 13, 3.2f },
                        { 'e', 1.3f, 21, 3.2f, 13 }, { 'f', 1.3f, 4, 3.2f, 13 }, { 'g', 5, 17.9f, 13, 3.2f } };
        const String s = String (num).paddedLeft ('0', 2);
        for (int i = 0; i < 2; ++i)
        {
            const String on (segs[s[i] - '0']);
            const auto xf = AffineTransform::shear (-0.105f, 0.0f).translated (10.0f + i * 25.0f, 1.0f);
            for (auto& r : rs)
            {
                Path p; p.addRoundedRectangle (r.x, r.y, r.w, r.h, 1.2f); p.applyTransform (xf);
                const bool lit = on.containsChar (r.k);
                const Colour segCol = ramp (1.0f - (r.y + r.h * 0.5f) / 38.0f);   // yellow at the top, orange at the bottom
                if (lit) { g.setColour (segCol.withAlpha (0.35f)); g.strokePath (p, PathStrokeType (3.0f)); }
                g.setColour (lit ? segCol : Colour (0xff26120a));
                g.fillPath (p);
            }
        }
    }
private:
    int num = 1;
};

class LedDot : public Component
{
public:
    void setOn (bool b) { if (b != on) { on = b; repaint(); } }
    void paint (Graphics& g) override { drawLed (g, getLocalBounds().toFloat().reduced (8.0f), on); }
private:
    bool on = false;
};

class TriggerButton : public Component
{
public:
    std::function<void (bool)> onPress;
    void setLit (bool b) { if (b != lit) { lit = b; repaint(); } }
    void paint (Graphics& g) override
    {
        // glowing orange ring button, like the round controls in the HUD reference
        const auto r = getLocalBounds().toFloat().reduced (2.0f);
        const auto c = r.getCentre();
        g.setColour (Colours::black.withAlpha (0.6f)); g.fillEllipse (r.translated (0, 3));
        g.setColour (Colour (0xff0b0908)); g.fillEllipse (r);
        g.setColour (col::line.withAlpha (0.35f)); g.drawEllipse (r.reduced (0.5f), 1.0f);
        const auto ring = r.reduced (r.getWidth() * 0.16f);
        strokeRampRing (g, c, ring.getWidth() * 0.5f, lit ? 11.0f : 8.0f, lit ? 0.35f : 0.16f);
        strokeRampRing (g, c, ring.getWidth() * 0.5f, 3.4f, lit ? 1.0f : 0.85f);
        g.setColour (Colour (0xfffff0c8).withAlpha (lit ? 0.95f : 0.45f)); g.drawEllipse (ring, 1.0f);
        g.setColour (Colour (0xff120c08)); g.fillEllipse (ring.reduced (4.0f));
        if (lit)
        {
            g.setGradientFill (ColourGradient (col::amber.withAlpha (0.45f), c.x, c.y, col::amber.withAlpha (0.0f), c.x + ring.getWidth() * 0.5f, c.y, true));
            g.fillEllipse (ring.reduced (4.0f));
        }
        g.setGradientFill (ColourGradient (col::orange, c.x - ring.getWidth() * 0.21f, 0, col::yellow, c.x + ring.getWidth() * 0.21f, 0, false));
        g.fillRoundedRectangle (Rectangle<float> (ring.getWidth() * 0.42f, 3.2f).withCentre (c), 1.6f);
    }
    void mouseDown (const MouseEvent&) override { if (onPress) onPress (true); }
    void mouseUp (const MouseEvent&) override   { if (onPress) onPress (false); }
private:
    bool lit = false;
};

//==============================================================================
// Row of console LEDs across the preset strip that light up with the output level, orange to yellow
class LevelLights : public Component
{
public:
    void setLevel (float vuDb) { const int n = jlimit (0, count, roundToInt (jmap (vuDb, -24.0f, 3.0f, 0.0f, (float) count))); if (n != lit) { lit = n; repaint(); } }
    void paint (Graphics& g) override
    {
        const float step = (float) getWidth() / count, cy = getHeight() * 0.5f;
        for (int i = 0; i < count; ++i)
        {
            const auto r = Rectangle<float> (step * 0.62f, 4.0f).withCentre ({ step * (i + 0.5f), cy });
            const Colour c = ramp ((float) i / (count - 1));
            if (i < lit)
            {
                g.setColour (c.withAlpha (0.3f)); g.fillRoundedRectangle (r.expanded (2.5f, 2.5f), 3.0f);
                g.setColour (c.brighter (0.2f));  g.fillRoundedRectangle (r, 2.0f);
            }
            else { g.setColour (c.withAlpha (0.13f)); g.fillRoundedRectangle (r, 2.0f); }
        }
    }
private:
    static constexpr int count = 48;
    int lit = 0;
};

// Potato emblem: clean orange line art with a calm face. The eyes glow brighter while notes play.
class PotatoBadge : public Component
{
public:
    void setActivity (float a)
    {
        a = jlimit (0.0f, 1.0f, a);
        if (std::abs (a - act) > 0.01f) { act = a; repaint(); }
    }

    void paint (Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        const Point<float> c (b.getCentreX(), b.getCentreY() - 6.0f);
        const float R = jmin (b.getWidth(), b.getHeight()) * 0.37f;
        const Path spud = potatoPath (c, R);

        // backlight
        g.setGradientFill (ColourGradient (col::amber.withAlpha (0.16f + 0.14f * act), c.x, c.y, col::amber.withAlpha (0.0f), c.x + R * 1.7f, c.y, true));
        g.fillEllipse (Rectangle<float> (R * 3.4f, R * 2.7f).withCentre (c));

        // body and glowing outline
        g.setGradientFill (ColourGradient (Colour (0xff3a200c), c.x - R * 0.35f, c.y - R * 0.45f, Colour (0xff0d0704), c.x + R * 0.95f, c.y + R * 0.8f, true));
        g.fillPath (spud);
        const auto sb = spud.getBounds();
        auto rampFill = [&] (float a) { return ColourGradient (col::orange.withAlpha (a), sb.getX(), sb.getBottom(), col::yellow.withAlpha (a), sb.getRight(), sb.getY(), false); };
        g.setGradientFill (rampFill (0.16f)); g.strokePath (spud, PathStrokeType (9.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setGradientFill (rampFill (0.40f)); g.strokePath (spud, PathStrokeType (4.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setGradientFill (rampFill (1.0f));  g.strokePath (spud, PathStrokeType (1.7f, PathStrokeType::curved, PathStrokeType::rounded));

        // skin highlight and the little potato "eyes" (sprout spots)
        Path hl; hl.addCentredArc (c.x - R * 0.1f, c.y + R * 0.02f, R * 1.0f, R * 0.66f, degToRad (-14.0f), degToRad (-78.0f), degToRad (-22.0f), true);
        g.setColour (col::amberHi.withAlpha (0.35f)); g.strokePath (hl, PathStrokeType (1.2f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (col::amber.withAlpha (0.5f));
        for (auto p : { Point<float> (-0.86f, 0.34f), Point<float> (0.88f, -0.40f), Point<float> (0.66f, 0.52f), Point<float> (-0.48f, 0.62f), Point<float> (-0.10f, -0.70f) })
            g.drawEllipse (Rectangle<float> (3.4f, 2.2f).withCentre (c + p * R), 1.0f);

        // face: calm round eyes with glowing pupils, small smile
        for (int i = 0; i < 2; ++i)
        {
            const Point<float> e = c + Point<float> (i == 0 ? -0.34f : 0.30f, i == 0 ? -0.16f : -0.19f) * R;
            const auto socket = Rectangle<float> (R * 0.30f, R * 0.36f).withCentre (e);
            g.setColour (Colour (0xff080402)); g.fillEllipse (socket);
            g.setColour (col::amber.withAlpha (0.85f)); g.drawEllipse (socket, 1.3f);
            const float pr = R * (0.075f + 0.02f * act);
            const Point<float> pc = e + Point<float> (R * 0.025f, R * 0.03f);
            g.setColour (col::amber.withAlpha (0.25f + 0.4f * act)); g.fillEllipse (Rectangle<float> (pr * 4.0f, pr * 4.0f).withCentre (pc));
            g.setGradientFill (ColourGradient (Colour (0xfffff3c8), pc.x - pr * 0.3f, pc.y - pr * 0.3f, col::yellow, pc.x + pr, pc.y + pr, true));
            g.fillEllipse (Rectangle<float> (pr * 2.0f, pr * 2.0f).withCentre (pc));
            g.setColour (Colours::white.withAlpha (0.85f)); g.fillEllipse (Rectangle<float> (pr * 0.6f, pr * 0.6f).withCentre (pc - Point<float> (pr * 0.35f, pr * 0.4f)));
        }
        Path mouth;
        mouth.startNewSubPath (c.x - R * 0.22f, c.y + R * 0.30f);
        mouth.quadraticTo (c.x - R * 0.02f, c.y + R * 0.46f, c.x + R * 0.22f, c.y + R * 0.27f);
        g.setColour (col::amber.withAlpha (0.3f)); g.strokePath (mouth, PathStrokeType (4.5f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (col::yellow);                 g.strokePath (mouth, PathStrokeType (1.9f, PathStrokeType::curved, PathStrokeType::rounded));

        // HUD caption
        g.setColour (Colour (0xffb98c62)); g.setFont (mono (11.5f));
        g.drawText ("SPUD-01", Rectangle<float> (0, b.getBottom() - 14, b.getWidth(), 12), Justification::centred, false);
    }

private:
    static Path potatoPath (Point<float> c, float R)
    {
        const float radii[16] { 1.02f, 0.97f, 0.90f, 0.93f, 0.99f, 0.95f, 0.91f, 0.98f, 1.06f, 1.03f, 0.94f, 0.90f, 0.96f, 1.04f, 1.00f, 1.05f };
        const float tilt = degToRad (-10.0f), ct = std::cos (tilt), st = std::sin (tilt);
        Point<float> pts[16];
        for (int i = 0; i < 16; ++i)
        {
            const float a = degToRad (i * 22.5f);
            const float x = std::cos (a) * R * 1.22f * radii[i], y = std::sin (a) * R * 1.0f * radii[i];
            pts[i] = c + Point<float> (x * ct - y * st, x * st + y * ct);
        }
        Path p;
        auto mid = [&] (int i, int j) { return (pts[i] + pts[j]) * 0.5f; };
        p.startNewSubPath (mid (15, 0));
        for (int i = 0; i < 16; ++i) p.quadraticTo (pts[i], mid (i, (i + 1) % 16));
        p.closeSubPath();
        return p;
    }

    float act = 0.0f;
};

// Preset browser overlay: genres on the left, the genre's 10 presets on the right
class PresetBrowser : public Component
{
public:
    std::function<void (int)> onPick;

    void show (int currentPreset)
    {
        current = currentPreset;
        genre = currentPreset / pb::presetsPerGenre;
        setVisible (true);
        toFront (false);
        repaint();
    }

    void paint (Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        g.setColour (Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (b.translated (0, 12), 12);
        g.setColour (Colour (0xf7080809)); g.fillRoundedRectangle (b, 12);
        g.setColour (col::line.withAlpha (0.45f)); g.drawRoundedRectangle (b.reduced (0.5f), 12, 1);
        g.setColour (col::line.withAlpha (0.2f)); g.drawVerticalLine (210, 0, b.getBottom());

        for (int i = 0; i < pb::numGenres; ++i)
        {
            const auto r = genreRect (i);
            if (i == genre) { g.setColour (Colour (0xff1e1006)); g.fillRect (r); }
            g.setFont (label (16.0f, 0.12f));
            if (i == genre) glowText (g, String (pb::genreNames[i]).toUpperCase(), r.withTrimmedLeft (22), Justification::centredLeft, col::amber, 0.2f);
            else { g.setColour (col::inkDim); g.drawText (String (pb::genreNames[i]).toUpperCase(), r.withTrimmedLeft (22), Justification::centredLeft, false); }
            g.setColour (col::inkDim.withAlpha (0.6f)); g.setFont (mono (11.5f));
            g.drawText ("10", r.withTrimmedRight (20), Justification::centredRight, false);
        }

        g.setColour (col::ink); g.setFont (label (17.0f, 0.22f));
        g.drawText (String (pb::genreNames[genre]).toUpperCase(), Rectangle<float> (232, 18, 400, 28), Justification::centredLeft, false);
        const auto xr = closeRect();
        g.setColour (Colour (0xff1d1d20)); g.fillRoundedRectangle (xr, 4);
        g.setColour (col::ink); g.drawLine (xr.getX() + 10, xr.getY() + 10, xr.getRight() - 10, xr.getBottom() - 10, 1.6f);
        g.drawLine (xr.getRight() - 10, xr.getY() + 10, xr.getX() + 10, xr.getBottom() - 10, 1.6f);

        for (int i = 0; i < pb::presetsPerGenre; ++i)
        {
            const int idx = genre * pb::presetsPerGenre + i;
            const auto& p = pb::presets[idx];
            const auto r = presetRect (i);
            const bool on = idx == current, over = idx == hoverIdx;
            g.setColour (on ? Colour (0xff1b150c) : over ? Colour (0xff1a1a1d) : Colour (0xff121214)); g.fillRoundedRectangle (r, 6);
            g.setColour (on ? col::amber : col::line.withAlpha (0.2f)); g.drawRoundedRectangle (r.reduced (0.5f), 6, 1);
            g.setColour (col::amber); g.setFont (mono (15.0f));
            g.drawText (String (i + 1).paddedLeft ('0', 2), Rectangle<float> (r.getX() + 14, r.getY(), 30, r.getHeight()), Justification::centredLeft, false);
            g.setColour (col::ink); g.setFont (label (16.5f, 0.05f));
            g.drawText (p.name, Rectangle<float> (r.getX() + 50, r.getY() + 8, r.getWidth() - 56, 20), Justification::centredLeft, false);
            g.setColour (col::inkDim); g.setFont (mono (11.5f));
            const String meta = Wavetables::names()[(int) p.v[0]] + " + " + Wavetables::names()[(int) p.v[4]] + " | " + String ((int) p.v[13]) + " Hz";
            g.drawText (meta, Rectangle<float> (r.getX() + 50, r.getY() + 28, r.getWidth() - 56, 14), Justification::centredLeft, false);
        }
    }

    void mouseMove (const MouseEvent& e) override
    {
        int h = -1;
        for (int i = 0; i < pb::presetsPerGenre; ++i)
            if (presetRect (i).contains (e.position)) h = genre * pb::presetsPerGenre + i;
        if (h != hoverIdx) { hoverIdx = h; repaint(); }
    }

    void mouseDown (const MouseEvent& e) override
    {
        if (closeRect().contains (e.position)) { setVisible (false); return; }
        for (int i = 0; i < pb::numGenres; ++i)
            if (genreRect (i).contains (e.position)) { genre = i; repaint(); return; }
        for (int i = 0; i < pb::presetsPerGenre; ++i)
            if (presetRect (i).contains (e.position))
            {
                current = genre * pb::presetsPerGenre + i;
                if (onPick) onPick (current);
                repaint();
                return;
            }
    }

private:
    Rectangle<float> genreRect (int i) const  { return { 0, 16.0f + i * 35.0f, 210, 35 }; }
    Rectangle<float> presetRect (int i) const { return { 230.0f + (i % 2) * 333.0f, 60.0f + (i / 2) * 58.0f, 325, 50 }; }
    Rectangle<float> closeRect() const        { return { (float) getWidth() - 50.0f, 16, 30, 30 }; }

    int genre = 0, current = 0, hoverIdx = -1;
};

//==============================================================================
// Rotary knob bound to one parameter: Kicker's glossy black knob, light ring and mono value readout.
class KKnob : public Component
{
public:
    KKnob (RangedAudioParameter& p, String title, float diameter, bool sequencerPalette = false)
        : param (p), name (std::move (title)), size (diameter), seqPal (sequencerPalette),
          attach (p, [this] (float) { repaint(); }, nullptr)
    {
        attach.sendInitialUpdate();
        setWantsKeyboardFocus (true);
        setMouseCursor (MouseCursor::UpDownResizeCursor);
        setTitle (name);
    }

    // centre x and top y of the whole knob (caption, dial, value)
    void placeTop (int cx, int top) { setBounds (cx - ((int) size + 40) / 2, top, (int) size + 40, (int) size + 34); }

    void paint (Graphics& g) override
    {
        const int oldMode = rampMode();
        rampMode() = seqPal ? 1 : 0;
        const float w = (float) getWidth();
        g.setFont (label (16.0f, 0.1f));
        glowText (g, name, Rectangle<float> (0, 0, w, 17), Justification::centred, seqPal ? Colour (0xffc2b4ff) : col::ink, 0.22f);
        drawLitKnob (g, { w * 0.5f, 18.0f + size * 0.5f }, size, param.getValue(), size >= 84.0f);
        const Colour valueCol = ramp (param.getValue()).brighter (0.4f);
        g.setColour (dragging ? valueCol : (seqPal ? Colour (0xff9a8ed8) : Colour (0xffb98c62))); g.setFont (mono (12.5f));
        g.drawText (param.getText (param.getValue(), 14), Rectangle<float> (0, 18.0f + size, w, 15), Justification::centred, false);
        if (hasKeyboardFocus (false))
        {
            g.setColour (ramp (0.5f).withAlpha (0.6f));
            g.drawEllipse (Rectangle<float> (size * 0.66f, size * 0.66f).withCentre ({ w * 0.5f, 18.0f + size * 0.5f }), 1.5f);
        }
        rampMode() = oldMode;
    }

    void mouseDown (const MouseEvent& e) override
    {
        grabKeyboardFocus();
        attach.beginGesture();
        dragging = true;
        startY = e.getScreenY();
        startN = param.getValue();
        repaint();
    }
    void mouseDrag (const MouseEvent& e) override
    {
        const float n = startN + (float) (startY - e.getScreenY()) / (e.mods.isShiftDown() ? 900.0f : 220.0f);
        attach.setValueAsPartOfGesture (param.convertFrom0to1 (jlimit (0.0f, 1.0f, n)));
    }
    void mouseUp (const MouseEvent&) override { attach.endGesture(); dragging = false; repaint(); }
    void mouseDoubleClick (const MouseEvent&) override { attach.setValueAsCompleteGesture (param.convertFrom0to1 (param.getDefaultValue())); }
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& w) override
    {
        if (w.deltaY == 0.0f) return;
        nudge ((w.deltaY > 0 ? 1.0f : -1.0f) * (e.mods.isShiftDown() ? 0.005f : 0.025f));
    }
    bool keyPressed (const KeyPress& k) override
    {
        const float st = k.getModifiers().isShiftDown() ? 0.005f : 0.02f;
        if (k.getKeyCode() == KeyPress::upKey || k.getKeyCode() == KeyPress::rightKey) { nudge (st); return true; }
        if (k.getKeyCode() == KeyPress::downKey || k.getKeyCode() == KeyPress::leftKey) { nudge (-st); return true; }
        return false;
    }
    void focusGained (FocusChangeType) override { repaint(); }
    void focusLost (FocusChangeType) override { repaint(); }
    bool hitTest (int x, int) override { return std::abs ((float) x - (float) getWidth() * 0.5f) <= size * 0.5f + 10.0f; }

private:
    void nudge (float d) { attach.setValueAsCompleteGesture (param.convertFrom0to1 (jlimit (0.0f, 1.0f, param.getValue() + d))); }

    RangedAudioParameter& param;
    String name;
    float size;
    bool seqPal;
    ParameterAttachment attach;
    bool dragging = false;
    int startY = 0;
    float startN = 0.0f;
};

//==============================================================================
// Inset LED window that opens a pop-up list: used for every choice parameter and the pattern picker.
class PickWindow : public Button
{
public:
    PickWindow() : Button ("pick") {}

    std::function<StringArray()> getItems;
    std::function<int()> getCurrent;
    std::function<void (int)> onPickItem;
    bool seqStyle = false;

    void paintButton (Graphics& g, bool over, bool) override
    {
        const auto r = getLocalBounds().toFloat().reduced (1.0f);
        insetWindow (g, r);
        if (over) { g.setColour (Colours::white.withAlpha (0.03f)); g.fillRoundedRectangle (r, 3); }
        const auto items = getItems ? getItems() : StringArray();
        const int cur = getCurrent ? getCurrent() : 0;
        const String t = cur >= 0 && cur < items.size() ? items[cur].toUpperCase() : String();
        g.setFont (mono (15.0f));
        glowText (g, t, r.withTrimmedLeft (10).withTrimmedRight (22), Justification::centredLeft, seqStyle ? Colour (0xffc2b4ff) : col::yellow, 0.25f);
        Path tri;
        const float ax = r.getRight() - 12.0f, ay = r.getCentreY();
        tri.addTriangle (ax - 4.5f, ay - 2.5f, ax + 4.5f, ay - 2.5f, ax, ay + 3.5f);
        g.setColour (seqStyle ? Colour (0xff8a2fe0) : col::amber);
        g.fillPath (tri);
    }

    void clicked() override
    {
        const auto items = getItems ? getItems() : StringArray();
        PopupMenu m;
        const int cur = getCurrent ? getCurrent() : -1;
        for (int i = 0; i < items.size(); ++i)
            m.addItem (i + 1, items[i].toUpperCase(), true, i == cur);
        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this).withMinimumWidth (getWidth()),
                         [this, safe = SafePointer<PickWindow> (this)] (int result)
                         {
                             if (safe != nullptr && result > 0 && onPickItem) onPickItem (result - 1);
                         });
    }
};

// PickWindow bound to a choice parameter
class ChoiceWindow : public PickWindow
{
public:
    ChoiceWindow (RangedAudioParameter& p, StringArray names, String title)
        : param (p), items (std::move (names)),
          attach (p, [this] (float v) { index = jlimit (0, items.size() - 1, (int) std::lround (v)); repaint(); }, nullptr)
    {
        setTitle (title);
        getItems = [this] { return items; };
        getCurrent = [this] { return index; };
        onPickItem = [this] (int i) { attach.setValueAsCompleteGesture ((float) i); };
        attach.sendInitialUpdate();
    }

private:
    RangedAudioParameter& param;
    StringArray items;
    ParameterAttachment attach;
    int index = 0;
};

//==============================================================================
// Section on/off LED, bound to a bool parameter
class PowerLed : public Component
{
public:
    explicit PowerLed (RangedAudioParameter& p, String title)
        : attach (p, [this] (float v) { on = v > 0.5f; repaint(); if (onChange) onChange(); }, nullptr)
    {
        attach.sendInitialUpdate();
        setTitle (title);
        setMouseCursor (MouseCursor::PointingHandCursor);
    }

    bool isOn() const noexcept { return on; }
    std::function<void()> onChange;

    void paint (Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat().withSizeKeepingCentre (13.0f, 13.0f);
        drawLed (g, r, on);
        if (hover) { g.setColour (col::amber.withAlpha (0.6f)); g.drawEllipse (r.expanded (3.0f), 1.0f); }
    }
    void mouseEnter (const MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const MouseEvent&) override { hover = false; repaint(); }
    void mouseDown (const MouseEvent&) override { attach.setValueAsCompleteGesture (on ? 0.0f : 1.0f); }

private:
    ParameterAttachment attach;
    bool on = true, hover = false;
};

//==============================================================================
// Large section switch: LED plus "ON / OFF" caption with the active word lit.
class PowerToggle : public Component
{
public:
    PowerToggle (RangedAudioParameter& p, String title, bool sequencerStyle = false)
        : seqStyle (sequencerStyle), attach (p, [this] (float v) { on = v > 0.5f; repaint(); if (onChange) onChange(); }, nullptr)
    {
        attach.sendInitialUpdate();
        setTitle (title);
        setMouseCursor (MouseCursor::PointingHandCursor);
    }

    bool isOn() const noexcept { return on; }
    std::function<void()> onChange;

    void paint (Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();
        g.setColour (Colour (0xff0b0908)); g.fillRoundedRectangle (b.reduced (1.0f), 6.0f);
        g.setColour ((seqStyle ? Colour (0xff6a4cff) : col::line).withAlpha (hover ? 0.8f : 0.4f)); g.drawRoundedRectangle (b.reduced (1.5f), 6.0f, 1.0f);
        drawLed (g, Rectangle<float> (16.0f, 16.0f).withCentre ({ 17.0f, b.getCentreY() }), on);
        const Colour lit = seqStyle ? Colour (0xffc2b4ff) : col::yellow, dim (0xff6e5238);
        g.setFont (label (16.0f, 0.12f, true));
        auto word = [&] (const String& s, Rectangle<float> r, bool active)
        {
            if (active) glowText (g, s, r, Justification::centredLeft, lit, 0.3f);
            else { g.setColour (dim); g.drawText (s, r, Justification::centredLeft, false); }
        };
        word ("ON", { 34.0f, 0.0f, 26.0f, b.getHeight() }, on);
        g.setColour (dim); g.drawText ("/", Rectangle<float> (57.0f, 0.0f, 10.0f, b.getHeight()), Justification::centred, false);
        word ("OFF", { 68.0f, 0.0f, 32.0f, b.getHeight() }, ! on);
    }
    void mouseEnter (const MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const MouseEvent&) override { hover = false; repaint(); }
    void mouseDown (const MouseEvent&) override { attach.setValueAsCompleteGesture (on ? 0.0f : 1.0f); }

private:
    bool seqStyle;
    ParameterAttachment attach;
    bool on = true, hover = false;
};
}
