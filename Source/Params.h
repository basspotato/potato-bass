#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include "Wavetables.h"

namespace ID
{
    inline constexpr const char* osc1Table = "osc1_table";
    inline constexpr const char* osc1Oct   = "osc1_oct";
    inline constexpr const char* osc1Fine  = "osc1_fine";
    inline constexpr const char* osc1Level = "osc1_level";
    inline constexpr const char* osc2Table = "osc2_table";
    inline constexpr const char* osc2Oct   = "osc2_oct";
    inline constexpr const char* osc2Fine  = "osc2_fine";
    inline constexpr const char* osc2Level = "osc2_level";
    inline constexpr const char* attack    = "amp_attack";
    inline constexpr const char* decay     = "amp_decay";
    inline constexpr const char* sustain   = "amp_sustain";
    inline constexpr const char* release   = "amp_release";
    inline constexpr const char* fType     = "filt_type";
    inline constexpr const char* fCutoff   = "filt_cutoff";
    inline constexpr const char* fRes      = "filt_res";
    inline constexpr const char* fVel      = "filt_vel";
    inline constexpr const char* fMix      = "filt_mix";
    inline constexpr const char* dType     = "dist_type";
    inline constexpr const char* dDrive    = "dist_drive";
    inline constexpr const char* dGain     = "dist_gain";
    inline constexpr const char* dRelease  = "dist_release";
    inline constexpr const char* cThresh   = "comp_thresh";
    inline constexpr const char* cRatio    = "comp_ratio";
    inline constexpr const char* cAttack   = "comp_attack";
    inline constexpr const char* cRelease  = "comp_release";
    inline constexpr const char* cKnee     = "comp_knee";
    inline constexpr const char* cMakeup   = "comp_makeup";
    inline constexpr const char* cMix      = "comp_mix";
    inline constexpr const char* o1On     = "osc1_on";
    inline constexpr const char* o2On     = "osc2_on";
    inline constexpr const char* eOn      = "env_on";
    inline constexpr const char* fOn      = "filt_on";
    inline constexpr const char* dOn      = "dist_on";
    inline constexpr const char* cOn      = "comp_on";
    inline constexpr const char* lOn      = "lfo_on";
    inline constexpr const char* master    = "master_gain";
    inline constexpr const char* stereo    = "stereo";
    inline constexpr const char* lShape    = "lfo_shape";
    inline constexpr const char* lRate     = "lfo_rate";
    inline constexpr const char* lTarget   = "lfo_target";
    inline constexpr const char* lDepth    = "lfo_depth";
    inline constexpr const char* lCurve    = "lfo_curve";
    inline constexpr const char* sOn       = "seq_on";
    inline constexpr const char* sRate     = "seq_rate";
    inline constexpr const char* sGate     = "seq_gate";
    inline constexpr const char* sDir      = "seq_dir";
}

// Filter types: LP12, LP24, HP12, HP24, BP, Notch
inline const juce::StringArray filterTypeNames { "LP 12", "LP 24", "HP 12", "HP 24", "BP", "Notch" };

// mode: 0 LP, 1 HP, 2 BP, 3 Notch
inline int filterMode(int type) { constexpr int m[] { 0, 0, 1, 1, 2, 3 }; return m[juce::jlimit(0, 5, type)]; }
inline bool filterTwoStage(int type) { return type == 1 || type == 3; }
inline float resToK(float r) { return 2.f * std::pow(0.15f, r); }

inline const juce::StringArray distTypeNames { "Off", "Tube", "Tape", "Soft Clip", "Hard Clip", "Diode", "Fold" };

inline float shape(int type, float x)
{
    switch (type)
    {
        case 1:  return std::tanh(x + 0.25f) - 0.24491866f;
        case 2:  return std::atan(x * 1.5f) / 1.5f;
        case 3:  return x / (1.f + std::abs(x));
        case 4:  return juce::jlimit(-1.f, 1.f, x);
        case 5:  return x >= 0.f ? 1.f - std::exp(-x) : -0.6f * (1.f - std::exp(x / 0.6f));
        case 6:  return std::sin(x);
        default: return x;
    }
}

inline float driveGain(float drive) { return juce::Decibels::decibelsToGain(drive * 36.f); }

// Transparent below ~0.5, bounded to +/-1 above 3.
inline float softClip(float x)
{
    x = juce::jlimit(-3.f, 3.f, x);
    return x * (27.f + x * x) / (27.f + 9.f * x * x);
}

// Static compressor transfer curve: input dB -> output dB (soft knee).
inline float compCurve(float inDb, float thr, float ratio, float knee)
{
    const float over = inDb - thr;
    if (2.f * over < -knee) return inDb;
    if (knee > 0.f && 2.f * std::abs(over) <= knee)
    {
        const float t = over + knee * 0.5f;
        return inDb + (1.f / ratio - 1.f) * t * t / (2.f * knee);
    }
    return thr + over / ratio;
}

