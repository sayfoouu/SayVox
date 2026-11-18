#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

//==============================================================================
class SayVoxLook : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour bg      { 0xff0e1320 };
    static inline const juce::Colour panel   { 0xff171e30 };
    static inline const juce::Colour edge    { 0xff262f4a };
    static inline const juce::Colour accent  { 0xff35d0ff };
    static inline const juce::Colour accent2 { 0xffff4fa3 };
    static inline const juce::Colour textDim { 0xff8d98b8 };

    SayVoxLook();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float sliderPos,
                           float minPos, float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return juce::Font (juce::FontOptions (15.0f, juce::Font::bold)); }
};

//==============================================================================
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
          const juce::String& title, bool big);
    void resized() override;
    void paint (juce::Graphics&) override;

private:
    juce::Slider slider;
    juce::String titleText;
    juce::AudioProcessorValueTreeState::SliderAttachment attach;
};

//==============================================================================
class SayVoxEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit SayVoxEditor (SayVoxProcessor&);
    ~SayVoxEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void showPresetMenu();
    void selectPreset (int idx);
    int currentPreset() const;
    int currentRole() const;
    void setRole (int r);
    void drawMeter (juce::Graphics&, juce::Rectangle<int>, float level, const juce::String& label);

    SayVoxProcessor& proc;
    SayVoxLook look;

    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, presetBtn;
    juce::TextButton roleBtn[4];

    Knob kComp, kGrain, kBright, kEcho, kSpace, kWidth, kIntensity;
    juce::Slider inSlider, outSlider;
    juce::ComboBox delayBox;
    juce::AudioProcessorValueTreeState::SliderAttachment inAtt, outAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> delayAtt;

    float inDisp = 0.0f, outDisp = 0.0f;
    int shownPreset = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SayVoxEditor)
};
