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
        synth.addVoice(new BassVoice(apvts, wavetables));

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
}

void PotatoBassProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    sr = sampleRate;
    maxBlock = juce::jmax(1, samplesPerBlock);

    oversampling.initProcessing((size_t) maxBlock);
    oversampling.reset();
    dgBuf.assign((size_t) maxBlock, 0.f);
    ogBuf.assign((size_t) maxBlock, 0.f);

    driveSm.reset(sampleRate, 0.02);  driveSm.setCurrentAndTargetValue(driveGain(dDrive->load()));
    outSm.reset(sampleRate, 0.02);    outSm.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(dGain->load()));
    makeupSm.reset(sampleRate, 0.02); makeupSm.setCurrentAndTargetValue(cMakeup->load());
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

void PotatoBassProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
    applyEffects(buffer);
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
    const int dt = (int) dType->load();

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
    const bool active = ratio > 1.005f || makeupTarget > 0.01f || makeupSm.isSmoothing();
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
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void PotatoBassProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PotatoBassProcessor(); }