inline const juce::StringArray lfoShapeNames { "Duck", "Sine", "Triangle", "Saw Down", "Square" };
inline const juce::StringArray lfoTargetNames { "Volume", "Cutoff" };
inline const juce::StringArray syncNames { "2 Bars", "1 Bar", "1/2", "1/4", "1/4D", "1/4T", "1/8", "1/8D", "1/8T", "1/16", "1/16D", "1/16T", "1/32" };
inline constexpr double syncBeats[] { 8.0, 4.0, 2.0, 1.0, 1.5, 2.0 / 3.0, 0.5, 0.75, 1.0 / 3.0, 0.25, 0.375, 1.0 / 6.0, 0.125 };
inline const juce::StringArray seqDirNames { "Up", "Down", "Up/Down", "Random" };
inline const juce::StringArray seqRateNames { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" };
inline constexpr double seqBeats[] { 1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125 };

inline float lfoCurveExp(float curve) { return std::pow(4.f, 2.f * curve - 1.f); }

// LFO output 0..1 for phase 0..1, bent by expo (1 = unbent).
inline float lfoShape(int shape, float p, float expo)
{
    float v;
    switch (shape)
    {
        case 1:  v = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * p); break;
        case 2:  v = p < 0.5f ? 2.f * p : 2.f - 2.f * p; break;
        case 3:  v = 1.f - p; break;
        case 4:  v = p < 0.5f ? 0.f : 1.f; break;
        default: v = p; break;
    }
    return expo == 1.f ? v : std::pow(v, expo);
}

inline juce::String stepId(const char* prefix, int i) { return juce::String(prefix) + "_" + juce::String(i + 1); }

