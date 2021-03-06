#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EqualizerEditor.h"
#include"WebBrowser.h"
//==============================================================================
VoiceChanger_wczAudioProcessorEditor::VoiceChanger_wczAudioProcessorEditor(VoiceChanger_wczAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
    , audioSetupComp(juce::StandalonePluginHolder::getInstance()->deviceManager,0,4,0,4,false,false,false,false)
    , circularMeterL([&]() { return audioProcessor.getRmsLevel(0); },juce::Colours::violet)
    , circularMeterR([&]() { return audioProcessor.getRmsLevel(1); },juce::Colours::lightblue)
	, pEqEditor(std::make_unique<FrequalizerAudioProcessorEditor>(p))
	
{
    //设置参数和旋钮的绑定方法
    reverbSliderAttachments.add(new AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), ParamNames::size, reverbSizeSlider));
    reverbSliderAttachments.add(new AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), ParamNames::damp, reverbDampSlider));
    reverbSliderAttachments.add(new AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), ParamNames::width, reverbWidthSlider));
    reverbSliderAttachments.add(new AudioProcessorValueTreeState::SliderAttachment(audioProcessor.getPluginState(), ParamNames::dryWet, reverbDrywetSlider));
    // reverbButtonAttachments.add(new AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.getPluginState(), ParamNames::freeze, freezeButton));
	/*reverbSizeAttachment(audioProcessor.getPluginState(), ParamNames::size, reverbSizeSlider)
        , reverbDampAttachment(audioProcessor.getPluginState(), ParamNames::damp, reverbDampSlider)
        , reverbWidthAttachment(audioProcessor.getPluginState(), ParamNames::width, reverbWidthSlider)
        , reverbDrywetAttachment(audioProcessor.getPluginState(), ParamNames::dryWet, reverbDrywetSlider)
        , reverbFreezeAttachment(audioProcessor.getPluginState(), ParamNames::freeze, freezeButton)*/
    backgroundTexture = backgroundTexture.rescaled(1400, 600);

    thumbnailCore = new AudioThumbnailComp(audioProcessor.formatManager, audioProcessor.transportSource, audioProcessor.thumbnailCache, audioProcessor.currentlyLoadedFile);
    addAndMakeVisible(thumbnailCore);
    thumbnailCore->addChangeListener(this);
    //可视化电平表组件（paint方法重载）
    addAndMakeVisible(circularMeterL);
    addAndMakeVisible(circularMeterR);
    addAndMakeVisible(horizontalMeterL);
    addAndMakeVisible(horizontalMeterR);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //addAndMakeVisible(audioSetupComp);
    //音频信息设置显示
    auto pluginHolder = juce::StandalonePluginHolder::getInstance();
    juce::AudioDeviceManager::AudioDeviceSetup anotherSetup = pluginHolder->deviceManager.getAudioDeviceSetup();
    //// DBG(audioSetupComp.deviceManager.getAudioDeviceSetup().outputDeviceName);
    //anotherSetup.inputDeviceName = juce::String("VoiceMeeter Output (VB-Audio VoiceMeeter VAIO)");
    //anotherSetup.outputDeviceName = juce::CharPointer_UTF8("\xe8\x80\xb3\xe6\x9c\xba (AirPods)");
    anotherSetup.sampleRate = 44100;
    //audioSetupComp.deviceManager.setAudioDeviceSetup(anotherSetup, false);
    setSize(1400, 600);
    //// addAndMakeVisible(bkg);
    //// addAndMakeVisible(playAudioFileComponent);
    addAndMakeVisible(audioSetupComp);

    //juce::StandalonePluginHolder::getInstance()
    // audioSetupComp
    //// = juce::String("VB-Audio VoiceMeeter VAIO");

    startTimerHz(20);//主界面定时器工作
    //=====================================================================================================
    //设置所有按钮的位置、参数绑定、及视觉效果
    
    pPitchSlider.reset(new juce::Slider("PitchShiftSlider"));
    pPitchSlider->setLookAndFeel(&otherLookAndFeel);
    AudioProcessorEditor::addAndMakeVisible(pPitchSlider.get());
    pPitchSlider->setRange(-12, 12.0, 0.02);
    pPitchSlider->setTooltip(TRANS("higher"));
    pPitchSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pPitchSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    pPitchSlider->addListener(this);

    pPitchSlider->setBounds(20, 376, 80, 80);


    addAndMakeVisible(pitchShiftLabel);
    pitchShiftLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    pitchShiftLabel.setJustificationType(juce::Justification::topLeft);
    pitchShiftLabel.setText(juce::CharPointer_UTF8("\xe9\x9f\xb3\xe8\xb0\x83\xe7\xbc\xa9\xe6\x94\xbe"), dontSendNotification);
    pitchShiftLabel.setBounds(23, 359, 80, 20);

    pPeakSlider.reset(new juce::Slider("PeakShiftSlider"));
    pPeakSlider->setLookAndFeel(&otherLookAndFeel);
    AudioProcessorEditor::addAndMakeVisible(pPeakSlider.get());
    pPeakSlider->setRange(0.5, 2.0, 0.02);
    pPeakSlider->setTooltip(TRANS("peaker"));
    
    pPeakSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pPeakSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    pPeakSlider->addListener(this);

    pPeakSlider->setBounds(90, 376, 80, 80);

    addAndMakeVisible(formantShiftLabel);
    formantShiftLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    formantShiftLabel.setJustificationType(juce::Justification::topLeft);
    formantShiftLabel.setText(juce::CharPointer_UTF8("\xe5\x85\xb1\xe6\x8c\xaf\xe5\xb3\xb0\xe7\xa7\xbb\xe5\x8a\xa8"), dontSendNotification);
    formantShiftLabel.setBounds(93, 359, 80, 20);

    pDynamicsThresholdSlider.reset(new juce::Slider("DynamicsThresholdSlider"));
    pDynamicsThresholdSlider->setRange(-50, 0, 0.001f);
    pDynamicsThresholdSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsThresholdSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsThresholdSlider->addListener(this);
    pDynamicsThresholdSlider->setTooltip(TRANS("dynamicsThreshold"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsThresholdSlider.get());

    pDynamicsThresholdSlider->setBounds(30, 271, 70, 70);

    pDynamicsRatioSlider.reset(new juce::Slider("DynamicsRatioSlider"));
    pDynamicsRatioSlider->setRange(1.0, 25.0, 0.01f);
    pDynamicsRatioSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsRatioSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsRatioSlider->addListener(this);
    pDynamicsRatioSlider->setTooltip(TRANS("dynamicsRatio"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsRatioSlider.get());

    pDynamicsRatioSlider->setBounds(85, 271, 70, 70);


    pDynamicsAttackSlider.reset(new juce::Slider("DynamicsAttackSlider"));
    pDynamicsAttackSlider->setRange(0.00001, 1.0f, 0.00001f);
    pDynamicsAttackSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsAttackSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsAttackSlider->addListener(this);
    pDynamicsAttackSlider->setTooltip(TRANS("dynamicsAttack"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsAttackSlider.get());

    pDynamicsAttackSlider->setBounds(140, 271, 70, 70);

    pDynamicsReleaseSlider.reset(new juce::Slider("DynamicsReleaseSlider"));
    pDynamicsReleaseSlider->setRange(0.001f, 2.0f, 0.001f);
    pDynamicsReleaseSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsReleaseSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsReleaseSlider->addListener(this);
    pDynamicsReleaseSlider->setTooltip(TRANS("dynamicsRelease"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsReleaseSlider.get());

    pDynamicsReleaseSlider->setBounds(195, 271, 70, 70);


    pDynamicsMakeupGainSlider.reset(new juce::Slider("DynamicsMakeupGainSlider"));
    pDynamicsMakeupGainSlider->setRange(-60.0f, 30.0f, 0.01f);
    pDynamicsMakeupGainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsMakeupGainSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsMakeupGainSlider->addListener(this);
    pDynamicsMakeupGainSlider->setTooltip(TRANS("dynamicsMakeupGain"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsMakeupGainSlider.get());

    pDynamicsMakeupGainSlider->setBounds(250, 271, 70, 70);

    mmButton.setButtonText(juce::CharPointer_UTF8("\xe5\xa6\xb9\xe5\xa6\xb9"));
    mmButton.onClick = [this] { mmButtonClicked(); };
    mmButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    AudioProcessorEditor::addAndMakeVisible(&mmButton);
    mmButton.setEnabled(true);


    xjjButton.setButtonText(juce::CharPointer_UTF8 ("\xe5\xb0\x8f\xe5\xa7\x90\xe5\xa7\x90"));
    xjjButton.onClick = [this] { xjjButtonClicked(); };
    xjjButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    AudioProcessorEditor::addAndMakeVisible(&xjjButton);
    xjjButton.setEnabled(true);

    ljButton.setButtonText("giegie");
    ljButton.onClick = [this] {ljButtonClicked(); };
    ljButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
    AudioProcessorEditor::addAndMakeVisible(&ljButton);
    ljButton.setEnabled(true);


    xpyButton.setButtonText(juce::CharPointer_UTF8("\xe5\xb0\x8f\xe6\x9c\x8b\xe5\x8f\x8b"));
    xpyButton.onClick = [this] {xpyButtonClicked(); };
    xpyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    AudioProcessorEditor::addAndMakeVisible(&xpyButton);
    xpyButton.setEnabled(true);



    realtimeButton.setButtonText(juce::CharPointer_UTF8("\xe5\xae\x9e\xe6\x97\xb6\xe6\xa8\xa1\xe5\xbc\x8f"));
    realtimeButton.onClick = [this] {realtimeButtonClicked(); };
    realtimeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkslateblue);
    AudioProcessorEditor::addAndMakeVisible(&realtimeButton);
    realtimeButton.setEnabled(true);


    offlineButton.setButtonText(juce::CharPointer_UTF8("\xe7\xa6\xbb\xe7\xba\xbf\xe6\xa8\xa1\xe5\xbc\x8f"));
    offlineButton.onClick = [this] {offlineButtonClicked(); };
    offlineButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    AudioProcessorEditor::addAndMakeVisible(&offlineButton);
    offlineButton.setEnabled(true);


    resetAllButton.setButtonText("RESET");
    resetAllButton.onClick = [this] {resetAllButtonClicked(); };
    resetAllButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    AudioProcessorEditor::addAndMakeVisible(&resetAllButton);
    resetAllButton.setEnabled(true);

    addAndMakeVisible(modeChooseLabel);
    modeChooseLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    modeChooseLabel.setJustificationType(juce::Justification::topLeft);
    modeChooseLabel.setText(CharPointer_UTF8("\xe6\xa8\xa1\xe5\xbc\x8f\xe9\x80\x89\xe6\x8b\xa9"), dontSendNotification);

    addAndMakeVisible(freqDomainLabel);
    freqDomainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    freqDomainLabel.setJustificationType(juce::Justification::topLeft);
    freqDomainLabel.setText(juce::CharPointer_UTF8("\xe9\xa2\x91\xe5\x9f\x9f"), juce::NotificationType::dontSendNotification);



    addAndMakeVisible(timeDomainLabel);
    timeDomainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    timeDomainLabel.setJustificationType(juce::Justification::topLeft);
    timeDomainLabel.setText(juce::CharPointer_UTF8("\xe6\x97\xb6\xe5\x9f\x9f"), juce::NotificationType::dontSendNotification);

    switchPitchMethodButton.setButtonText("FD Proc.");
    switchPitchMethodButton.onClick = [this] { switchPitchMethodButtonClicked(); };
    switchPitchMethodButton.setClickingTogglesState(true);
    switchPitchMethodButton.setImages(
        false, true,false,
        juce::ImageFileFormat::loadFrom(BinaryData::switch_left_png,BinaryData::switch_left_pngSize),1.0f,{},
        {},1.0f,{},
        juce::ImageFileFormat::loadFrom(BinaryData::switch_right_png,BinaryData::switch_right_pngSize),1.0f,{},
        0.0f);
	addAndMakeVisible(&switchPitchMethodButton);

    addAndMakeVisible(specificConversionLabel);
    specificConversionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    specificConversionLabel.setJustificationType(juce::Justification::topLeft);
    specificConversionLabel.setText(juce::CharPointer_UTF8("\xe7\x89\xb9\xe5\xae\x9a"), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(generalConversionLabel);
    generalConversionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    generalConversionLabel.setJustificationType(juce::Justification::topLeft);
    generalConversionLabel.setText(juce::CharPointer_UTF8("\xe9\x80\x9a\xe7\x94\xa8"), juce::NotificationType::dontSendNotification);

    switchVoiceConversionButton.setClickingTogglesState(true);
	switchVoiceConversionButton.onClick = [this] {switchVoiceConversionButtonClicked(); };
    switchVoiceConversionButton.setImages(
        false, true, false,
        juce::ImageFileFormat::loadFrom(BinaryData::switch_left_png, BinaryData::switch_left_pngSize), 1.0f, {},
        {}, 1.0f, {},
        juce::ImageFileFormat::loadFrom(BinaryData::switch_right_png, BinaryData::switch_right_pngSize), 1.0f, {},
        0.0f);
    addAndMakeVisible(switchVoiceConversionButton);
    
    allFuncButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80\xe5\x8f\x98\xe5\xa3\xb0\xe5\x8a\x9f\xe8\x83\xbd"));
    allFuncButton.onClick = [this] { allFuncButtonClicked(); };
    allFuncButton.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::red);
    allFuncButton.setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, juce::Colours::green);
    addAndMakeVisible(&allFuncButton);
    allFuncButton.setEnabled(true);


    openReverbButton.setClickingTogglesState(true);
    openReverbButton.onClick = [this] {openReverbButtonClicked(); };
    openReverbButton.setImages(
        false, true, false,
        juce::ImageFileFormat::loadFrom(BinaryData::switch_left_png, BinaryData::switch_left_pngSize), 1.0f, {},
        {}, 1.0f, {},
        juce::ImageFileFormat::loadFrom(BinaryData::switch_right_png, BinaryData::switch_right_pngSize), 1.0f, {},
        0.0f);
    addAndMakeVisible(openReverbButton);

    reverbSizeSlider.setLookAndFeel(&otherLookAndFeel);
    reverbSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    addAndMakeVisible(reverbSizeSlider);
    reverbWidthSlider.setLookAndFeel(&otherLookAndFeel);
    reverbWidthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    addAndMakeVisible(reverbWidthSlider);
    reverbDampSlider.setLookAndFeel(&otherLookAndFeel);
    reverbDampSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    addAndMakeVisible(reverbDampSlider);
    reverbDrywetSlider.setLookAndFeel(&otherLookAndFeel);
    reverbDrywetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    addAndMakeVisible(reverbDrywetSlider);
    addAndMakeVisible(reverbDampLabel);
    addAndMakeVisible(reverbWidthLabel);
    addAndMakeVisible(reverbOpenLabel);
    addAndMakeVisible(reverbDryWetLabel);
    addAndMakeVisible(reverbSizeLabel);



    openFileButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80\xe6\x9c\xac\xe5\x9c\xb0\xe6\x96\x87\xe4\xbb\xb6"));
    openFileButton.onClick = [this] { openFileButtonClicked(); };
    openFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::mediumseagreen);
    AudioProcessorEditor::addAndMakeVisible(&openFileButton);

    openModelButton.setButtonText(juce::CharPointer_UTF8("\xe5\x8a\xa0\xe8\xbd\xbd\xe6\xa8\xa1\xe5\x9e\x8b"));
    openModelButton.onClick = [this] {openModelButtonClicked(); };
    openModelButton.setColour(juce::TextButton::buttonColourId, juce::Colours::gold);
    //openModelButton.setColour(juce::TextButton::textColourOffId, juce::Colours::red);
    //openModelButton.setColour(juce::TextButton::textColourOnId, juce::Colours::red);
	addAndMakeVisible(openModelButton);

    stopPlayFileButton.setButtonText(juce::CharPointer_UTF8("\xe6\x9a\x82\xe5\x81\x9c"));
    stopPlayFileButton.onClick = [this] { stopPlayFileButtonClicked(); };
    stopPlayFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    stopPlayFileButton.setEnabled(true);
    AudioProcessorEditor::addAndMakeVisible(&stopPlayFileButton);

    playFileButton.setButtonText(juce::CharPointer_UTF8("\xe6\x92\xad\xe6\x94\xbe"));
    playFileButton.onClick = [this] { playFileButtonClicked(); };
    playFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::hotpink);
    playFileButton.setEnabled(true);
    addAndMakeVisible(&playFileButton);


    openInnerRecordingButton.setButtonText(juce::CharPointer_UTF8("\xe5\x86\x85\xe5\xbd\x95"));
    openInnerRecordingButton.onClick = [this] {audioProcessor.startRecording(audioProcessor.lastRecording); };
    openInnerRecordingButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    openInnerRecordingButton.setEnabled(true);
    addAndMakeVisible(&openInnerRecordingButton);
    closeInnerRecordingButton.setButtonText(juce::CharPointer_UTF8("\xe5\x81\x9c\xe6\xad\xa2"));
    closeInnerRecordingButton.onClick = [this] {audioProcessor.stopRecording(); };
    closeInnerRecordingButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    closeInnerRecordingButton.setEnabled(true);
    addAndMakeVisible(&closeInnerRecordingButton);

    openTemplateWindowButton.setButtonText(juce::CharPointer_UTF8("\xe5\xbd\x95\xe5\x85\xa5\xe6\xa8\xa1\xe6\x9d\xbf"));
    openTemplateWindowButton.onClick = [this] { openTemplateWindowButtonClicked(); };
    openTemplateWindowButton.setColour(juce::TextButton::buttonColourId, juce::Colours::cornflowerblue);
    openTemplateWindowButton.setEnabled(true);
    addAndMakeVisible(&openTemplateWindowButton);

    openCameraButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80\xe6\x91\x84\xe5\x83\x8f\xe5\xa4\xb4"));
    openCameraButton.onClick = [this] { openCameraButtonClicked(); };
    openCameraButton.setColour(juce::TextButton::buttonColourId, juce::Colours::yellowgreen);
    openCameraButton.setEnabled(true);
    addAndMakeVisible(&openCameraButton);

    
    
    //  AudioProcessorEditor::addAndMakeVisible(pPlayPositionSlider.get());
    openDawButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80\xe9\x9f\xb3\xe9\xa2\x91\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8"));
    openDawButton.onClick = [this] {openDawButtonClicked(); };
    openDawButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightgreen);
    openDawButton.setEnabled(true);
    addAndMakeVisible(&openDawButton);

    openEqButton.setButtonText(juce::CharPointer_UTF8("\xe5\x9d\x87\xe8\xa1\xa1\xe5\x99\xa8"));
    openEqButton.onClick = [this] { openEqButtonClicked(); };
    openEqButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    openEqButton.setEnabled(true);
    addAndMakeVisible(&openEqButton);

    openWebButton.setButtonText(juce::CharPointer_UTF8("\xe4\xb8\x8a\xe7\xbd\x91"));
    openWebButton.onClick = [this] { openWebButtonClicked(); };
    openWebButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightskyblue);
    openWebButton.setEnabled(true);
    addAndMakeVisible(&openWebButton);

}
VoiceChanger_wczAudioProcessorEditor::~VoiceChanger_wczAudioProcessorEditor()//析构：销毁子界面，关闭定时器
{
    delete[] templateRecordingWindow;
    delete[] eqWindow;
    delete[] dawWindow;
    delete[]webWindow;
    delete[]cameraWindow;
    stopTimer();
}

