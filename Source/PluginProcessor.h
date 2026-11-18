#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "Dsp.h"
#include "Presets.h"

class SayVoxProcessor : public juce::AudioProcessor
{
public:
    SayVoxProcessor();
    ~SayVoxProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SayVox"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Applique un preset (thread UI)
    void applyPreset (int index);

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float> inLevel { 0.0f }, outLevel { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    float P (const char* id) const { return apvts.getRawParameterValue (id)->load(); }

    double fs = 44100.0;

    // Filtres
    sv::EqBand hpf1, hpf2, deHp, mud, body, pres, air, sendHp, sendLp;
    sv::LinkedComp comp1, comp2;
    float deGr = 0.0f, deEnv = 0.0f;

    // Doubler / largeur
    sv::DelayLine wideA, wideB;
    float phA = 0.0f, phB = 1.7f;

    // Echo
    sv::DelayLine echoL, echoR;
    sv::EchoBand fbL, fbR;
    juce::SmoothedValue<float> delaySm;

    // Reverb
    juce::Reverb reverb;
    juce::AudioBuffer<float> sendBuf;

    // Lissages
    juce::SmoothedValue<float> echoAmt, spaceAmt, widthAmt, outGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SayVoxProcessor)
};
