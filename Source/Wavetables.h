#pragma once
#include <juce_core/juce_core.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

// 20 single-cycle wavetables, each stored as 9 band-limited mip levels (384, 192, ... 1 harmonics)
// so no oscillator ever aliases. Read with 4-point cubic interpolation.
class Wavetables
{
public:
    static constexpr int numTables = 20;
    static constexpr int size = 2048;
    static constexpr int maxHarmonics = 384;
    static constexpr int numLevels = 9;

    static const juce::StringArray& names()
    {
        static const juce::StringArray n {
            "Sine", "Triangle", "Saw", "Square", "Pulse 25", "Pulse 10", "Soft Saw",
            "Soft Square", "Sub Heavy", "Fat Bass", "Hollow", "Wow Formant", "Growl",
            "Reese", "Organ", "Bright Saw", "Metallic", "Sync Sweep", "Vocal Ah", "Digital Edge" };
        return n;
    }

    Wavetables() : data((size_t) numTables * numLevels * stride, 0.f)
    {
        for (int t = 0; t < numTables; ++t)
            build(t);
    }

    // inc = phase increment per sample
    static int levelFor(float inc) noexcept
    {
        const float need = 0.45f / std::max(inc, 1.0e-6f);
        for (int k = 0; k < numLevels - 1; ++k)
            if ((float) (maxHarmonics >> k) <= need)
                return k;
        return numLevels - 1;
    }

    float read(int table, int level, float phase) const noexcept
    {
        const float pos = phase * (float) size;
        const int i = juce::jlimit(0, size - 1, (int) pos);
        const float f = pos - (float) i;
        const float* p = &data[offset(table, level) + (size_t) i];
        const float y0 = p[0], y1 = p[1], y2 = p[2], y3 = p[3];
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + y1;
    }

    float readDisplay(int table, float phase) const noexcept { return read(table, 0, phase); }

private:
    static constexpr size_t stride = (size_t) size + 3;
    static size_t offset(int table, int level) noexcept { return ((size_t) table * numLevels + (size_t) level) * stride; }
    static float sq(float x) { return x * x; }

    static float amplitude(int t, int n, float& phase)
    {
        const float fn = (float) n;
        const bool odd = (n & 1) != 0;
        const float pi = juce::MathConstants<float>::pi;
        phase = 0.f;

        switch (t)
        {
            case 0:  return n == 1 ? 1.f : 0.f;
            case 1:  return odd ? ((((n - 1) / 2) % 2 == 0) ? 1.f : -1.f) / (fn * fn) : 0.f;
            case 2:  return 1.f / fn;
            case 3:  return odd ? 1.f / fn : 0.f;
            case 4:  phase = pi * 0.5f - fn * pi * 0.25f; return std::sin(fn * pi * 0.25f) / fn;
            case 5:  phase = pi * 0.5f - fn * pi * 0.10f; return std::sin(fn * pi * 0.10f) / fn;
            case 6:  return 1.f / std::pow(fn, 1.5f);
            case 7:  return odd ? 1.f / std::pow(fn, 1.5f) : 0.f;
            case 8:  return n == 1 ? 1.f : n == 2 ? 0.45f : n == 3 ? 0.2f : n == 4 ? 0.08f : 0.f;
            case 9:  return std::pow(0.8f, fn - 1.f);
            case 10: return odd ? std::exp(-fn / 18.f) / std::sqrt(fn) : 0.f;
            case 11: return (1.f / fn) * (0.3f + 2.5f * std::exp(-sq(fn - 3.f) / 4.f) + 1.5f * std::exp(-sq(fn - 9.f) / 8.f));
            case 12: return (odd ? -1.f : 1.f) * (1.f / fn) * (0.5f + 3.f * std::exp(-sq(fn - 7.f) / 20.f));
            case 13: return (1.f / fn) * (0.55f + 0.45f * std::cos(fn * 0.9f));
            case 14:
            {
                switch (n) { case 1: return 1.f; case 2: return 0.8f; case 3: return 0.5f; case 4: return 0.6f;
                             case 6: return 0.3f; case 8: return 0.25f; default: return 0.f; }
            }
            case 15: return 1.f / std::pow(fn, 0.75f);
            case 16: return n == 1 ? 1.f : (n % 3 == 0 ? 1.f / std::pow(fn, 0.7f) : 0.12f / fn);
            case 17: return (1.f / fn) * (0.4f + 3.f * std::exp(-sq(fn - 12.f) / 3.f));
            case 18: return (1.f / fn) * (0.2f + 3.f * std::exp(-sq(fn - 6.f) / 4.f)
                                          + 2.f * std::exp(-sq(fn - 15.f) / 8.f)
                                          + 1.f * std::exp(-sq(fn - 24.f) / 12.f));
            case 19: phase = fn * 0.7f; return (1.f / std::pow(fn, 0.9f)) * (odd ? 1.f : 0.6f);
            default: return 0.f;
        }
    }

    // All levels share one normalisation (from level 0) so the fundamental level is identical across mip levels.
    void build(int t)
    {
        std::array<float, maxHarmonics + 1> amps {}, phases {};
        for (int n = 1; n <= maxHarmonics; ++n)
            amps[(size_t) n] = amplitude(t, n, phases[(size_t) n]);

        const double pi = juce::MathConstants<double>::pi;
        std::vector<double> buf((size_t) size);
        double norm = 1.0;

        for (int k = 0; k < numLevels; ++k)
        {
            const int H = maxHarmonics >> k;
            std::fill(buf.begin(), buf.end(), 0.0);

            for (int n = 1; n <= H; ++n)
            {
                if (amps[(size_t) n] == 0.f)
                    continue;

                const double x = pi * (double) n / (double) (H + 1);
                const double a = (double) amps[(size_t) n] * (std::sin(x) / x);
                const double th = 2.0 * pi * (double) n / (double) size;
                const double rs = std::sin(th), rc = std::cos(th);
                double s = std::sin((double) phases[(size_t) n]), c = std::cos((double) phases[(size_t) n]);

                for (int i = 0; i < size; ++i)
                {
                    buf[(size_t) i] += a * s;
                    const double ns = s * rc + c * rs;
                    c = c * rc - s * rs;
                    s = ns;
                }
            }

            if (k == 0)
            {
                double peak = 0.0;
                for (double v : buf) peak = std::max(peak, std::abs(v));
                norm = peak > 0.0 ? 1.0 / peak : 1.0;
            }

            float* p = &data[offset(t, k)];
            for (int i = 0; i < size; ++i)
                p[i + 1] = (float) (buf[(size_t) i] * norm);
            p[0] = p[size];
            p[size + 1] = p[1];
            p[size + 2] = p[2];
        }
    }

    std::vector<float> data;
};