//==============================================================================
void VoiceChanger_wczAudioProcessorEditor::paint (juce::Graphics& g)
{
    //g.setGradientFill(juce::ColourGradient{ juce::Colours::darkgrey,getLocalBounds().toFloat().getCentre(), juce::Colours::darkgrey.darker(0.7f), {}, true });
    g.drawImageAt(backgroundTexture, 0, 0);
	pPitchSlider.get()->setValue(audioProcessor.getPitchShift());
    pPeakSlider.get()->setValue(audioProcessor.getPeakShift());

    
    int x = 12, y = 12, width = 512, height = 256;
    g.setColour(juce::Colours::lightblue);
    g.drawRect(x - 3, y - 3, width + 6, height + 6, 3);
    // g.drawImage(audioProcessor.getSpectrumView(), x, y, width, height, 0, 0, width, height);
    g.drawImage(audioProcessor.mainpageAnalyser.createSpectrumPlot(), x, y, width, height, 0, 0, width, height);
    g.setColour(juce::Colours::lightblue);
    g.drawRoundedRectangle(circularMeterL.getBounds().toFloat(), 20,3);

    g.drawRoundedRectangle(18, 357, 300, 105, 5, 3);
    g.drawRoundedRectangle(552, 312, 380, 100, 5, 3);
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(circularMeterL.getBounds().reduced(3,3).toFloat(),20);
    // readFilePosition = audioProcessor.nPlayAudioFileSampleNum == 0 ? 0 : audioProcessor.nPlayAudioFilePosition / audioProcessor.nPlayAudioFileSampleNum;
    // pPlayPositionSlider.get()->setValue(readFilePosition);
}
bool operator!=(const TransportInformation A, const TransportInformation B)
{
    if (A.state != B.state)
        return true;
    if (A.transportType != B.transportType)
        return true;
    if (A.audioFilePlayPoint != B.audioFilePlayPoint)
        return true;
    return false;
}
void VoiceChanger_wczAudioProcessorEditor::timerCallback()
{//定时器行为：定期重新绘制图像
    const auto leftGain = audioProcessor.getRmsLevel(0);
    const auto rightGain = audioProcessor.getRmsLevel(1);

    horizontalMeterL.setLevel(leftGain);
    horizontalMeterR.setLevel(rightGain);
    repaint();
    //horizontalMeterL.repaint();
    //horizontalMeterR.repaint();
}