// Shared from processor to voices: LFO samples for the current block when the LFO targets filter cutoff.
struct ModBus
{
    const float* lfo = nullptr;
    float cutOct = 0.f;
};

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto addOsc = [&layout](const char* tbl, const char* oct, const char* fine, const char* lvl,
                            const String& n, int defTable, int defOct, float defLevel)
    {
        layout.add(std::make_unique<AudioParameterChoice>(ParameterID { tbl, 1 }, n + " Wave", Wavetables::names(), defTable));
        layout.add(std::make_unique<AudioParameterInt>(ParameterID { oct, 1 }, n + " Octave", -2, 2, defOct));
        layout.add(std::make_unique<AudioParameterFloat>(ParameterID { fine, 1 }, n + " Fine", NormalisableRange<float>(-100.f, 100.f, 0.1f), 0.f));
        layout.add(std::make_unique<AudioParameterFloat>(ParameterID { lvl, 1 }, n + " Level", NormalisableRange<float>(0.f, 1.f, 0.001f), defLevel));
    };
    addOsc(ID::osc1Table, ID::osc1Oct, ID::osc1Fine, ID::osc1Level, "Osc 1", 2, 0, 0.8f);
    addOsc(ID::osc2Table, ID::osc2Oct, ID::osc2Fine, ID::osc2Level, "Osc 2", 0, -1, 0.5f);

    auto timeStr = [](float v, int) { return v < 1.f ? String(roundToInt(v * 1000.f)) + " ms" : String(v, 2) + " s"; };
    auto timeRange = [](float centre) { NormalisableRange<float> r(0.001f, 5.f); r.setSkewForCentre(centre); return r; };
    auto timeAttr = [&]() { return AudioParameterFloatAttributes().withStringFromValueFunction(timeStr); };

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::attack, 1 },  "Attack",  timeRange(0.25f), 0.005f, timeAttr()));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::decay, 1 },   "Decay",   timeRange(0.25f), 0.25f,  timeAttr()));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::sustain, 1 }, "Sustain", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.7f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::release, 1 }, "Release", timeRange(0.25f), 0.15f,  timeAttr()));

    NormalisableRange<float> cutoffRange(20.f, 20000.f);
    cutoffRange.setSkewForCentre(1000.f);
    auto hzStr = [](float v, int) { return v >= 1000.f ? String(v / 1000.f, 2) + " kHz" : String(roundToInt(v)) + " Hz"; };

    layout.add(std::make_unique<AudioParameterChoice>(ParameterID { ID::fType, 1 }, "Filter Type", filterTypeNames, 1));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::fCutoff, 1 }, "Cutoff", cutoffRange, 1200.f,
                                                     AudioParameterFloatAttributes().withStringFromValueFunction(hzStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::fRes, 1 }, "Resonance", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.2f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::fVel, 1 }, "Velocity", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.3f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::fMix, 1 }, "Filter Mix", NormalisableRange<float>(0.f, 1.f, 0.001f), 1.f));

    auto msStr = [](float v, int) { return v < 100.f ? String(v, 1) + " ms" : String(roundToInt(v)) + " ms"; };
    auto dbStr = [](float v, int) { return String(v, 1) + " dB"; };
    auto ratioStr = [](float v, int) { return String(v, 1) + ":1"; };
    auto skewed = [](float lo, float hi, float centre) { NormalisableRange<float> r(lo, hi); r.setSkewForCentre(centre); return r; };
    auto fAttr = [](auto fn) { return AudioParameterFloatAttributes().withStringFromValueFunction(fn); };

    layout.add(std::make_unique<AudioParameterChoice>(ParameterID { ID::dType, 1 }, "Dist Type", distTypeNames, 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::dDrive, 1 }, "Dist Drive", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.3f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::dGain, 1 }, "Dist Gain", NormalisableRange<float>(-24.f, 12.f, 0.1f), 0.f, fAttr(dbStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::dRelease, 1 }, "Dist Release", skewed(10.f, 1000.f, 150.f), 150.f, fAttr(msStr)));

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::cThresh, 1 }, "Comp Threshold", NormalisableRange<float>(-60.f, 0.f, 0.1f), -12.f, fAttr(dbStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::cRatio, 1 }, "Comp Ratio", skewed(1.f, 20.f, 4.f), 4.f, fAttr(ratioStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::cAttack, 1 }, "Comp Attack", skewed(0.1f, 100.f, 10.f), 10.f, fAttr(msStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::cRelease, 1 }, "Comp Release", skewed(10.f, 1000.f, 100.f), 100.f, fAttr(msStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::cKnee, 1 }, "Comp Knee", NormalisableRange<float>(0.f, 24.f, 0.1f), 6.f, fAttr(dbStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::cMakeup, 1 }, "Comp Makeup", NormalisableRange<float>(0.f, 24.f, 0.1f), 0.f, fAttr(dbStr)));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::cMix, 1 }, "Comp Mix", NormalisableRange<float>(0.f, 1.f, 0.001f), 1.f));

    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::o1On, 1 }, "Osc1 On", true));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::o2On, 1 }, "Osc2 On", true));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::eOn, 1 }, "Env On", true));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::fOn, 1 }, "Filter On", true));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::dOn, 1 }, "Dist On", true));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::cOn, 1 }, "Comp On", true));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::lOn, 1 }, "LFO On", true));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::master, 1 }, "Master", skewed(-60.f, 6.f, -14.f), 0.f,
                                                     fAttr([](float v, int) { return v <= -59.9f ? String("-inf") : String(v, 1) + " dB"; })));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::stereo, 1 }, "Stereo Output", false,
                                                    AudioParameterBoolAttributes().withStringFromValueFunction([](bool b, int) { return b ? "Stereo" : "Mono"; })));

    layout.add(std::make_unique<AudioParameterChoice>(ParameterID { ID::lShape, 1 }, "LFO Shape", lfoShapeNames, 0));
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID { ID::lRate, 1 }, "LFO Rate", syncNames, 3));
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID { ID::lTarget, 1 }, "LFO Target", lfoTargetNames, 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::lDepth, 1 }, "LFO Depth", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::lCurve, 1 }, "LFO Curve", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.6f));

    layout.add(std::make_unique<AudioParameterBool>(ParameterID { ID::sOn, 1 }, "Seq On", false));
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID { ID::sRate, 1 }, "Seq Rate", seqRateNames, 3));
    layout.add(std::make_unique<AudioParameterChoice>(ParameterID { ID::sDir, 1 }, "Seq Direction", seqDirNames, 0));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID { ID::sGate, 1 }, "Seq Gate", NormalisableRange<float>(0.05f, 1.f, 0.01f), 0.6f,
                                                     AudioParameterFloatAttributes().withStringFromValueFunction([](float v, int) { return String(roundToInt(v * 100.f)) + "%"; })));

    const bool defOn[16] { 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0 };
    const int defPit[16] { 0, 0, 12, 0, 0, 7, 0, 3, 0, 0, 12, 0, 0, 5, 3, 0 };
    for (int i = 0; i < 16; ++i)
    {
        const String n = "Step " + String(i + 1);
        layout.add(std::make_unique<AudioParameterBool>(ParameterID { stepId("seq_on", i), 1 }, n + " On", defOn[i]));
        layout.add(std::make_unique<AudioParameterInt>(ParameterID { stepId("seq_pit", i), 1 }, n + " Pitch", -12, 12, defPit[i]));
        layout.add(std::make_unique<AudioParameterFloat>(ParameterID { stepId("seq_vel", i), 1 }, n + " Velocity", NormalisableRange<float>(0.f, 1.f, 0.01f), i == 0 ? 1.f : 0.8f));
    }

    return layout;
}
