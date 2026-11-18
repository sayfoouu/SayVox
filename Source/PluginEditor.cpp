#include "PluginEditor.h"

using namespace juce;

namespace
{
String u8 (const char* s) { return String (CharPointer_UTF8 (s)); }
Font f (float h, bool bold = false) { return Font (FontOptions (h, bold ? Font::bold : Font::plain)); }
}

//==============================================================================
SayVoxLook::SayVoxLook()
{
    setColour (ResizableWindow::backgroundColourId, bg);
    setColour (Slider::textBoxTextColourId, Colours::white);
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour (Slider::textBoxHighlightColourId, accent.withAlpha (0.4f));
    setColour (TextButton::textColourOffId, Colours::white);
    setColour (TextButton::textColourOnId, Colours::black);
    setColour (ComboBox::backgroundColourId, panel);
    setColour (ComboBox::textColourId, Colours::white);
    setColour (ComboBox::outlineColourId, edge);
    setColour (ComboBox::arrowColourId, accent);
    setColour (PopupMenu::backgroundColourId, panel);
    setColour (PopupMenu::textColourId, Colours::white);
    setColour (PopupMenu::highlightedBackgroundColourId, accent);
    setColour (PopupMenu::highlightedTextColourId, Colours::black);
}

void SayVoxLook::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                   float startAngle, float endAngle, Slider& s)
{
    auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (8.0f);
    const float r = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float ang = startAngle + pos * (endAngle - startAngle);
    const bool hot = s.isMouseOverOrDragging();

    // Corps du potard
    const float rb = r * 0.74f;
    g.setGradientFill (ColourGradient (Colour (0xff2c3556), c.x, c.y - rb, Colour (0xff0f1527), c.x, c.y + rb, false));
    g.fillEllipse (c.x - rb, c.y - rb, rb * 2.0f, rb * 2.0f);
    g.setColour (edge);
    g.drawEllipse (c.x - rb, c.y - rb, rb * 2.0f, rb * 2.0f, 1.5f);

    // Piste + valeur
    const float ra = r - 2.0f;
    Path track;
    track.addCentredArc (c.x, c.y, ra, ra, 0.0f, startAngle, endAngle, true);
    g.setColour (Colour (0xff222a43));
    g.strokePath (track, PathStrokeType (5.0f, PathStrokeType::curved, PathStrokeType::rounded));

    Path val;
    val.addCentredArc (c.x, c.y, ra, ra, 0.0f, startAngle, ang, true);
    g.setGradientFill (ColourGradient (accent, c.x - r, c.y, accent2, c.x + r, c.y, false));
    g.strokePath (val, PathStrokeType (hot ? 6.0f : 5.0f, PathStrokeType::curved, PathStrokeType::rounded));

    // Aiguille
    Path p;
    p.addRoundedRectangle (-1.8f, -rb * 0.92f, 3.6f, rb * 0.42f, 1.8f);
    g.setColour (Colours::white);
    g.fillPath (p, AffineTransform::rotation (ang).translated (c.x, c.y));
}

void SayVoxLook::drawLinearSlider (Graphics& g, int x, int y, int w, int h, float sliderPos,
                                   float, float, Slider::SliderStyle, Slider&)
{
    const float cy = (float) y + (float) h * 0.5f;
    g.setColour (Colour (0xff222a43));
    g.fillRoundedRectangle ((float) x, cy - 2.0f, (float) w, 4.0f, 2.0f);
    g.setColour (accent);
    g.fillRoundedRectangle ((float) x, cy - 2.0f, sliderPos - (float) x, 4.0f, 2.0f);
    g.setColour (Colours::white);
    g.fillEllipse (sliderPos - 7.0f, cy - 7.0f, 14.0f, 14.0f);
}

void SayVoxLook::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();

    if (on)
        g.setGradientFill (ColourGradient (accent, r.getX(), r.getY(), accent2, r.getRight(), r.getBottom(), false));
    else
        g.setColour (down ? edge.brighter (0.2f) : (over ? edge : panel));

    g.fillRoundedRectangle (r, 8.0f);

    if (! on)
    {
        g.setColour (edge);
        g.drawRoundedRectangle (r, 8.0f, 1.2f);
    }
}

//==============================================================================
Knob::Knob (AudioProcessorValueTreeState& apvts, const String& paramId, const String& title, bool big)
    : titleText (title), attach (apvts, paramId, slider)
{
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (Slider::TextBoxBelow, false, big ? 90 : 70, 22);
    slider.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    slider.setDoubleClickReturnValue (true, apvts.getParameter (paramId)->convertFrom0to1 (
                                                apvts.getParameter (paramId)->getDefaultValue()));
    addAndMakeVisible (slider);
}

