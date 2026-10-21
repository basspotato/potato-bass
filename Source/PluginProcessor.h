#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>
#include "Params.h"
#include "Presets.h"
#include "SeqPatterns.h"
#include "Wavetables.h"

class PotatoBassProcessor : public juce::AudioProcessor
{
public:
    PotatoBassProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Potato Bass"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    Wavetables wavetables;
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeter { 0.f };
    std::atomic<int> seqStep { -1 };

    void loadPreset(int index, bool asUserGesture = true);
    bool presetIsModified() const;
    void loadSeqPattern(int index);
    std::atomic<int> currentPattern { 1 };
    juce::MidiKeyboardState keyboardState;
    std::atomic<int> currentPreset { 1 };
    std::atomic<float> meterRms { 0.f };
    static constexpr int scopeSize = 8192;
    int getScopeWritePos() const { return scopeWrite.load(std::memory_order_acquire); }
    void copyScope(float* dst, int n, int pos) const
    {
        for (int i = 0; i < n; ++i)
            dst[i] = scopeBuf[(size_t) ((pos - n + i) & (scopeSize - 1))];
    }
    std::atomic<int> scopeMode { 0 };
    std::atomic<int> heldNotes { 0 };
    std::atomic<unsigned> noteOnCounter { 0 };

private:
    void applyEffects(juce::AudioBuffer<float>&);
    void processChunk(float* L, float* R, int n);
    void runSequencer(juce::MidiBuffer& midi, int n, double p0, double bpb);

    ModBus bus;
    juce::Synthesiser synth;
    juce::MidiBuffer seqMidi;
    std::vector<float> lfoBuf;
    std::atomic<float> *lShape = nullptr, *lRate = nullptr, *lTarget = nullptr, *lDepth = nullptr, *lCurve = nullptr;
    std::atomic<float> *master = nullptr, *dOn = nullptr, *cOn = nullptr, *lOn = nullptr, *sOn = nullptr, *sRate = nullptr, *sGate = nullptr, *sDir = nullptr;
    std::atomic<float> *stOn[16] {}, *stPit[16] {}, *stVel[16] {};
    int held[128] {}, heldCount = 0, seqNote = -1;
    long long lastAbsStep = -1;
    double freePpq = 0.0;
    std::array<float, scopeSize> scopeBuf {};
    std::atomic<int> scopeWrite { 0 };
    float lfoState = 1.f;
    bool lfoPrimed = false;
    juce::dsp::Oversampling<float> oversampling { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };
    juce::SmoothedValue<float> driveSm, outSm, makeupSm, mixSm, masterSm;
    std::vector<float> dgBuf, ogBuf;

    std::atomic<float> *dType = nullptr, *dDrive = nullptr, *dGain = nullptr, *dRelease = nullptr;
    std::atomic<float> *cThresh = nullptr, *cRatio = nullptr, *cAttack = nullptr, *cRelease = nullptr;
    std::atomic<float> *cKnee = nullptr, *cMakeup = nullptr, *cMix = nullptr;

    double sr = 44100.0;
    int maxBlock = 512, lastDt = 0;
    float compGr = 0.f, distEnv = 0.f;
    float dcXL = 0.f, dcYL = 0.f, dcXR = 0.f, dcYR = 0.f, dcCoef = 0.999f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PotatoBassProcessor)
};
