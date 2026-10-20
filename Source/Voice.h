#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include "Params.h"
#include "Wavetables.h"

struct BassSound : juce::SynthesiserSound
{
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

// Zavalishin TPT state-variable filter, one 12 dB stage.
class StateVariable
{
public:
    void reset() { ic1 = ic2 = 0.f; }

    void setCoefficients(float g, float kIn)
    {
        k = kIn;
        a1 = 1.f / (1.f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    // mode: 0 LP, 1 HP, 2 BP (unity peak), 3 Notch
    float process(float v0, int mode) noexcept
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.f * v1 - ic1;
        ic2 = 2.f * v2 - ic2;

        switch (mode)
        {
            case 0:  return v2;
            case 1:  return v0 - k * v1 - v2;
            case 2:  return k * v1;
            default: return v0 - k * v1;
        }
    }

private:
    float ic1 = 0.f, ic2 = 0.f, k = 1.f, a1 = 0.f, a2 = 0.f, a3 = 0.f;
};

// Parameters are smoothed at control rate (every kTick samples); audio-rate work is just two table reads,
// one or two filter stages and the envelope.
class BassVoice : public juce::SynthesiserVoice
{
public:
    BassVoice(juce::AudioProcessorValueTreeState& s, const Wavetables& w, const ModBus& m) : wt(w), bus(m)
    {
        osc1Table = s.getRawParameterValue(ID::osc1Table);
        osc1Oct   = s.getRawParameterValue(ID::osc1Oct);
        osc1Fine  = s.getRawParameterValue(ID::osc1Fine);
        osc1Level = s.getRawParameterValue(ID::osc1Level);
        osc2Table = s.getRawParameterValue(ID::osc2Table);
        osc2Oct   = s.getRawParameterValue(ID::osc2Oct);
        osc2Fine  = s.getRawParameterValue(ID::osc2Fine);
        osc2Level = s.getRawParameterValue(ID::osc2Level);
        attack    = s.getRawParameterValue(ID::attack);
        decay     = s.getRawParameterValue(ID::decay);
        sustain   = s.getRawParameterValue(ID::sustain);
        release   = s.getRawParameterValue(ID::release);
        fType     = s.getRawParameterValue(ID::fType);
        fCutoff   = s.getRawParameterValue(ID::fCutoff);
        fRes      = s.getRawParameterValue(ID::fRes);
        fVel      = s.getRawParameterValue(ID::fVel);
        fMix      = s.getRawParameterValue(ID::fMix);
        o1On = s.getRawParameterValue(ID::o1On);
        o2On = s.getRawParameterValue(ID::o2On);
        eOn = s.getRawParameterValue(ID::eOn);
        fOn = s.getRawParameterValue(ID::fOn);
    }

    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<BassSound*>(s) != nullptr; }

    void setCurrentPlaybackSampleRate(double sr) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate(sr);
        adsr.setSampleRate(sr);
        tickCoef = 1.f - std::exp(-(float) kTick / (0.005f * (float) sr));
    }

