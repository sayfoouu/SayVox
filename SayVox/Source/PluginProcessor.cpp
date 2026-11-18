#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

namespace
{
struct RoleMod { float comp, grain, bright, echoMul, echoAdd, spaceMul, spaceAdd, width, gainDb; };

// Lead, Doubles, Backs, Adlibs
constexpr RoleMod kRoles[4] = {
    {  0.f,   0.f,   0.f, 1.0f,  0.f, 1.0f,  0.f,  0.f,  0.f },
    { 10.f,   0.f,  -8.f, 0.4f,  0.f, 0.6f,  0.f, 25.f, -4.f },
    { 15.f,  -5.f, -12.f, 0.6f,  0.f, 1.0f, 15.f, 45.f, -5.f },
    {  5.f,  10.f,   6.f, 1.0f, 30.f, 1.0f, 10.f, 25.f, -2.f }
};

constexpr float kBeats[4] = { 1.0f, 0.5f, 0.75f, 0.25f }; // 1/4, 1/8, 1/8 pointée, 1/16

String pctText (float v, int) { return String (roundToInt (v)); }
String dbText (float v, int) { return String (v, 1) + " dB"; }
String hzText (float v, int) { return String (roundToInt (v)) + " Hz"; }
String pc2Text (float v, int) { return String (roundToInt (v)) + " %"; }
}

//==============================================================================
AudioProcessorValueTreeState::ParameterLayout SayVoxProcessor::createLayout()
{
    AudioProcessorValueTreeState::ParameterLayout l;

    auto pct = [&] (const char* id, const char* name, float def)
    {
        l.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
                                                      NormalisableRange<float> (0.f, 100.f, 0.1f), def,
                                                      AudioParameterFloatAttributes().withStringFromValueFunction (pctText)));
    };

    pct ("comp",   "Compression", 55.f);
    pct ("grain",  "Grain",       15.f);
    pct ("bright", "Brillance",   50.f);
    pct ("echo",   "Echo",         8.f);
    pct ("space",  "Espace",      14.f);
    pct ("width",  "Largeur",     12.f);

    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "intensity", 1 }, "Intensite",
                                                  NormalisableRange<float> (0.f, 150.f, 0.1f), 100.f,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction (pc2Text)));

    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "inGain", 1 }, "Entree",
                                                  NormalisableRange<float> (-24.f, 24.f, 0.1f), 0.f,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "outGain", 1 }, "Sortie",
                                                  NormalisableRange<float> (-24.f, 12.f, 0.1f), 0.f,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    // Paramètres de ton (réglés par les presets)
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "hpf", 1 }, "Filtre graves",
                                                  NormalisableRange<float> (40.f, 200.f, 1.f), 85.f,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction (hzText)));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "mud", 1 }, "Anti-boue",
                                                  NormalisableRange<float> (-8.f, 0.f, 0.1f), -3.f,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "body", 1 }, "Corps",
                                                  NormalisableRange<float> (-3.f, 5.f, 0.1f), 1.f,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));
    pct ("deess", "De-esser", 45.f);

    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "role", 1 }, "Role",
                                                   StringArray { "Lead", "Doubles", "Backs", "Adlibs" }, 0));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "delayDiv", 1 }, "Echo sync",
                                                   StringArray { "1/4", "1/8", "1/8 pointee", "1/16" }, 0));
    l.add (std::make_unique<AudioParameterInt> (ParameterID { "preset", 1 }, "Preset", 0, sv::kNumPresets - 1, 0,
                                                AudioParameterIntAttributes().withAutomatable (false)));
    return l;
}

//==============================================================================
SayVoxProcessor::SayVoxProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", AudioChannelSet::stereo(), true)
                          .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SAYVOX", createLayout())
{
}

bool SayVoxProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet();
    const auto out = l.getMainOutputChannelSet();

    if (out != AudioChannelSet::mono() && out != AudioChannelSet::stereo())
        return false;

    if (in == out)
        return true;

    return in == AudioChannelSet::mono() && out == AudioChannelSet::stereo();
}

void SayVoxProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fs = sampleRate;

    for (auto* b : { &hpf1, &hpf2, &deHp, &mud, &body, &pres, &air, &sendHp, &sendLp })
        b->reset();

    comp1.prepare (fs); comp1.setTimes (15.0f, 120.0f);
    comp2.prepare (fs); comp2.setTimes (1.5f, 55.0f);
    deGr = deEnv = 0.0f;

    wideA.prepare ((int) (0.05 * fs));
    wideB.prepare ((int) (0.05 * fs));
    echoL.prepare ((int) (2.5 * fs));
    echoR.prepare ((int) (2.5 * fs));
    fbL.prepare (fs);
    fbR.prepare (fs);
    delaySm.reset (fs, 0.15);
    delaySm.setCurrentAndTargetValue ((float) (0.5 * fs));

    reverb.setSampleRate (fs);
    reverb.reset();
    sendBuf.setSize (2, jmax (samplesPerBlock, 512));

    echoAmt.reset (fs, 0.05);   echoAmt.setCurrentAndTargetValue (0.0f);
    spaceAmt.reset (fs, 0.05);  spaceAmt.setCurrentAndTargetValue (0.0f);
    widthAmt.reset (fs, 0.05);  widthAmt.setCurrentAndTargetValue (0.0f);
    outGain.reset (fs, 0.03);   outGain.setCurrentAndTargetValue (1.0f);
}

