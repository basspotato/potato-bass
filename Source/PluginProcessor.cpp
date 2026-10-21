#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Voice.h"

namespace
{
    // Type is a compile-time constant so the shaper switch folds away in the inner loop.
    template <int Type>
    void shapeBlock(float* uL, float* uR, int m, const float* dg, const float* og)
    {
        for (int j = 0; j < m; ++j)
        {
            const float g = dg[j >> 2], o = og[j >> 2];
            uL[j] = shape(Type, uL[j] * g) * o;
            uR[j] = shape(Type, uR[j] * g) * o;
        }
    }
}

PotatoBassProcessor::PotatoBassProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
    synth.addSound(new BassSound());
    for (int i = 0; i < 8; ++i)
        synth.addVoice(new BassVoice(apvts, wavetables, bus));

    dType = apvts.getRawParameterValue(ID::dType);
    dDrive = apvts.getRawParameterValue(ID::dDrive);
    dGain = apvts.getRawParameterValue(ID::dGain);
    dRelease = apvts.getRawParameterValue(ID::dRelease);
    cThresh = apvts.getRawParameterValue(ID::cThresh);
    cRatio = apvts.getRawParameterValue(ID::cRatio);
    cAttack = apvts.getRawParameterValue(ID::cAttack);
    cRelease = apvts.getRawParameterValue(ID::cRelease);
    cKnee = apvts.getRawParameterValue(ID::cKnee);
    cMakeup = apvts.getRawParameterValue(ID::cMakeup);
    cMix = apvts.getRawParameterValue(ID::cMix);
    lShape = apvts.getRawParameterValue(ID::lShape);
    lRate = apvts.getRawParameterValue(ID::lRate);
    lTarget = apvts.getRawParameterValue(ID::lTarget);
    lDepth = apvts.getRawParameterValue(ID::lDepth);
    lCurve = apvts.getRawParameterValue(ID::lCurve);
    master = apvts.getRawParameterValue(ID::master);
    dOn = apvts.getRawParameterValue(ID::dOn);
    cOn = apvts.getRawParameterValue(ID::cOn);
    lOn = apvts.getRawParameterValue(ID::lOn);
    sOn = apvts.getRawParameterValue(ID::sOn);
    sRate = apvts.getRawParameterValue(ID::sRate);
    sGate = apvts.getRawParameterValue(ID::sGate);
    sDir = apvts.getRawParameterValue(ID::sDir);
    for (int i = 0; i < 16; ++i)
    {
        stOn[i] = apvts.getRawParameterValue(stepId("seq_on", i));
        stPit[i] = apvts.getRawParameterValue(stepId("seq_pit", i));
        stVel[i] = apvts.getRawParameterValue(stepId("seq_vel", i));
    }
    loadPreset(currentPreset.load(), false);
}

void PotatoBassProcessor::loadPreset(int index, bool asUserGesture)
{
    index = juce::jlimit(0, pb::numPresets - 1, index);
    currentPreset = index;
    const auto& p = pb::presets[index];

    auto set = [asUserGesture](juce::RangedAudioParameter* prm, float value)
    {
        if (prm == nullptr) return;
        if (asUserGesture) prm->beginChangeGesture();
        prm->setValueNotifyingHost(prm->convertTo0to1(value));
        if (asUserGesture) prm->endChangeGesture();
    };
    for (int i = 0; i < pb::numParams; ++i)
        set(apvts.getParameter(pb::presetIds[i]), p.v[i]);
    for (auto* id : { ID::o1On, ID::o2On, ID::eOn, ID::fOn, ID::dOn, ID::cOn, ID::lOn })
        set(apvts.getParameter(id), 1.f);
}

void PotatoBassProcessor::loadSeqPattern(int index)
{
    index = juce::jlimit(0, pb::numSeqPatterns - 1, index);
    currentPattern = index;
    const auto& p = pb::seqPatterns[index];
    auto set = [](juce::RangedAudioParameter* prm, float value)
    {
        if (prm == nullptr) return;
        prm->beginChangeGesture();
        prm->setValueNotifyingHost(prm->convertTo0to1(value));
        prm->endChangeGesture();
    };
    for (int i = 0; i < 16; ++i)
    {
        set(apvts.getParameter(stepId("seq_on", i)), p.on[i] ? 1.f : 0.f);
        set(apvts.getParameter(stepId("seq_pit", i)), (float) p.pit[i]);
        set(apvts.getParameter(stepId("seq_vel", i)), p.vel[i]);
    }
}