void Knob::resized()
{
    slider.setBounds (getLocalBounds().withTrimmedTop (24));
}

void Knob::paint (Graphics& g)
{
    g.setColour (SayVoxLook::textDim);
    g.setFont (f (13.0f, true));
    g.drawText (titleText, getLocalBounds().removeFromTop (22), Justification::centred);
}

//==============================================================================
SayVoxEditor::SayVoxEditor (SayVoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      kComp (p.apvts, "comp", u8 ("COMPRESSION"), false),
      kGrain (p.apvts, "grain", u8 ("GRAIN"), false),
      kBright (p.apvts, "bright", u8 ("BRILLANCE"), false),
      kEcho (p.apvts, "echo", u8 ("ÉCHO"), false),
      kSpace (p.apvts, "space", u8 ("ESPACE"), false),
      kWidth (p.apvts, "width", u8 ("LARGEUR"), false),
      kIntensity (p.apvts, "intensity", u8 ("INTENSITÉ"), true),
      inAtt (p.apvts, "inGain", inSlider),
      outAtt (p.apvts, "outGain", outSlider)
{
    setLookAndFeel (&look);
    setSize (960, 540);

    for (auto* k : { &kComp, &kGrain, &kBright, &kEcho, &kSpace, &kWidth, &kIntensity })
        addAndMakeVisible (k);

    const char* roleNames[4] = { "LEAD", "DOUBLES", "BACKS", "ADLIBS" };
    for (int i = 0; i < 4; ++i)
    {
        roleBtn[i].setButtonText (roleNames[i]);
        roleBtn[i].setClickingTogglesState (false);
        roleBtn[i].onClick = [this, i] { setRole (i); };
        addAndMakeVisible (roleBtn[i]);
    }

    presetBtn.onClick = [this] { showPresetMenu(); };
    prevBtn.onClick = [this] { selectPreset ((currentPreset() + sv::kNumPresets - 1) % sv::kNumPresets); };
    nextBtn.onClick = [this] { selectPreset ((currentPreset() + 1) % sv::kNumPresets); };
    for (auto* b : { &prevBtn, &nextBtn, &presetBtn })
        addAndMakeVisible (b);

    for (auto* s : { &inSlider, &outSlider })
    {
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::TextBoxRight, false, 64, 22);
        addAndMakeVisible (s);
    }
    inSlider.setDoubleClickReturnValue (true, 0.0);
    outSlider.setDoubleClickReturnValue (true, 0.0);

    delayBox.addItemList (StringArray { "1/4", "1/8", "1/8 pointée", "1/16" }, 1);
    addAndMakeVisible (delayBox);
    delayAtt = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "delayDiv", delayBox);

    startTimerHz (30);
    timerCallback();
}

SayVoxEditor::~SayVoxEditor()
{
    setLookAndFeel (nullptr);
}

int SayVoxEditor::currentPreset() const
{
    return jlimit (0, sv::kNumPresets - 1, roundToInt (proc.apvts.getRawParameterValue ("preset")->load()));
}

int SayVoxEditor::currentRole() const
{
    return jlimit (0, 3, roundToInt (proc.apvts.getRawParameterValue ("role")->load()));
}

void SayVoxEditor::setRole (int r)
{
    if (auto* prm = proc.apvts.getParameter ("role"))
    {
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (prm->convertTo0to1 ((float) r));
        prm->endChangeGesture();
    }
}

void SayVoxEditor::selectPreset (int idx)
{
    proc.applyPreset (idx);
    shownPreset = -1;
    timerCallback();
}

void SayVoxEditor::showPresetMenu()
{
    PopupMenu menu;
    PopupMenu sub;
    String lastCat;
    const int cur = currentPreset();

    for (int i = 0; i < sv::kNumPresets; ++i)
    {
        const String cat = u8 (sv::kPresets[i].category);

        if (cat != lastCat)
        {
            if (lastCat.isNotEmpty())
                menu.addSubMenu (lastCat, sub);
            sub = PopupMenu();
            lastCat = cat;
        }
        sub.addItem (i + 1, u8 (sv::kPresets[i].name), true, i == cur);
    }
    if (lastCat.isNotEmpty())
        menu.addSubMenu (lastCat, sub);

    Component::SafePointer<SayVoxEditor> safe (this);
    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&presetBtn),
                        [safe] (int result)
                        {
                            if (safe != nullptr && result > 0)
                                safe->selectPreset (result - 1);
                        });
}