void VoiceChanger_wczAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if(source==thumbnailCore)
	{
        audioProcessor.loadFileIntoTransport(thumbnailCore->getLastDroppedFile());
        thumbnailCore->setFile(thumbnailCore->getLastDroppedFile());
	}
}


void VoiceChanger_wczAudioProcessorEditor::resized()
{
    // 设置页面布局
    auto a = getWidth() / 8;
    auto b = getWidth() / 16;

    xjjButton.setBounds(340, 340, a, 30);
    xpyButton.setBounds(340, 390, a, 30);
    ljButton.setBounds(340, 440, a, 30);
    mmButton.setBounds(340, 490, a, 30);
    realtimeButton.setBounds(340, 540, b, 30);
    offlineButton.setBounds(340 + b, 540, b, 30);
    resetAllButton.setBounds(340, 290, a / 2 , 30);
    openModelButton.setBounds(340 + a / 2, 290, a / 2, 30);

    switchPitchMethodButton.setBounds(220, 375, 50 , 50);
    switchVoiceConversionButton.setBounds(220, 410, 50, 50);
    

    modeChooseLabel.setBounds(213, 360, 90, 30);

    freqDomainLabel.setBounds(275, 390, 50, 50);
    timeDomainLabel.setBounds(170, 390, 50, 50);


    specificConversionLabel.setBounds(275, 425, 50, 50);
    generalConversionLabel.setBounds(170, 425, 50, 50);

    
    
    openReverbButton.setBounds(560, 320, 40, 40);
    reverbSizeSlider.setBounds(620, 310, 70, 70);
    reverbWidthSlider.setBounds(700, 310, 70, 70);
    reverbDampSlider.setBounds(780, 310, 70, 70);
    reverbDrywetSlider.setBounds(860, 310, 70, 70);


    reverbOpenLabel.setBounds(550, 360, 90, 40);
    reverbSizeLabel.setBounds(620, 380, 70, 30);
    reverbWidthLabel.setBounds(700, 380, 70, 30);
    reverbDampLabel.setBounds(780, 380, 70, 30);
    reverbDryWetLabel.setBounds(860, 380, 70, 30);

    openFileButton.setBounds(570, 420, a, 40);
    playFileButton.setBounds(570, 480, a, 40);
    stopPlayFileButton.setBounds(570, 540, a, 40);

    openInnerRecordingButton.setBounds(800, 420, getWidth() / 24, 40);
    closeInnerRecordingButton.setBounds(800 + getWidth() / 24, 420, getWidth() / 24, 40);
    openTemplateWindowButton.setBounds(800, 480, getWidth() / 12, 40);
    openCameraButton.setBounds(800, 540, getWidth() / 12, 40);
    

	audioSetupComp.setBounds(545, 20, 400, 100);
    //playAudioFileComponent.setBounds(AudioProcessorEditor::getLocalBounds());
    allFuncButton.setBounds(580, 3, 200, 20);
    circularMeterL.setBounds(970, 20, 400, 400);
    circularMeterR.setBounds(970, 20, 400, 400);

    horizontalMeterL.setBounds(970, 440, 400, 12);
    horizontalMeterR.setBounds(970, 465, 400, 12);


    openDawButton.setBounds(1000, 515, 100, 50);
    openEqButton.setBounds(1125, 515, 100, 50);
    openWebButton.setBounds(1250, 515, 100, 50);
    
    thumbnailCore->setBounds(10, 470, 320, 110);
}
void VoiceChanger_wczAudioProcessorEditor::sliderValueChanged(juce::Slider* sliderThatWasMoved)//旋钮改变响应
{
    if (sliderThatWasMoved == pPitchSlider.get())
    {
        audioProcessor.setPitchShift((float)pPitchSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pPeakSlider.get())
    {
        audioProcessor.setPeakShift((float)pPeakSlider.get()->getValue());
    }

    else if (sliderThatWasMoved == pDynamicsThresholdSlider.get())
    {
        audioProcessor.setDynamicsThresholdShift((float)pDynamicsThresholdSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsRatioSlider.get())
    {
        audioProcessor.setDynamicsRatioShift((float)pDynamicsRatioSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsAttackSlider.get())
    {
        audioProcessor.setDynamicsAttackShift((float)pDynamicsAttackSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsReleaseSlider.get())
    {
        audioProcessor.setDynamicsReleaseShift((float)pDynamicsReleaseSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsMakeupGainSlider.get())
    {
        audioProcessor.setDynamicsMakeupGainShift((float)pDynamicsMakeupGainSlider.get()->getValue());
    }
    //else if (sliderThatWasMoved == pPlayPositionSlider.get())
    {
        if(&juce::MouseEvent::mouseWasClicked)
        // audioProcessor.transportSource.setPosition(pPlayPositionSlider.get()->getValue());
        //if (shouldUpdatePosition)
        {
            //audioProcessor.nPlayAudioFilePosition = (int)(pPlayPositionSlider.get()->getValue() * audioProcessor.nPlayAudioFileSampleNum);
            // shouldUpdatePosition = false;
        }
    }
}

void VoiceChanger_wczAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatWasMoved)//已被替换成更高级参数树方法
{
/*#if _OPEN_FILTERS
    if (comboBoxThatWasMoved == pFilterTypeComboBox.get())
    {
        audioProcessor.setFilterTypeShift((int)pFilterTypeComboBox.get()->getSelectedId(), audioProcessor.currentFilterIndex);
    }
    else if (comboBoxThatWasMoved == pFilterIndexComboBox.get())
    {
        audioProcessor.currentFilterIndex = round(pFilterIndexComboBox.get()->getSelectedId());
    }
#endif*/
}

void VoiceChanger_wczAudioProcessorEditor::ljButtonClicked()
{
    audioProcessor.setPitchShift(-5);
    // audioProcessor.updateBackendFilterControls();
}
void VoiceChanger_wczAudioProcessorEditor::xjjButtonClicked()
{
    audioProcessor.setPitchShift(2);
}
void VoiceChanger_wczAudioProcessorEditor::mmButtonClicked()
{
    audioProcessor.setPitchShift(4);
}
void VoiceChanger_wczAudioProcessorEditor::xpyButtonClicked()
{
    audioProcessor.setPitchShift(6);
}



void VoiceChanger_wczAudioProcessorEditor::openFileButtonClicked()
{
    // 打开界面选择器，获取文件信息创建相应的读取句柄，而后读入音频流
    juce::FileChooser chooser("choose a WAV or AIFF file",juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3; *.flac");
    //auto chooserFlags = juce::FileBrowserComponent::openMode
    //    | juce::FileBrowserComponent::canSelectFiles;
    if(chooser.browseForFileToOpen())
    //chooser.launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        //auto file = fc.getResult();
        auto file = chooser.getResult();
        if (file == juce::File{})
            return;

        std::unique_ptr<juce::AudioFormatReader>reader(audioProcessor.formatManager.createReaderFor(file));
        if (reader.get() != nullptr)
        {
            // juce_path.cpp line835 JUCE_CHECK_COORDS_ARE_VALID (d[0], d[1])
            thumbnailCore->lastFileDropped = file;
            thumbnailCore->sendChangeMessage();
            //transportInfo.pFileBufferPointingAt = &audioProcessor.fileBuffer;
            duration = (float)reader->lengthInSamples / reader->sampleRate;
            //pPlayPositionSlider->setRange(0, duration, 0.01f);
            // playAudioFileComponent.pPlayPositionSlider->setRange(0, duration, 0.01f);
            audioProcessor.nPlayAudioFileSampleNum = reader->sampleRate;
            if (duration < 1000)
            {
                audioProcessor.setState(Stopping);
                audioProcessor.setTarget(Mainpage);
                audioProcessor.fileBuffer.clear();
                audioProcessor.fileBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                reader->read(
                    &audioProcessor.fileBuffer,
                    0,
                    (int)reader->lengthInSamples,
                    0,
                    true,
                    true
                );
                audioProcessor.nPlayAudioFilePosition = 0;
                //readFilePosition = 0;
                audioProcessor.canReadSampleBuffer = true;
            }
        }
    }
    //);
}

void VoiceChanger_wczAudioProcessorEditor::openModelButtonClicked()
{
    juce::FileChooser chooser(juce::CharPointer_UTF8("\xe5\xaf\xbb\xe6\x89\xbe\xe6\xa8\xa1\xe5\x9e\x8b\xe6\x96\x87\xe4\xbb\xb6"),
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.dat");
    //载入模型信息并为处理器的模型文件初始化
	if(chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        if (file == juce::File{})
            return;
        // String filePath = File::getCurrentWorkingDirectory().getFullPathName();
        auto filenameStr = file.getFullPathName().toStdString();
        size_t start_pos = filenameStr.find("\\");
        filenameStr.replace(start_pos, 1, "/");
        auto filename = filenameStr.c_str();
		{
            // DBG(filename);
            const ScopedLock s1(audioProcessor.modelWriterLock);
            audioProcessor.model = deserializeModel(filename);

            audioProcessor.vcv.release();
            audioProcessor.vcv = std::make_unique<PhaseVocoderForVC>(audioProcessor.getSampleRate(), audioProcessor.model);
            // audioProcessor.vcb.release();
            // audioProcessor.vcb = std::make_unique<VoiceConversionBuffer>(1,audioProcessor.getSampleRate(), audioProcessor.samplesPerBlock, audioProcessor.model);
        }
        audioProcessor.isModelLoaded = true;
    }
}

void VoiceChanger_wczAudioProcessorEditor::stopPlayFileButtonClicked()
{
    //ti.setState(Stopping);
    audioProcessor.setState(Stopping);
    audioProcessor.transportSource.stop();
    // changeState(Stopping);
}
void VoiceChanger_wczAudioProcessorEditor::playFileButtonClicked()
{
    //ti.setState(Starting);
    audioProcessor.setState(Starting);
    // audioProcessor.transportSource.setPosition(0);
    audioProcessor.transportSource.start();
    // changeState(Starting);
}


void VoiceChanger_wczAudioProcessorEditor::realtimeButtonClicked()
{
    audioProcessor.realtimeMode = true;
    audioProcessor.mainpageAnalyser.spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));

}
void VoiceChanger_wczAudioProcessorEditor::offlineButtonClicked()
{
    audioProcessor.realtimeMode = false;
    audioProcessor.mainpageAnalyser.spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));
}

void VoiceChanger_wczAudioProcessorEditor::resetAllButtonClicked()
{
    // changeState(close);
    pPitchSlider->setValue(0.0);
    pPeakSlider->setValue(1.0);
}
void VoiceChanger_wczAudioProcessorEditor::switchPitchMethodButtonClicked()
{
    if (audioProcessor.useFD.load())
        audioProcessor.useFD = false;
    else
        audioProcessor.useFD = true;
}

void VoiceChanger_wczAudioProcessorEditor::switchVoiceConversionButtonClicked()
{
    if (audioProcessor.openVoiceConversion.load())
        audioProcessor.openVoiceConversion = false;
    else
    {
	    if (audioProcessor.isModelLoaded.load())
	    {
            audioProcessor.openVoiceConversion = true;
	    }
	    else
	    {
            MessageBoxIconType icon = MessageBoxIconType::NoIcon;
            icon = MessageBoxIconType::InfoIcon;
            AlertWindow::showMessageBoxAsync(icon, juce::CharPointer_UTF8("\xe8\xad\xa6\xe5\x91\x8a")
                , juce::CharPointer_UTF8("\xe5\x8a\xa0\xe8\xbd\xbd\xe6\xa8\xa1\xe5" \
                    "\x9e\x8b\xe6\x96\x87\xe4\xbb\xb6\xe5\x90\x8e\xe6\x89\x8d\xe8\x83\xbd\xe6\x89\x93\xe5\xbc\x80"),
                "OK");
            switchVoiceConversionButton.setToggleState(false,dontSendNotification);
	    }
    }
}
void VoiceChanger_wczAudioProcessorEditor::allFuncButtonClicked()
{
    if (audioProcessor.allFunc.load())
        audioProcessor.allFunc = false;
    else
    {
        audioProcessor.allFunc = true;
    }
}
void VoiceChanger_wczAudioProcessorEditor::openReverbButtonClicked()
{
    if (audioProcessor.openReverb.load())
        audioProcessor.openReverb = false;
    else
    {
        audioProcessor.openReverb = true;
    }
}




//打开子界面按钮响应
void VoiceChanger_wczAudioProcessorEditor::openTemplateWindowButtonClicked()
{
    if (templateRecordingWindow)
    {
        templateRecordingWindow->toFront(true);
        templateRecordingWindow->broughtToFront();
    }
    else
    {
        templateRecordingWindow = new NewWindow(juce::String("templateRecordingWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);// new TemplateRecordingWindow();
        templateRecordingWindow->setContentOwned(new TemplateRecordingWindow(audioProcessor), true);
        templateRecordingWindow->addToDesktop();
        templateRecordingWindow->centreWithSize(600, 300);
        templateRecordingWindow->setVisible(true);
        // templateRecordingWindow->setName(juce::CharPointer_UTF8("\xe6\xa8\xa1\xe6\x9d\xbf"));
    }
}

void VoiceChanger_wczAudioProcessorEditor::openCameraButtonClicked()
{
    if (cameraWindow)
    {
        cameraWindow->toFront(true);
        cameraWindow->broughtToFront();
    }
    else
    {
        cameraWindow = new NewWindow(juce::String("cameraWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);
        cameraWindow->setContentOwned(new CameraWindow(), true);
        cameraWindow->addToDesktop();
        cameraWindow->centreWithSize(500, 500);
        cameraWindow->setVisible(true);
    }
}

void VoiceChanger_wczAudioProcessorEditor::openDawButtonClicked()
{
    if (dawWindow)
    {
        dawWindow->toFront(true);
        dawWindow->broughtToFront();
    }
    else
    {
        dawWindow = new NewWindow(juce::String("dawWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);// new TemplateRecordingWindow();
        dawWindow->setContentOwned(new DawComponent(*this,audioProcessor.engineWrapper->engine,audioProcessor.engineWrapper->edit,audioProcessor), true);
        dawWindow->addToDesktop();
        dawWindow->centreWithSize(800, 600);
        dawWindow->setVisible(true);
        // templateRecordingWindow->setName(juce::CharPointer_UTF8("\xe6\xa8\xa1\xe6\x9d\xbf"));
    }
}

void VoiceChanger_wczAudioProcessorEditor::openEqButtonClicked()
{
	if(eqWindow)
	{
        eqWindow->toFront(true);
        
        eqWindow->broughtToFront();
	} 
	else
	{
        eqWindow = new NewWindow(juce::String("eqWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);
#if _OPEN_FILTERS
        pEqEditor.release();
        pEqEditor = std::make_unique<FrequalizerAudioProcessorEditor>(audioProcessor);
		eqWindow->setContentOwned(pEqEditor.get(), true);
#endif
		eqWindow->centreWithSize(900, 500);
		eqWindow->addToDesktop();
        eqWindow->setVisible(true);
	}
}
void VoiceChanger_wczAudioProcessorEditor::openWebButtonClicked()
{
	if(webWindow)
	{
        webWindow->toFront(true);
        webWindow->broughtToFront();
	}
	else
	{
        webWindow = new NewWindow(juce::String("webWindow"), Colours::darkslategrey, DocumentWindow::allButtons);
        webWindow->setContentOwned(new WebBrowser(),true);
        // webWindow->setUsingNativeTitleBar(true);
		webWindow->centreWithSize(500, 500);
        webWindow->addToDesktop();
        webWindow->setVisible(true);
	}
}