//==============================================================================
void SayVoxProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const int nIn = getTotalNumInputChannels();
    const int nOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();

    for (int c = nIn; c < nOut; ++c)
        buffer.clear (c, 0, n);
    if (nIn == 1 && nOut >= 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, n);

    const int nc = jmin (2, buffer.getNumChannels());
    if (nc < 1 || n < 1)
        return;

    // ---- Paramètres -------------------------------------------------------
    const float k = P ("intensity") * 0.01f;
    const int role = jlimit (0, 3, (int) P ("role"));
    const auto& rm = kRoles[role];

    const float cComp  = jlimit (0.f, 100.f, (P ("comp") + rm.comp) * k);
    const float cGrain = jlimit (0.f, 100.f, (P ("grain") + rm.grain) * k);
    const float cEcho  = jlimit (0.f, 100.f, (P ("echo") * rm.echoMul + rm.echoAdd) * k);
    const float cSpace = jlimit (0.f, 100.f, (P ("space") * rm.spaceMul + rm.spaceAdd) * k);
    const float cWidth = jlimit (0.f, 100.f, (P ("width") + rm.width) * k);
    const float cDeess = jlimit (0.f, 100.f, P ("deess") * jmin (1.0f, k));

    const float b = jlimit (0.f, 100.f, P ("bright") + rm.bright);
    const float airDb  = (-2.0f + 9.0f * b / 100.0f) * k;
    const float presDb = (-1.5f + 4.5f * b / 100.0f) * k;

    // ---- Entrée ---------------------------------------------------------------
    buffer.applyGain (Decibels::decibelsToGain (P ("inGain")));
    inLevel.store (buffer.getMagnitude (0, n));

    // ---- Filtre passe-haut 24 dB/oct ---------------------------------------
    hpf1.update (sv::EqBand::HighPass, fs, P ("hpf"), 0.5412f, 0.0f);
    hpf2.update (sv::EqBand::HighPass, fs, P ("hpf"), 1.3066f, 0.0f);

    for (int c = 0; c < nc; ++c)
    {
        auto* d = buffer.getWritePointer (c);
        for (int i = 0; i < n; ++i)
            d[i] = hpf2.process (c, hpf1.process (c, d[i]));
    }

    // ---- De-esser (détection liée sur les aigus) ---------------------------
    if (cDeess > 2.0f)
    {
        deHp.update (sv::EqBand::HighPass, fs, 5500.0f, 0.707f, 0.0f);
        const float thr = -14.0f - 18.0f * (cDeess / 100.0f);
        const float atk = std::exp (-1.0f / (0.0005f * (float) fs));
        const float rel = std::exp (-1.0f / (0.045f * (float) fs));
        float* ch[2] = { buffer.getWritePointer (0), nc > 1 ? buffer.getWritePointer (1) : nullptr };

        for (int i = 0; i < n; ++i)
        {
            float hp[2] = { deHp.process (0, ch[0][i]), ch[1] != nullptr ? deHp.process (1, ch[1][i]) : 0.0f };
            const float lvl = jmax (std::abs (hp[0]), std::abs (hp[1]));
            deEnv = lvl > deEnv ? atk * deEnv + (1.0f - atk) * lvl : rel * deEnv + (1.0f - rel) * lvl;

            const float db = Decibels::gainToDecibels (deEnv, -100.0f);
            const float gr = jmax (0.0f, db - thr) * 0.75f; // ratio 4:1
            const float g = Decibels::decibelsToGain (-gr);

            ch[0][i] -= hp[0] * (1.0f - g);
            if (ch[1] != nullptr)
                ch[1][i] -= hp[1] * (1.0f - g);
        }
    }

    // ---- EQ soustractif (boue + corps) -------------------------------------
    mud.update (sv::EqBand::Peak, fs, 300.0f, 1.0f, P ("mud") * k);
    body.update (sv::EqBand::LowShelf, fs, 140.0f, 0.7f, P ("body") * k);

    for (int c = 0; c < nc; ++c)
    {
        auto* d = buffer.getWritePointer (c);
        for (int i = 0; i < n; ++i)
            d[i] = body.process (c, mud.process (c, d[i]));
    }

    // ---- Compresseur 1 (doux, type opto) -----------------------------------
    {
        const float thr = -8.0f - cComp * 0.20f;
        const float ratio = 1.0f + cComp * 0.035f;
        const float makeup = (-thr) * (1.0f - 1.0f / ratio) * 0.35f;
        comp1.setParams (thr, ratio, 6.0f, cComp < 0.5f ? 0.0f : makeup);
        comp1.process (buffer);
    }

    // ---- Saturation (grain) ---------------------------------------------------
    if (cGrain > 0.5f)
    {
        const float d = 1.0f + cGrain * 0.05f;
        const float mix = jmin (1.0f, cGrain / 30.0f);
        const float bias = 0.08f * (cGrain / 100.0f);
        const float tb = std::tanh (bias);

        for (int c = 0; c < nc; ++c)
        {
            auto* x = buffer.getWritePointer (c);
            for (int i = 0; i < n; ++i)
            {
                const float s = (std::tanh (d * x[i] + bias) - tb) / d;
                x[i] = (1.0f - mix) * x[i] + mix * s;
            }
        }
    }

    // ---- Présence + air ---------------------------------------------------------
    pres.update (sv::EqBand::Peak, fs, 3500.0f, 0.8f, presDb);
    air.update (sv::EqBand::HighShelf, fs, 11000.0f, 0.6f, airDb);

    for (int c = 0; c < nc; ++c)
    {
        auto* d = buffer.getWritePointer (c);
        for (int i = 0; i < n; ++i)
            d[i] = air.process (c, pres.process (c, d[i]));
    }

    // ---- Compresseur 2 (attrape-crêtes rapide) ----------------------------
    {
        const float thr = -4.0f - cComp * 0.08f;
        const float ratio = 1.0f + cComp * 0.06f;
        const float makeup = (-thr) * (1.0f - 1.0f / ratio) * 0.25f;
        comp2.setParams (thr, ratio, 4.0f, cComp < 0.5f ? 0.0f : makeup);
        comp2.process (buffer);
    }

    // ---- Effets : largeur (doubler), écho ping-pong, reverb ----------------
    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bp = pos->getBpm())
                if (*bp > 20.0 && *bp < 400.0)
                    bpm = *bp;

    const float beats = kBeats[jlimit (0, 3, (int) P ("delayDiv"))];
    const float delaySamples = jlimit (64.0f, (float) (2.4 * fs), (float) (60.0 / bpm * beats * fs));
    delaySm.setTargetValue (delaySamples);

    echoAmt.setTargetValue (cEcho / 100.0f * 0.55f);
    spaceAmt.setTargetValue (cSpace / 100.0f * 0.5f);
    widthAmt.setTargetValue (nc > 1 ? cWidth / 100.0f * 0.9f : 0.0f);
    outGain.setTargetValue (Decibels::decibelsToGain (P ("outGain") + rm.gainDb));

    if (sendBuf.getNumSamples() < n)
        sendBuf.setSize (2, n, false, false, true);

    float* L = buffer.getWritePointer (0);
    float* R = nc > 1 ? buffer.getWritePointer (1) : nullptr;
    float* sL = sendBuf.getWritePointer (0);
    float* sR = sendBuf.getWritePointer (1);

    const float twoPi = MathConstants<float>::twoPi;
    const float incA = twoPi * 0.45f / (float) fs;
    const float incB = twoPi * 0.62f / (float) fs;
    const float msToSamp = 0.001f * (float) fs;
    const float fb = 0.38f;

    for (int i = 0; i < n; ++i)
    {
        const float inL = L[i];
        const float inR = R != nullptr ? R[i] : inL;
        const float m = 0.5f * (inL + inR);

        float outL = inL, outR = inR;

        // Largeur : deux copies décorrélées en opposition (compatible mono)
        const float ww = widthAmt.getNextValue();
        wideA.write (m);
        wideB.write (m);
        phA += incA; if (phA > twoPi) phA -= twoPi;
        phB += incB; if (phB > twoPi) phB -= twoPi;
        const float dA = (11.0f + 0.5f * std::sin (phA)) * msToSamp;
        const float dB = (17.0f + 0.5f * std::sin (phB)) * msToSamp;
        const float side = ww * 0.5f * (wideA.read (dA) - wideB.read (dB));
        outL += side;
        outR -= side;

        // Écho ping-pong filtré
        const float dly = delaySm.getNextValue();
        const float dl = echoL.read (dly);
        const float dr = echoR.read (dly);
        echoL.write (fbL.process (m + fb * dr));
        echoR.write (fbR.process (fb * dl));

        const float e = echoAmt.getNextValue();
        outL += e * dl;
        outR += e * dr;

        // Envoi reverb : voix + un peu d'écho
        const float send = m + 0.5f * e * (dl + dr);
        sL[i] = send;
        sR[i] = send;

        L[i] = outL;
        if (R != nullptr) R[i] = outR;
    }

    // Reverb (envoi filtré 250 Hz - 7,5 kHz)
    sendHp.update (sv::EqBand::HighPass, fs, 250.0f, 0.707f, 0.0f);
    sendLp.update (sv::EqBand::LowPass, fs, 7500.0f, 0.707f, 0.0f);

    for (int i = 0; i < n; ++i)
    {
        const float x = sendLp.process (0, sendHp.process (0, sL[i]));
        sL[i] = x;
        sR[i] = x;
    }

    Reverb::Parameters rp;
    rp.roomSize = 0.62f;
    rp.damping = 0.5f;
    rp.wetLevel = 0.25f;
    rp.dryLevel = 0.0f;
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
    reverb.processStereo (sL, sR, n);

    // Mixage final + sortie
    float peakOut = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        const float sp = spaceAmt.getNextValue();
        const float og = outGain.getNextValue();

        if (R != nullptr)
        {
            L[i] = sv::softClip ((L[i] + sp * sL[i]) * og);
            R[i] = sv::softClip ((R[i] + sp * sR[i]) * og);
            peakOut = jmax (peakOut, std::abs (L[i]), std::abs (R[i]));
        }
        else
        {
            L[i] = sv::softClip ((L[i] + sp * 0.5f * (sL[i] + sR[i])) * og);
            peakOut = jmax (peakOut, std::abs (L[i]));
        }
    }
    outLevel.store (peakOut);
}

//==============================================================================
void SayVoxProcessor::applyPreset (int index)
{
    index = jlimit (0, sv::kNumPresets - 1, index);
    const auto& p = sv::kPresets[index];

    auto set = [this] (const char* id, float v)
    {
        if (auto* prm = apvts.getParameter (id))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (prm->convertTo0to1 (v));
            prm->endChangeGesture();
        }
    };

    set ("comp", p.comp);   set ("grain", p.grain);   set ("bright", p.bright);
    set ("echo", p.echo);   set ("space", p.space);   set ("width", p.width);
    set ("hpf", p.hpf);     set ("mud", p.mud);       set ("body", p.body);
    set ("deess", p.deess); set ("preset", (float) index);
}

//==============================================================================
AudioProcessorEditor* SayVoxProcessor::createEditor()
{
    return new SayVoxEditor (*this);
}

void SayVoxProcessor::getStateInformation (MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void SayVoxProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (ValueTree::fromXml (*xml));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SayVoxProcessor();
}