void SayVoxEditor::timerCallback()
{
    inDisp = jmax (proc.inLevel.exchange (0.0f), inDisp * 0.88f);
    outDisp = jmax (proc.outLevel.exchange (0.0f), outDisp * 0.88f);

    const int role = currentRole();
    for (int i = 0; i < 4; ++i)
        roleBtn[i].setToggleState (i == role, dontSendNotification);

    const int pr = currentPreset();
    if (pr != shownPreset)
    {
        shownPreset = pr;
        presetBtn.setButtonText (u8 (sv::kPresets[pr].name));
        repaint();
    }

    repaint (0, 140, 60, 320);
    repaint (getWidth() - 60, 140, 60, 320);
}

//==============================================================================
void SayVoxEditor::drawMeter (Graphics& g, Rectangle<int> r, float level, const String& label)
{
    g.setColour (Colour (0xff0a0f1c));
    g.fillRoundedRectangle (r.toFloat(), 4.0f);

    const float db = Decibels::gainToDecibels (level, -60.0f);
    const float norm = jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    auto fill = r.toFloat().reduced (3.0f);
    const float h = fill.getHeight() * norm;

    g.setGradientFill (ColourGradient (SayVoxLook::accent2, 0.0f, fill.getY(),
                                       SayVoxLook::accent, 0.0f, fill.getBottom(), false));
    g.fillRoundedRectangle (fill.getX(), fill.getBottom() - h, fill.getWidth(), h, 2.0f);

    g.setColour (SayVoxLook::textDim);
    g.setFont (f (11.0f, true));
    g.drawText (label, r.getX() - 10, r.getBottom() + 4, r.getWidth() + 20, 14, Justification::centred);
}

void SayVoxEditor::paint (Graphics& g)
{
    g.setGradientFill (ColourGradient (Colour (0xff131a2c), 0.0f, 0.0f, SayVoxLook::bg, 0.0f, (float) getHeight(), false));
    g.fillAll();

    // En-tête
    g.setColour (Colours::white);
    g.setFont (f (30.0f, true));
    g.drawText ("SAYVOX", 24, 12, 220, 34, Justification::centredLeft);
    g.setColour (SayVoxLook::accent);
    g.setFont (f (11.0f, true));
    g.drawText ("VOCAL STYLE", 26, 44, 220, 16, Justification::centredLeft);

    g.setColour (SayVoxLook::edge);
    g.drawLine (0.0f, 70.0f, (float) getWidth(), 70.0f, 1.0f);

    // Panneau central
    g.setColour (SayVoxLook::panel);
    g.fillRoundedRectangle (44.0f, 138.0f, 872.0f, 312.0f, 14.0f);
    g.setColour (SayVoxLook::edge);
    g.drawRoundedRectangle (44.0f, 138.0f, 872.0f, 312.0f, 14.0f, 1.2f);

    // Libellés du bas
    g.setColour (SayVoxLook::textDim);
    g.setFont (f (12.0f, true));
    g.drawText (u8 ("ENTRÉE"), 50, 458, 200, 16, Justification::centredLeft);
    g.drawText (u8 ("SORTIE"), 660, 458, 200, 16, Justification::centredLeft);
    g.drawText (u8 ("ÉCHO SYNC"), 400, 458, 160, 16, Justification::centred);

    g.setFont (f (11.0f));
    g.drawText (u8 ("Reconstitutions de style approximatives, inspirées des artistes : pas des copies exactes."),
                0, 516, getWidth(), 18, Justification::centred);

    drawMeter (g, { 16, 150, 18, 280 }, inDisp, "IN");
    drawMeter (g, { getWidth() - 34, 150, 18, 280 }, outDisp, "OUT");
}

void SayVoxEditor::resized()
{
    // Préset
    prevBtn.setBounds (300, 18, 40, 34);
    presetBtn.setBounds (346, 18, 268, 34);
    nextBtn.setBounds (620, 18, 40, 34);

    // Rôles
    const int bw = 120, gap = 10;
    int x = (getWidth() - (4 * bw + 3 * gap)) / 2;
    for (auto& b : roleBtn)
    {
        b.setBounds (x, 86, bw, 34);
        x += bw + gap;
    }

    // Potards
    kComp.setBounds (50, 156, 115, 288);
    kGrain.setBounds (165, 156, 115, 288);
    kBright.setBounds (280, 156, 115, 288);
    kIntensity.setBounds (400, 150, 160, 296);
    kEcho.setBounds (565, 156, 115, 288);
    kSpace.setBounds (680, 156, 115, 288);
    kWidth.setBounds (795, 156, 115, 288);

    // Bas
    inSlider.setBounds (50, 476, 280, 28);
    delayBox.setBounds (420, 476, 120, 28);
    outSlider.setBounds (660, 476, 250, 28);
}