bool PotatoBassProcessor::presetIsModified() const
{
    const auto& p = pb::presets[currentPreset.load()];
    for (int i = 0; i < pb::numParams; ++i)
        if (auto* prm = apvts.getParameter(pb::presetIds[i]))
            if (std::abs(prm->getValue() - prm->convertTo0to1(p.v[i])) > 0.002f)
                return true;
    return false;
}

void PotatoBassProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    sr = sampleRate;
    maxBlock = juce::jmax(1, samplesPerBlock);

    oversampling.initProcessing((size_t) maxBlock);
    oversampling.reset();
    lfoBuf.assign((size_t) maxBlock, 1.f);
    seqMidi.ensureSize(8192);
    heldCount = 0;
    seqNote = -1;
    lastAbsStep = -1;
    freePpq = 0.0;
    dgBuf.assign((size_t) maxBlock, 0.f);
    ogBuf.assign((size_t) maxBlock, 0.f);

    driveSm.reset(sampleRate, 0.02);  driveSm.setCurrentAndTargetValue(driveGain(dDrive->load()));
    outSm.reset(sampleRate, 0.02);    outSm.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(dGain->load()));
    makeupSm.reset(sampleRate, 0.02); makeupSm.setCurrentAndTargetValue(cMakeup->load());
    masterSm.reset(sampleRate, 0.02); masterSm.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(master->load(), -59.9f));
    mixSm.reset(sampleRate, 0.02);    mixSm.setCurrentAndTargetValue(cMix->load());

    dcCoef = 1.f - juce::MathConstants<float>::twoPi * 10.f / (float) sampleRate;
    compGr = 0.f;
    distEnv = 0.f;
    lastDt = 0;
    dcXL = dcYL = dcXR = dcYR = 0.f;
}

bool PotatoBassProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// Maps a running step count to a pattern step for the chosen direction (0 up, 1 down, 2 up/down, 3 random).
static int stepForCount(long long count, int dir)
{
    const int k = (int) (((count % 16) + 16) % 16);
    if (dir == 1)
        return 15 - k;
    if (dir == 2)
    {
        const int m = (int) (((count % 30) + 30) % 30);
        return m < 16 ? m : 30 - m;
    }
    if (dir == 3)
    {
        auto h = (unsigned long long) count * 0x9E3779B97F4A7C15ULL;
        h ^= h >> 29; h *= 0xBF58476D1CE4E5B9ULL; h ^= h >> 32;
        return (int) (h % 16ULL);
    }
    return k;
}

void PotatoBassProcessor::runSequencer(juce::MidiBuffer& midi, int n, double p0, double bpb)
{
    seqMidi.clear();
    const bool on = sOn->load() > 0.5f;
    for (const auto meta : midi)
        if (meta.getMessage().isNoteOn())
            noteOnCounter.fetch_add(1);

    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())
        {
            const int note = m.getNoteNumber();
            if (heldCount < 128)
                held[heldCount++] = note;
            if (!on)
                seqMidi.addEvent(m, meta.samplePosition);
        }
        else if (m.isNoteOff())
        {
            const int note = m.getNoteNumber();
            for (int k = 0; k < heldCount; ++k)
                if (held[k] == note)
                {
                    for (int j = k; j < heldCount - 1; ++j)
                        held[j] = held[j + 1];
                    --heldCount;
                    break;
                }
            if (!on)
                seqMidi.addEvent(m, meta.samplePosition);
        }
        else if (!(on && m.isAllNotesOff()))
            seqMidi.addEvent(m, meta.samplePosition);
    }

    if (!on)
    {
        if (seqNote >= 0)
            seqMidi.addEvent(juce::MidiMessage::noteOff(1, seqNote), 0);
        seqNote = -1;
        lastAbsStep = -1;
        seqStep.store(-1);
        midi.swapWith(seqMidi);
        return;
    }

    const double stepBeats = seqBeats[(int) sRate->load()];
    const double gate = sGate->load();
    const int dir = (int) sDir->load();

    for (int i = 0; i < n; ++i)
    {
        const double pos = (p0 + i * bpb) / stepBeats;
        const long long abs = (long long) std::floor(pos);
        const bool newStep = abs != lastAbsStep;
        const int step = stepForCount(abs, dir);

        if (seqNote >= 0 && (newStep || pos - (double) abs >= gate || heldCount == 0))
        {
            seqMidi.addEvent(juce::MidiMessage::noteOff(1, seqNote), i);
            seqNote = -1;
        }

        if (newStep)
        {
            lastAbsStep = abs;
            seqStep.store(step);
            if (heldCount > 0 && stOn[step]->load() > 0.5f)
            {
                seqNote = juce::jlimit(0, 127, held[heldCount - 1] + (int) stPit[step]->load());
                seqMidi.addEvent(juce::MidiMessage::noteOn(1, seqNote, juce::jlimit(0.01f, 1.f, stVel[step]->load())), i);
            }
        }
    }

    midi.swapWith(seqMidi);
}

void PotatoBassProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer(midi, 0, n, true);

    double bpm = 120.0, p0 = freePpq;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())
                bpm = juce::jmax(20.0, *b);
            if (pos->getIsPlaying())
                if (auto q = pos->getPpqPosition())
                    p0 = *q;
        }
    const double bpb = bpm / 60.0 / sr;
    freePpq = p0 + n * bpb;

    runSequencer(midi, n, p0, bpb);

    const float depth = lOn->load() > 0.5f ? lDepth->load() : 0.f;
    const bool lfoOn = depth > 0.0005f && n <= (int) lfoBuf.size();
    bus.lfo = nullptr;
    if (!lfoOn)
        lfoPrimed = false;
    if (lfoOn)
    {
        const int shapeIdx = (int) lShape->load();
        const float expo = lfoCurveExp(lCurve->load());
        const double lfoBeats = syncBeats[(int) lRate->load()];
        const float slew = 1.f - std::exp(-1.f / (0.001f * (float) sr));
        for (int i = 0; i < n; ++i)
        {
            const double ph = (p0 + i * bpb) / lfoBeats;
            const float raw = lfoShape(shapeIdx, (float) (ph - std::floor(ph)), expo);
            if (!lfoPrimed) { lfoState = raw; lfoPrimed = true; }
            lfoState += slew * (raw - lfoState);
            lfoBuf[(size_t) i] = lfoState;
        }

        if ((int) lTarget->load() == 1)
        {
            bus.lfo = lfoBuf.data();
            bus.cutOct = depth * 5.f;
        }
    }

    buffer.clear();
    synth.renderNextBlock(buffer, midi, 0, n);

    if (lfoOn && bus.lfo == nullptr)
    {
        float* L = buffer.getWritePointer(0);
        float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
        for (int i = 0; i < n; ++i)
        {
            const float g = 1.f - depth * (1.f - lfoBuf[(size_t) i]);
            L[i] *= g;
            if (R != nullptr)
                R[i] *= g;
        }
    }

    applyEffects(buffer);

    masterSm.setTargetValue(juce::Decibels::decibelsToGain(master->load(), -59.9f));
    if (buffer.getNumChannels() >= 2)
    {
        float* L = buffer.getWritePointer(0);
        float* R = buffer.getWritePointer(1);
        float sumSq = 0.f;
        int w = scopeWrite.load(std::memory_order_relaxed);
        for (int i = 0; i < n; ++i)
        {
            const float g = masterSm.getNextValue();
            L[i] *= g;
            R[i] *= g;
            sumSq += L[i] * L[i];
            scopeBuf[(size_t) (w++ & (scopeSize - 1))] = 0.5f * (L[i] + R[i]);
        }
        scopeWrite.store(w & (scopeSize - 1), std::memory_order_release);
        meterRms = std::sqrt(sumSq / (float) juce::jmax(1, n));
    }
    heldNotes = heldCount;
}

void PotatoBassProcessor::applyEffects(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2)
        return;

    float* L = buffer.getWritePointer(0);
    float* R = buffer.getWritePointer(1);
    const int total = buffer.getNumSamples();

    for (int off = 0; off < total; off += maxBlock)
        processChunk(L + off, R + off, juce::jmin(maxBlock, total - off));
}

