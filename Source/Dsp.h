#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>

namespace sv
{
//==============================================================================
// Filtre biquad stéréo avec cache des coefficients
class EqBand
{
public:
    enum Type { Peak, LowShelf, HighShelf, HighPass, LowPass };

    void reset()
    {
        for (auto& f : filt)
            f.reset();
    }

    void update (Type t, double fs, float freq, float q, float gainDb)
    {
        if (t == type && freq == lastF && q == lastQ && gainDb == lastG && fs == lastFs)
            return;

        type = t; lastF = freq; lastQ = q; lastG = gainDb; lastFs = fs;
        const float gain = juce::Decibels::decibelsToGain (gainDb);
        const float f = juce::jlimit (20.0f, (float) (fs * 0.45), freq);

        juce::dsp::IIR::Coefficients<float>::Ptr c;
        switch (t)
        {
            case Peak:      c = juce::dsp::IIR::Coefficients<float>::makePeakFilter (fs, f, q, gain); break;
            case LowShelf:  c = juce::dsp::IIR::Coefficients<float>::makeLowShelf   (fs, f, q, gain); break;
            case HighShelf: c = juce::dsp::IIR::Coefficients<float>::makeHighShelf  (fs, f, q, gain); break;
            case HighPass:  c = juce::dsp::IIR::Coefficients<float>::makeHighPass   (fs, f, q);       break;
            case LowPass:   c = juce::dsp::IIR::Coefficients<float>::makeLowPass    (fs, f, q);       break;
        }

        for (auto& fl : filt)
            fl.coefficients = c;
    }

    float process (int ch, float x) noexcept { return filt[(size_t) ch].processSample (x); }

private:
    juce::dsp::IIR::Filter<float> filt[2];
    Type type = Peak;
    float lastF = -1.0f, lastQ = -1.0f, lastG = -1000.0f;
    double lastFs = 0.0;
};

//==============================================================================
// Compresseur feed-forward, détection liée (pas de dérive d'image stéréo)
class LinkedComp
{
public:
    void prepare (double sr)
    {
        fs = sr;
        gr = 0.0f;
        setTimes (atkMs, relMs);
    }

    void reset() { gr = 0.0f; }

    void setTimes (float attackMs, float releaseMs)
    {
        atkMs = attackMs; relMs = releaseMs;
        atkC = std::exp (-1.0f / (0.001f * atkMs * (float) fs));
        relC = std::exp (-1.0f / (0.001f * relMs * (float) fs));
    }

    void setParams (float thresholdDb, float r, float kneeDb, float makeup)
    {
        thr = thresholdDb; ratio = juce::jmax (1.0f, r); knee = kneeDb; makeupDb = makeup;
    }

    void process (juce::AudioBuffer<float>& b)
    {
        const int n = b.getNumSamples();
        const int nc = juce::jmin (2, b.getNumChannels());
        float* ch[2] = { b.getWritePointer (0), nc > 1 ? b.getWritePointer (1) : nullptr };
        const float slope = 1.0f - 1.0f / ratio;

        for (int i = 0; i < n; ++i)
        {
            float peak = std::abs (ch[0][i]);
            if (ch[1] != nullptr)
                peak = juce::jmax (peak, std::abs (ch[1][i]));

            const float db = juce::Decibels::gainToDecibels (peak, -100.0f);
            const float over = db - thr;
            float target;

            if (2.0f * over < -knee)
                target = 0.0f;
            else if (2.0f * std::abs (over) <= knee)
            {
                const float t = over + knee * 0.5f;
                target = slope * t * t / (2.0f * knee);
            }
            else
                target = slope * over;

            gr = target > gr ? atkC * gr + (1.0f - atkC) * target
                             : relC * gr + (1.0f - relC) * target;

            const float g = juce::Decibels::decibelsToGain (makeupDb - gr);
            ch[0][i] *= g;
            if (ch[1] != nullptr)
                ch[1][i] *= g;
        }
    }

private:
    double fs = 44100.0;
    float thr = 0.0f, ratio = 1.0f, knee = 6.0f, makeupDb = 0.0f;
    float atkMs = 10.0f, relMs = 100.0f, atkC = 0.0f, relC = 0.0f, gr = 0.0f;
};

//==============================================================================
// Ligne à retard avec interpolation linéaire
class DelayLine
{
public:
    void prepare (int maxSamples)
    {
        size = maxSamples + 8;
        buf.assign ((size_t) size, 0.0f);
        w = 0;
    }

    void write (float x) noexcept
    {
        buf[(size_t) w] = x;
        if (++w >= size) w = 0;
    }

    float read (float delay) const noexcept
    {
        delay = juce::jlimit (1.0f, (float) (size - 4), delay);
        float rp = (float) w - delay;
        if (rp < 0.0f) rp += (float) size;
        int i0 = (int) rp;
        const float fr = rp - (float) i0;
        if (i0 >= size) i0 -= size;
        int i1 = i0 + 1;
        if (i1 >= size) i1 = 0;
        return buf[(size_t) i0] * (1.0f - fr) + buf[(size_t) i1] * fr;
    }

private:
    std::vector<float> buf;
    int w = 0, size = 8;
};

//==============================================================================
// Filtre passe-bande doux (250 Hz - 4 kHz) pour le feedback de l'écho
struct EchoBand
{
    float s1 = 0.0f, s2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;

    void prepare (double fs)
    {
        a1 = std::exp (-2.0f * juce::MathConstants<float>::pi * 4000.0f / (float) fs);
        a2 = std::exp (-2.0f * juce::MathConstants<float>::pi * 250.0f / (float) fs);
        s1 = s2 = 0.0f;
    }

    float process (float x) noexcept
    {
        s1 = a1 * s1 + (1.0f - a1) * x;
        s2 = a2 * s2 + (1.0f - a2) * x;
        return s1 - s2;
    }
};

//==============================================================================
inline float softClip (float x) noexcept
{
    constexpr float t = 0.92f;
    const float a = std::abs (x);
    if (a <= t) return x;
    return std::copysign (t + (1.0f - t) * std::tanh ((a - t) / (1.0f - t)), x);
}
} // namespace sv