    // A voice that is already sounding (stolen or retriggered) keeps its phase, filter and envelope state,
    // so the new note continues from the current level instead of clicking.
    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override
    {
        baseFreq = (float) juce::MidiMessage::getMidiNoteInHertz(midiNote);
        vel = velocity;

        if (!adsr.isActive())
        {
            phase1 = phase2 = 0.f;
            filt1.reset();
            filt2.reset();
            snap = true;
        }

        setEnv();
        adsr.noteOn();
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (allowTailOff)
            adsr.noteOff();
        else
            clearCurrentNote();
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (!isVoiceActive())
        {
            if (adsr.isActive())
                adsr.reset();
            return;
        }

        const float sr = (float) getSampleRate();
        setEnv();

        const int t1 = (int) osc1Table->load(), t2 = (int) osc2Table->load();
        const float tL1 = o1On->load() > 0.5f ? osc1Level->load() : 0.f, tL2 = o2On->load() > 0.5f ? osc2Level->load() : 0.f;
        const float tInc1 = baseFreq * std::exp2(osc1Oct->load() + osc1Fine->load() / 1200.f) / sr;
        const float tInc2 = baseFreq * std::exp2(osc2Oct->load() + osc2Fine->load() / 1200.f) / sr;

        const int type = (int) fType->load();
        const int mode = filterMode(type);
        const bool twoStage = filterTwoStage(type);
        const float tRes = fRes->load();
        const float tMix = fOn->load() > 0.5f ? fMix->load() : 0.f;
        const float velOct = fVel->load() * (vel * 2.f - 1.f) * 3.f;
        const float minCut = std::log2(20.f), maxCut = std::log2(std::min(20000.f, 0.45f * sr));
        const float tCut = juce::jlimit(minCut, maxCut, std::log2(fCutoff->load()) + velOct);

        if (snap)
        {
            sL1 = tL1; sL2 = tL2; sInc1 = tInc1; sInc2 = tInc2; sRes = tRes; sMix = tMix; sCut = tCut;
            snap = false;
        }

        float* L = out.getWritePointer(0, start);
        float* R = out.getNumChannels() > 1 ? out.getWritePointer(1, start) : nullptr;
        const float pi = juce::MathConstants<float>::pi;

        for (int i = 0; i < num; i += kTick)
        {
            const int n = std::min(kTick, num - i);

            sL1 += tickCoef * (tL1 - sL1);
            sL2 += tickCoef * (tL2 - sL2);
            sInc1 += tickCoef * (tInc1 - sInc1);
            sInc2 += tickCoef * (tInc2 - sInc2);
            sRes += tickCoef * (tRes - sRes);
            sMix += tickCoef * (tMix - sMix);
            sCut += tickCoef * (tCut - sCut);

            const float lfoOct = bus.lfo != nullptr ? (bus.lfo[start + i] - 1.f) * bus.cutOct : 0.f;
            const float g = std::tan(pi * std::exp2(juce::jlimit(minCut, maxCut, sCut + lfoOct)) / sr);
            const float k = resToK(sRes);
            filt1.setCoefficients(g, k);
            filt2.setCoefficients(g, k);

            const int lev1 = Wavetables::levelFor(sInc1), lev2 = Wavetables::levelFor(sInc2);
            const bool useOsc2 = sL2 > 1.0e-4f;

            for (int j = 0; j < n; ++j)
            {
                float dry = wt.read(t1, lev1, phase1) * sL1;
                if (useOsc2)
                    dry += wt.read(t2, lev2, phase2) * sL2;

                phase1 += sInc1; if (phase1 >= 1.f) phase1 -= 1.f;
                phase2 += sInc2; if (phase2 >= 1.f) phase2 -= 1.f;

                float wet = filt1.process(dry, mode);
                if (twoStage)
                    wet = filt2.process(wet, mode);

                const float o = (dry + sMix * (wet - dry)) * adsr.getNextSample() * vel * 0.35f;
                L[i + j] += o;
                if (R != nullptr)
                    R[i + j] += o;
            }

            if (!adsr.isActive())
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    static constexpr int kTick = 16;

    void setEnv()
    {
        if (eOn->load() > 0.5f)
            adsr.setParameters({ attack->load(), decay->load(), sustain->load(), release->load() });
        else
            adsr.setParameters({ 0.002f, 0.f, 1.f, 0.01f });
    }

    const Wavetables& wt;
    const ModBus& bus;
    juce::ADSR adsr;
    StateVariable filt1, filt2;

    std::atomic<float> *osc1Table = nullptr, *osc1Oct = nullptr, *osc1Fine = nullptr, *osc1Level = nullptr;
    std::atomic<float> *osc2Table = nullptr, *osc2Oct = nullptr, *osc2Fine = nullptr, *osc2Level = nullptr;
    std::atomic<float> *attack = nullptr, *decay = nullptr, *sustain = nullptr, *release = nullptr;
    std::atomic<float> *o1On = nullptr, *o2On = nullptr, *eOn = nullptr, *fOn = nullptr, *fType = nullptr, *fCutoff = nullptr, *fRes = nullptr, *fVel = nullptr, *fMix = nullptr;

    float baseFreq = 0.f, vel = 0.f, phase1 = 0.f, phase2 = 0.f, tickCoef = 0.1f;
    float sL1 = 0.f, sL2 = 0.f, sInc1 = 0.f, sInc2 = 0.f, sRes = 0.f, sMix = 1.f, sCut = 10.f;
    bool snap = true;
};