// Chain: distortion (4x oversampled, skipped when Off) -> DC blocker -> compressor -> soft clip.
// Distortion drive follows the input level; its release sets how long that lingers.
void PotatoBassProcessor::processChunk(float* L, float* R, int n)
{
    const float fs = (float) sr;
    const int dt = dOn->load() > 0.5f ? (int) dType->load() : 0;

    if (dt > 0)
    {
        if (lastDt == 0)
        {
            oversampling.reset();
            dcXL = dcYL = dcXR = dcYR = 0.f;
        }

        driveSm.setTargetValue(driveGain(dDrive->load()));
        outSm.setTargetValue(juce::Decibels::decibelsToGain(dGain->load()));
        const float relCoef = std::exp(-1.f / (dRelease->load() * 0.001f * fs));

        for (int i = 0; i < n; ++i)
        {
            distEnv = std::max(std::max(std::abs(L[i]), std::abs(R[i])), distEnv * relCoef);
            dgBuf[(size_t) i] = driveSm.getNextValue() * (0.5f + 0.5f * juce::jmin(1.f, distEnv * 3.f));
            ogBuf[(size_t) i] = outSm.getNextValue();
        }

        float* chans[2] = { L, R };
        juce::dsp::AudioBlock<float> block(chans, 2, (size_t) n);
        auto up = oversampling.processSamplesUp(block);
        float* uL = up.getChannelPointer(0);
        float* uR = up.getChannelPointer(1);
        const int m = (int) up.getNumSamples();

        switch (dt)
        {
            case 1:  shapeBlock<1>(uL, uR, m, dgBuf.data(), ogBuf.data()); break;
            case 2:  shapeBlock<2>(uL, uR, m, dgBuf.data(), ogBuf.data()); break;
            case 3:  shapeBlock<3>(uL, uR, m, dgBuf.data(), ogBuf.data()); break;
            case 4:  shapeBlock<4>(uL, uR, m, dgBuf.data(), ogBuf.data()); break;
            case 5:  shapeBlock<5>(uL, uR, m, dgBuf.data(), ogBuf.data()); break;
            default: shapeBlock<6>(uL, uR, m, dgBuf.data(), ogBuf.data()); break;
        }

        oversampling.processSamplesDown(block);

        for (int i = 0; i < n; ++i)
        {
            const float xl = L[i], xr = R[i];
            dcYL = xl - dcXL + dcCoef * dcYL; dcXL = xl; L[i] = dcYL;
            dcYR = xr - dcXR + dcCoef * dcYR; dcXR = xr; R[i] = dcYR;
        }
    }
    lastDt = dt;

    const float thr = cThresh->load(), ratio = cRatio->load(), knee = cKnee->load();
    const float makeupTarget = cMakeup->load();
    const bool active = cOn->load() > 0.5f && ratio > 1.005f || makeupTarget > 0.01f || makeupSm.isSmoothing();
    mixSm.setTargetValue(cMix->load());
    makeupSm.setTargetValue(makeupTarget);

    if (active)
    {
        const float atkCoef = std::exp(-1.f / (cAttack->load() * 0.001f * fs));
        const float relCoef = std::exp(-1.f / (cRelease->load() * 0.001f * fs));
        const float quietLin = juce::Decibels::decibelsToGain(thr - 0.5f * knee - 1.f);

        for (int i = 0; i < n; ++i)
        {
            const float l = L[i], r = R[i];
            const float lvl = std::max(std::abs(l), std::abs(r));
            const float inDb = lvl >= quietLin ? juce::Decibels::gainToDecibels(lvl, -100.f) : -100.f;
            const float gr = compCurve(inDb, thr, ratio, knee) - inDb;
            const float c = gr < compGr ? atkCoef : relCoef;
            compGr = c * compGr + (1.f - c) * gr;

            const float cg = juce::Decibels::decibelsToGain(compGr + makeupSm.getNextValue());
            const float mix = mixSm.getNextValue();
            L[i] = softClip(l + mix * (l * cg - l));
            R[i] = softClip(r + mix * (r * cg - r));
        }
        grMeter.store(-compGr);
    }
    else
    {
        compGr = 0.f;
        mixSm.skip(n);
        makeupSm.skip(n);
        for (int i = 0; i < n; ++i)
        {
            L[i] = softClip(L[i]);
            R[i] = softClip(R[i]);
        }
        grMeter.store(0.f);
    }
}

juce::AudioProcessorEditor* PotatoBassProcessor::createEditor() { return new PotatoBassEditor(*this); }

void PotatoBassProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("preset", currentPreset.load(), nullptr);
    state.setProperty("pattern", currentPattern.load(), nullptr);
    state.setProperty("scope", scopeMode.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void PotatoBassProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml(*xml);
            currentPreset = juce::jlimit(0, pb::numPresets - 1, (int) tree.getProperty("preset", 1));
            currentPattern = juce::jlimit(0, pb::numSeqPatterns - 1, (int) tree.getProperty("pattern", 1));
            scopeMode = juce::jlimit(0, 2, (int) tree.getProperty("scope", 0));
            apvts.replaceState(tree);
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PotatoBassProcessor(); }
