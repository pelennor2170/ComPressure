#include "ComPressure.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "SvfLinearTrapOptimised2.hpp"


// webpage link to use https://www.pelennordsp.com/compressure



class GRMeterFrame : public IControl
{
public:
  GRMeterFrame(const IRECT& bounds)
  : IControl(bounds)
  {
    
    //AttachIControl(this, "");
  }
  
  void Draw(IGraphics& g) override
  {
    g.FillRect(COLOR_GRAY , mRECT.GetCentredInside(2,27).GetVShifted(8).GetHShifted(-107)); // -1 DB
    g.DrawText(myText, "-1dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(-104));
    g.FillRect(COLOR_GRAY , mRECT.GetCentredInside(2,27).GetVShifted(8).GetHShifted(-42));  // -2 DB
    g.DrawText(myText, "-2dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(-39));
    g.FillRect(COLOR_GRAY , mRECT.GetCentredInside(2,27).GetVShifted(8).GetHShifted(18));   // -4 DB
    g.DrawText(myText, "-4dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(15));
    g.FillRect(COLOR_GRAY , mRECT.GetCentredInside(2,27).GetVShifted(8).GetHShifted(67));   // -8 DB
    g.DrawText(myText, "-8dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(64));
    g.FillRect(COLOR_GRAY , mRECT.GetCentredInside(2,27).GetVShifted(8).GetHShifted(108));  // -21 DB
    g.DrawText(myText, "-21dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(105));


  }
private:
  IText myText = DEFAULT_VALUE_TEXT.WithFGColor(COLOR_WHITE).WithSize(13.);
};

  // std::initializer_list<int> custom_markers = {0, -6, -18, -30, -42};
class StdMeterFrame : public IControl
{
public:
StdMeterFrame(const IRECT& bounds)
  : IControl(bounds)
  {
    
    //AttachIControl(this, "");
  }
  
  void Draw(IGraphics& g) override
  {
    g.DrawText(myText, "-42dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(-104));
    g.DrawText(myText, "-30dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(-39));
    g.DrawText(myText, "-18dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(30));
    g.DrawText(myText, "-6dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(105));
    g.DrawText(myText, "0dB", mRECT.GetCentredInside(20,10).GetVShifted(-10).GetHShifted(142));


  }
private:
  IText myText = DEFAULT_VALUE_TEXT.WithFGColor(COLOR_WHITE).WithSize(13.);
};

void MessageBox(const char * str, const char * msg, IGraphics* pGraphics)
{
  #if defined OS_WIN || defined OS_LINUX
    pGraphics->ShowMessageBox(msg, str, kMB_OK);
  #elif defined OS_MAC
    pGraphics->ShowMessageBox(str, msg, kMB_OK);
  #endif


};

ComPressure::ComPressure(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kPressureL)->InitPercentage("Pressure", 0.);
  GetParam(kSpeedL)->InitPercentage("Speed", 25.);
  GetParam(kMewinessL)->InitPercentage("Mewiness", 100.);
  GetParam(kPawClawL)->InitPercentage("PawClaw", 50.);
  GetParam(kMakeupGainL)->InitGain("Makeup Gain", 0., 0., 12.0, 0.1);
  GetParam(kMaxGainReductL)->InitGain("GRLimit", 0, 0, 21, 0.1);

  GetParam(kPressureR)->InitPercentage("Pressure", 0.);
  GetParam(kSpeedR)->InitPercentage("Speed", 25.);
  GetParam(kMewinessR)->InitPercentage("Mewiness",100.);
  GetParam(kPawClawR)->InitPercentage("PawClaw", 50.);
  GetParam(kMakeupGainR)->InitGain("Makeup Gain", 0., 0., 12.0, 0.1);
  GetParam(kMaxGainReductR)->InitGain("GRLimit", 0, 0, 21, 0.1);


  GetParam(kDryWet)->InitPercentage("DryWet", 100.);
  GetParam(kMidSideMode)->InitBool("", false);
  GetParam(kSideDisableL)->InitBool("", false);
  GetParam(kSideDisableR)->InitBool("", false);
  GetParam(kSidechainOn)->InitBool("", false);
  GetParam(kHighpassSC)->InitFrequency("Highpass SC", 30., 30., 666., 1.);

  GetParam(kLinkSides)->InitBool("", false);

  GetParam(kBypass)->InitBool("", false);
  GetParam(kLimiter)->InitBool("", true);

  GetParam(kMainOutput)->InitGain("Output", 0.,-12.0, 12.0, 0.1);

  clipIndicator = false;
  clipperIsOn = true;
  sideEnabledL = true;
  sideEnabledR = true;
  bypassed = false;
  sideChainOn = false;
  sideChainHPFOn = false;
  GRLimitedL = false;
  GRLimitedR = false;


  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(IColor(255,35,35,35));
    
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    //pGraphics->LoadFont("FreeSans", FONTTEST_FN);




    std::initializer_list<int> custom_markers = {0, -6, -18, -30, -42};
    std::initializer_list<int> custom_markers2 = {-21, -8, -4, -2, -1};
    std::initializer_list<const char*> emptyTrackNames = {};
    

    const IRECT b = pGraphics->GetBounds();

    const IColor CP_COLOR_GOLD(255, 246,213,49);
    const IColor CP_COLOR_GREEN(255,116,254,0);
    const IColor CP_COLOR_ORANGE(255,255,147,0);
    const IColor CP_COLOR_RED(255,232,43,0);
    const IColor CP_COLOR_BG(255,35,35,35);


    const IVStyle styleKnobs {
      true, // Show label
      true, // Show value
      {
        DEFAULT_BGCOLOR, // Background
        COLOR_BLACK, // Foreground
        COLOR_BLACK, // Pressed
        CP_COLOR_GOLD, // Frame
        DEFAULT_HLCOLOR, // Highlight
        DEFAULT_SHCOLOR, // Shadow
        COLOR_WHITE, // Extra 1
        DEFAULT_X2COLOR, // Extra 2
        DEFAULT_X3COLOR  // Extra 3
      }, // Colors
      DEFAULT_LABEL_TEXT.WithFGColor(COLOR_WHITE).WithSize(16.),
      DEFAULT_VALUE_TEXT.WithFGColor(CP_COLOR_GOLD)
    };

    const IVStyle styleTogButts = styleKnobs.WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(COLOR_WHITE).WithSize(18.));
    const IVStyle styleHelpButts = styleKnobs.WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(COLOR_WHITE).WithSize(8.));

     const IVStyle styleOutMeter {
      true, // Show label
      true, // Show value
      {
        DEFAULT_BGCOLOR, // Background
        CP_COLOR_ORANGE, // Foreground
        DEFAULT_PRCOLOR, // Pressed
        COLOR_GRAY, // Frame
        COLOR_WHITE, // Highlight
        COLOR_WHITE, // Shadow
        CP_COLOR_GOLD, // Extra 1
        DEFAULT_X2COLOR, // Extra 2
        DEFAULT_X3COLOR  // Extra 3
      }, // Colors
      DEFAULT_LABEL_TEXT.WithFGColor(COLOR_TRANSPARENT).WithSize(16.),
      DEFAULT_VALUE_TEXT.WithFGColor(COLOR_TRANSPARENT).WithSize(13.)
    }; 

  const IVStyle styleInMeter = styleOutMeter.WithColor(kFG, CP_COLOR_GREEN);

     const IVStyle styleGRMeter {
      true, // Show label
      true, // Show value
      {
        DEFAULT_BGCOLOR, // Background
        CP_COLOR_RED, // Foreground
        DEFAULT_PRCOLOR, // Pressed
        COLOR_GRAY, // Frame
        COLOR_TRANSPARENT, // Highlight
        COLOR_TRANSPARENT, // Shadow
        COLOR_TRANSPARENT, // Extra 1
        DEFAULT_X2COLOR, // Extra 2
        DEFAULT_X3COLOR  // Extra 3
      }, // Colors
      DEFAULT_LABEL_TEXT.WithFGColor(COLOR_TRANSPARENT).WithSize(16.),
      DEFAULT_VALUE_TEXT.WithFGColor(COLOR_TRANSPARENT)
    }; 

  // const IVStyle styleGRMeter = styleOutMeter.WithColor(kFG, CP_COLOR_RED);

    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(-350), kPressureL, "Pressure", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(-250), kSpeedL, "Speed", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(-350), kMewinessL, "Mewiness", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(-250), kPawClawL, "PawClaw", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(-350), kMakeupGainL, "Makeup", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(-250), kMaxGainReductL, "GR Limit", styleKnobs,true));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(-34).GetHShifted(-250), CP_COLOR_GOLD), kGRLEnableLLED);

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(100,36).GetVShifted(110).GetHShifted(-300), kSideDisableL,"" , styleTogButts, "Left/Mid IN", "Left/Mid IN"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(78).GetHShifted(-298), CP_COLOR_GOLD), kSideEnableLLED);

    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(150), kPressureR, "Pressure", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(250), kSpeedR, "Speed", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(150), kMewinessR, "Mewiness", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(250), kPawClawR, "PawClaw", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(150), kMakeupGainR, "Makeup", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(250), kMaxGainReductR, "GR Limit", styleKnobs,true));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(-34).GetHShifted(250), CP_COLOR_GOLD), kGRLEnableRLED);

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(110,36).GetVShifted(110).GetHShifted(200), kSideDisableR,"" , styleTogButts, "Right/Side IN", "Right/Side IN"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(78).GetHShifted(202), CP_COLOR_GOLD), kSideEnableRLED);

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(60,36).GetVShifted(110).GetHShifted(-50), kLinkSides,"" , styleTogButts, "Link", "Link"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(78).GetHShifted(-48), CP_COLOR_GOLD), kLinkModeLED);

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(75,36).GetVShifted(-220).GetHShifted(345), kBypass,"" , styleTogButts, "Bypass", "Bypass"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(-252).GetHShifted(347), COLOR_RED), kBypassIndicatorLED);

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(90,36).GetVShifted(147).GetHShifted(345), kLimiter, "", styleTogButts, "Soft Clip","Soft Clip"));
    
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(115).GetHShifted(347), CP_COLOR_GOLD), kLimiterLED);
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(173).GetHShifted(347), COLOR_RED), kClipIndicatorLED);

      
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(190).GetHShifted(-200), kHighpassSC, "SC Filter", styleKnobs,true));  
    
    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(90,36).GetVShifted(205).GetHShifted(-100),kSidechainOn,"" , styleTogButts, "Ext. SC", "Ext. SC"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(173).GetHShifted(-98), CP_COLOR_GOLD), kSidechainLED);

      
    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(90,36).GetVShifted(205).GetHShifted(0),kMidSideMode,"" , styleTogButts, "Mid/Side", "Mid/Side"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(173).GetHShifted(2), CP_COLOR_GOLD), kMidSideLED);

    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(190).GetHShifted(100), kMainOutput, "Output", styleKnobs,true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(190).GetHShifted(200), kDryWet, "Dry-Wet", styleKnobs,true));


    pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(b.GetCentredInside(300,45).GetVShifted(-240).GetHShifted(-50), "Input", styleInMeter, EDirection::Horizontal, emptyTrackNames, 0, -50.f, 0.f, custom_markers), kInMeter);


    pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(b.GetCentredInside(300,45).GetVShifted(-160).GetHShifted(-50), "Output", styleOutMeter, EDirection::Horizontal, emptyTrackNames, 0, -50.f, 0.f, custom_markers), kOutputMeter);

    pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(b.GetCentredInside(300,45).GetVShifted(-80).GetHShifted(-50), "Gain Reduction", styleGRMeter, EDirection::Horizontal, emptyTrackNames, 0, -50.f, -24.f, custom_markers2), kGRMeter);

    updateLED();

    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50).GetHShifted(-150), "", styleKnobs.labelText), kTextCtrl);


    auto pressureaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox( "Pressure", "Pressure determines at what point the compressor activates, ie the threshold", pGraphics);
    };

  auto speedaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Speed", "Speed controls how fast the compressor acts, both attack and release times", pGraphics);
    };

  auto mewaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Mewiness", "Mewiness manages the way that the ratio engages, with high levels giving vari-mu like behaviour", pGraphics);
    };

  auto pawaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("PawClaw", "PawClaw controls how mewiness acts on transients, the claw end clamps down and the paw end lets them through more", pGraphics);
    };

  auto mkgainaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Makeup gain", "Makeup gain is the gain applied after any compression has been done", pGraphics);
    };

  auto grlaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Gain Reduction Limit", "The gain reduction limit causes the compressor to have a cap on how much gain reduction it will do.\nA zero value will cause the gain reduction limit to be disabled.", pGraphics);
    };

  auto schpfaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Sidechain Filter", "This is a high pass filter controlling how the detector responds to low frequencies.\nThis filter is active in either external or internal sidechain mode ,but disabled at the 30Hz setting", pGraphics);
    };

  auto mainoutaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Output", "Output controls the main (wet) output level, prior to the dry/wet mix setting", pGraphics);
    };

  auto drywetaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Dry-Wet", "Dry wet controls the balance between the dry input signal and the wet output signal", pGraphics);
    };

  auto sidechaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("External Sidechain", "This switches compressor detection to use the external sidechain", pGraphics);
    };

  auto midsideaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Mid/Side", "This changes the channel mode to mid/side mode", pGraphics);
    };

  auto softclipaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Soft Clip", "Soft Clip activates a soft/safety clipper at the very end of the output chain.\nIf enabled it will be active at every left/right channel setting", pGraphics);
    };

  auto disabsideaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("", "This allows processing on this side to be enabled or disabled", pGraphics);
    };

  auto linkaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Link", "This causes the controls on each of the left and right (or mid and side) sides to be linked\nWhen first enabled, the current left side settings are copied to the right side", pGraphics);
    };

  auto bypassaction = [pGraphics](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        MessageBox("Bypass", "This disables all plugin processing, except for the input and output meters", pGraphics);
    };


    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-255).GetHShifted(-315), pressureaction, "?", styleHelpButts, true), kNoTag); // Pressure L
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-255).GetHShifted(185), pressureaction, "?", styleHelpButts, true), kNoTag); // Pressure R
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-255).GetHShifted(-217), speedaction, "?", styleHelpButts, true), kNoTag); // Speed L
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-255).GetHShifted(283), speedaction, "?", styleHelpButts, true), kNoTag); // Speed R
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-135).GetHShifted(-315), mewaction, "?", styleHelpButts, true), kNoTag); // Mewiness L
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-135).GetHShifted(185), mewaction, "?", styleHelpButts, true), kNoTag); // Mewiness R
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-135).GetHShifted(-217), pawaction, "?", styleHelpButts, true), kNoTag); // Pawclaw L
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-135).GetHShifted(283), pawaction, "?", styleHelpButts, true), kNoTag); // Pawclaw R
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-15).GetHShifted(-320), mkgainaction, "?", styleHelpButts, true), kNoTag); // Makeup Gain Left
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-15).GetHShifted(180), mkgainaction, "?", styleHelpButts, true), kNoTag); // Makeup Gain Right        
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-15).GetHShifted(-217), grlaction, "?", styleHelpButts, true), kNoTag); // GRL Left
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-15).GetHShifted(283), grlaction, "?", styleHelpButts, true), kNoTag); // GRL Right 
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(155).GetHShifted(-165), schpfaction, "?", styleHelpButts, true), kNoTag); // sidechain HPF
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(155).GetHShifted(129), mainoutaction, "?", styleHelpButts, true), kNoTag); // main output
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(155).GetHShifted(233), drywetaction, "?", styleHelpButts, true), kNoTag); // dry-wet
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(234).GetHShifted(-70), sidechaction, "?", styleHelpButts, true), kNoTag); // sidechain toggle 
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(234).GetHShifted(30), midsideaction, "?", styleHelpButts, true), kNoTag); // midside toggle
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(176).GetHShifted(375), softclipaction, "?", styleHelpButts, true), kNoTag); // soft clip
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(139).GetHShifted(-266), disabsideaction, "?", styleHelpButts, true), kNoTag); // left side disable
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(139).GetHShifted(255), disabsideaction, "?", styleHelpButts, true), kNoTag); // right side disable
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(139).GetHShifted(-35), linkaction, "?", styleHelpButts, true), kNoTag); // link
    pGraphics->AttachControl(new IVButtonControl(b.GetCentredInside(23).GetVShifted(-191).GetHShifted(365), bypassaction, "?", styleHelpButts, true), kNoTag); // bypass

    pGraphics->AttachControl(new GRMeterFrame(b.GetCentredInside(300,45).GetVShifted(-80).GetHShifted(-50)), kNoTag);

    pGraphics->AttachControl(new StdMeterFrame(b.GetCentredInside(300,45).GetVShifted(-160).GetHShifted(-50)), kNoTag);
    pGraphics->AttachControl(new StdMeterFrame(b.GetCentredInside(300,45).GetVShifted(-240).GetHShifted(-50)), kNoTag);

    pGraphics->AttachControl(new IVLabelControl (b.GetCentredInside(100,20).GetVShifted(-205).GetHShifted(-50), "Input", DEFAULT_STYLE.WithDrawFrame(false).WithDrawShadows(false).WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(CP_COLOR_GOLD).WithSize(16.))));
    pGraphics->AttachControl(new IVLabelControl (b.GetCentredInside(100,20).GetVShifted(-125).GetHShifted(-50), "Output", DEFAULT_STYLE.WithDrawFrame(false).WithDrawShadows(false).WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(CP_COLOR_GOLD).WithSize(16.))));
    pGraphics->AttachControl(new IVLabelControl (b.GetCentredInside(100,20).GetVShifted(-45).GetHShifted(-50), "Gain Reduction", DEFAULT_STYLE.WithDrawFrame(false).WithDrawShadows(false).WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(CP_COLOR_GOLD).WithSize(16.))));

    pGraphics->AttachControl(new IVLabelControl (b.GetCentredInside(200,45).GetVShifted(-15).GetHShifted(-50), "ComPressure", DEFAULT_STYLE.WithDrawFrame(false).WithDrawShadows(false).WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(COLOR_RED).WithSize(24.))));

    pGraphics->AttachControl(new IVLabelControl (b.GetCentredInside(100,20).GetVShifted(15).GetHShifted(-50), "v1.1 by", DEFAULT_STYLE.WithDrawFrame(false).WithDrawShadows(false).WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(COLOR_WHITE).WithSize(16.))));
    pGraphics->AttachControl(new IURLControl (b.GetCentredInside(100,20).GetVShifted(40).GetHShifted(-50), "pelennor DSP", "http://pelennordsp.com", DEFAULT_VALUE_TEXT.WithFGColor(COLOR_WHITE).WithSize(16.)));

 	//IURLControl (const IRECT &bounds, const char *str, const char *url, const IText &text=DEFAULT_TEXT, const IColor &BGColor=DEFAULT_BGCOLOR, const IColor &MOColor=COLOR_WHITE, const IColor &CLColor=COLOR_BLUE)

 };

}

void ComPressure::ResetMainSettings()
{
    // assume nFrames = 512, nChans = 2
  sample emptySplRawBuf[1024] = {0.0}; // an array of 1024 samples
  sample* emptySplBuf[2] = {&emptySplRawBuf[0], &emptySplRawBuf[512]};

  for (int x = 0; x < 2; x++)
  {
    pressureSettings[x].muSpeedA = 10000;
    pressureSettings[x].muSpeedB = 10000;
    pressureSettings[x].muCoefficientA = 1;
    pressureSettings[x].muCoefficientB = 1;
    pressureSettings[x].muVary = 1;
    pressureSettings[x].slewMax = 0.0;

    clipOnly2Settings[x].lastSample = 0.0;
    clipOnly2Settings[x].wasPosClip = false;
    clipOnly2Settings[x].wasNegClip = false;
    for (int y = 0; y < 16; y++) {clipOnly2Settings[x].intermediate[y] = 0.0;}

  }

  flip = false;

  for (int x = 0; x < fix_total; x++) {fixA[x] = 0.0; fixB[x] = 0.0;}

	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;

  filter.resetState();

}

void ComPressure::OnReset()
{
  
  mInMeterSender.Reset(GetSampleRate());
  mOutMeterSender.Reset(GetSampleRate());
  mGRMeterSender.Reset(GetSampleRate());

  ResetMainSettings();

  clipIndicator = false;
  updateLED();

}

void ComPressure::OnActivate(bool active)
{
    
  mInMeterSender.Reset(GetSampleRate());
  mOutMeterSender.Reset(GetSampleRate());
  mGRMeterSender.Reset(GetSampleRate());

  ResetMainSettings();

  clipIndicator = false;
  updateLED();

}

void ComPressure::OnIdle()
{
    // if(GetUI()) {
    //   GetUI()->GetControlWithTag(kTextCtrl)->As<ITextControl>()->SetStrFmt(64, "GRL: %f GRR: %f GRT: %f", debug1, debug2, debug3);
    // }
    mOutMeterSender.TransmitData(*this);
    mInMeterSender.TransmitData(*this);
    mGRMeterSender.TransmitData(*this);
}

void ComPressure::OnParamChange(int paramIdx) {

    bool updateLEDs = false;
    bool updateParms = false;

    m_ParamValues[paramIdx] = GetParam(paramIdx)->Value();

    switch (paramIdx)
    {

        case kLinkSides:
          sidesLinked = m_ParamValues[paramIdx];
            
          if (sidesLinked)
          {
              GetParam(kPressureR)->Set(GetParam(kPressureL)->Value());
              GetParam(kSpeedR)->Set(GetParam(kSpeedL)->Value());
              GetParam(kMewinessR)->Set(GetParam(kMewinessL)->Value());
              GetParam(kPawClawR)->Set(GetParam(kPawClawL)->Value());
              GetParam(kMakeupGainR)->Set(GetParam(kMakeupGainL)->Value());
              GetParam(kMaxGainReductR)->Set(GetParam(kMaxGainReductL)->Value());
              updateParms = true;
          }
          ResetMainSettings();
          break;

        case kLimiter:
            clipIndicator = false;
            clipperIsOn = m_ParamValues[paramIdx];
            updateLEDs = true;
            break;
        
        case kBypass:
            bypassed = m_ParamValues[paramIdx];
            //mOutMeterSender.Reset(GetSampleRate());
            updateLEDs = true;
            ResetMainSettings();
            break;

        case kSideDisableL:
            sideEnabledL = !m_ParamValues[paramIdx];
            break;

        case kSideDisableR:
            sideEnabledR = !m_ParamValues[paramIdx];
            break;

        case kPressureL:
            if (sidesLinked)
              GetParam(kPressureR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kPressureR:
          if (sidesLinked)
              GetParam(kPressureL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kSpeedL:
            if (sidesLinked)
              GetParam(kSpeedR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kSpeedR:
          if (sidesLinked)
              GetParam(kSpeedL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kMewinessL:
            if (sidesLinked)
              GetParam(kMewinessR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kMewinessR:
          if (sidesLinked)
              GetParam(kMewinessL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kPawClawL:
            if (sidesLinked)
              GetParam(kPawClawR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kPawClawR:
          if (sidesLinked)
              GetParam(kPawClawL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kMakeupGainL:
            if (sidesLinked)
              GetParam(kMakeupGainR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kMakeupGainR:
          if (sidesLinked)
              GetParam(kMakeupGainL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kMaxGainReductL:
            if (GetParam(kMaxGainReductL)->Value() > GetParam(kMaxGainReductL)->GetMin())
                GRLimitedL = true;
            else
                GRLimitedL = false;

            if (sidesLinked)
            {
              GetParam(kMaxGainReductR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              GRLimitedR = GRLimitedL;
            }
            break;

        case kMaxGainReductR:

          if (GetParam(kMaxGainReductR)->Value() > GetParam(kMaxGainReductR)->GetMin())
              GRLimitedR = true;
          else
              GRLimitedR = false;

          if (sidesLinked)
          {
              GetParam(kMaxGainReductL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              GRLimitedL = GRLimitedR;
          }
          break;

        case kSidechainOn:
            sideChainOn = m_ParamValues[paramIdx];
            ResetMainSettings();
            break;

        case kHighpassSC:
            if (m_ParamValues[kHighpassSC] > GetParam(kHighpassSC)->GetMin())
                sideChainHPFOn = true;
            else 
                sideChainHPFOn = false;
            break;

        case kMidSideMode:
            ResetMainSettings();
            break;
            
      
      
         
    }

    if (updateParms)
    {
      SendCurrentParamValuesFromDelegate();
    }

    if (updateLEDs)
    {
        updateLED();
    }


}

double ComPressure::calcGainRedCoeff(double inputSense, pressureVars& ps)
{
    inputSense = fabs(inputSense);
    double mewiness = sin(ps.mewiness + (ps.slewMax * ps.pawClaw));
		bool positivemu = true; if (mewiness < 0) {positivemu = false; mewiness = -mewiness;}
		
		if (flip)
		{
			if (inputSense > ps.threshold)
			{
				ps.muVary = ps.threshold / inputSense;
				ps.muAttack = sqrt(fabs(ps.muSpeedA));
				ps.muCoefficientA *= (ps.muAttack-1.0);
				if (ps.muVary < ps.threshold)
				{
					ps.muCoefficientA += ps.threshold;
				}
				else
				{
					ps.muCoefficientA += ps.muVary;
				}
				ps.muCoefficientA /= ps.muAttack;
			}
			else
			{
				ps.muCoefficientA *= ((ps.muSpeedA * ps.muSpeedA)-1.0);
				ps.muCoefficientA += 1.0;
				ps.muCoefficientA /= (ps.muSpeedA * ps.muSpeedA);
			}
      ps.muNewSpeed = ps.muSpeedA * (ps.muSpeedA-1);
			ps.muNewSpeed += fabs(inputSense*ps.release)+ps.fastest;
			ps.muSpeedA = ps.muNewSpeed / ps.muSpeedA;
		}
		else
		{
			if (inputSense > ps.threshold)
			{
				ps.muVary = ps.threshold / inputSense;
				ps.muAttack = sqrt(fabs(ps.muSpeedB));
				ps.muCoefficientB *= (ps.muAttack-1);
				if (ps.muVary < ps.threshold)
				{
					ps.muCoefficientB += ps.threshold;
				}
				else
				{
					ps.muCoefficientB += ps.muVary;
				}
				ps.muCoefficientB /= ps.muAttack;
			}
			else
			{
				ps.muCoefficientB *= ((ps.muSpeedB * ps.muSpeedB)-1.0);
				ps.muCoefficientB += 1.0;
				ps.muCoefficientB /= (ps.muSpeedB * ps.muSpeedB);
			}
			ps.muNewSpeed = ps.muSpeedB * (ps.muSpeedB-1);
			ps.muNewSpeed += fabs(inputSense*ps.release)+ps.fastest;
      ps.muSpeedB = ps.muNewSpeed / ps.muSpeedB;
		}
		//got coefficients, adjusted speeds
		
		double coefficient;
		if (flip) {
			if (positivemu) coefficient = pow(ps.muCoefficientA,2);
			else coefficient = sqrt(ps.muCoefficientA);
			coefficient = (coefficient*mewiness)+(ps.muCoefficientA*(1.0-mewiness));
		} else {
			if (positivemu) coefficient = pow(ps.muCoefficientB,2);
			else coefficient = sqrt(ps.muCoefficientB);
			coefficient = (coefficient*mewiness)+(ps.muCoefficientB*(1.0-mewiness));
			
		}  
    return coefficient;
}

void ComPressure::ultrasonicFilter(double& spL, double& spR, double* fix)
{
		if (fix[fix_freq] < 0.4999) {
			double temp = (spL * fix[fix_a0]) + fix[fix_sL1];
			fix[fix_sL1] = (spL * fix[fix_a1]) - (temp * fix[fix_b1]) + fix[fix_sL2];
			fix[fix_sL2] = (spL * fix[fix_a2]) - (temp * fix[fix_b2]);
			spL = temp;
			temp = (spR * fix[fix_a0]) + fix[fix_sR1];
			fix[fix_sR1] = (spR * fix[fix_a1]) - (temp * fix[fix_b1]) + fix[fix_sR2];
			fix[fix_sR2] = (spR * fix[fix_a2]) - (temp * fix[fix_b2]);
			spR = temp; //fixed biquad filtering ultrasonics before Pressure
		}  
}

bool ComPressure::clipOnly2(double& spl, struct clipOnly2Vars& cov, int spacing)
{
      if (spl > 4.0) spl = 4.0; if (spl < -4.0) spl = -4.0;
      if (cov.wasPosClip == true) { //current will be over
          if (spl<cov.lastSample) cov.lastSample=0.7058208+(spl*0.2609148);
          else cov.lastSample = 0.2491717+(cov.lastSample*0.7390851);
      } cov.wasPosClip = false;
      if (spl>0.9549925859) {cov.wasPosClip=true;spl=0.7058208+(cov.lastSample*0.2609148);}
      if (cov.wasNegClip == true) { //current will be -over
          if (spl > cov.lastSample) cov.lastSample=-0.7058208+(spl*0.2609148);
          else cov.lastSample=-0.2491717+(cov.lastSample*0.7390851);
      } cov.wasNegClip = false;
      if (spl<-0.9549925859) {cov.wasNegClip=true;spl=-0.7058208+(cov.lastSample*0.2609148);}
      cov.intermediate[spacing] = spl;
      spl = cov.lastSample; //Latency is however many samples equals one 44.1k sample
      for (int x = spacing; x > 0; x--) cov.intermediate[x-1] = cov.intermediate[x];
      cov.lastSample = cov.intermediate[0]; //run a little buffer to handle this
      
      if (cov.wasNegClip || cov.wasPosClip)
        return true;
      else
        return false;  
}



void ComPressure::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  double inputSampleL;
  double inputSampleR;
  double convSampleL;
  double convSampleR;
  double gainRedL;
  double gainRedR;
  double detectSampleL;
  double detectSampleR;
  double drySampleL;
	double drySampleR;

  // DB2Amp and Amp2DB

  bool sideChainDetect = false;

  // assume nFrames = 512, nChans = 2
  sample grRawBuf[1024] = {0.0}; // an array of 1024 samples
  sample* grBuf[2] = {&grRawBuf[0], &grRawBuf[512]};
  
  for (int i=0; i < 4; i++) {
    bool connected = IsChannelConnected(ERoute::kInput, i);
    if(connected != mInputChansConnected[i]) {
      mInputChansConnected[i] = connected;
    }
  }

  if (sideChainOn && mInputChansConnected[2] && mInputChansConnected[3])
    sideChainDetect = true;

  if (bypassed)
  {
    // In bypass mode, we pass the input signal to output untouched
    for (int c = 0; c < 2; c++)
    {
        for (int s = 0; s < nFrames; s++) 
        {
            outputs[c][s] = inputs[c][s];
        }
    }
    mOutMeterSender.ProcessBlock(outputs, nFrames, kOutputMeter);
    mInMeterSender.ProcessBlock(inputs, nFrames,kInMeter);
    return;
  }
    

  bool clippedThisBlock = false;

  double GRScale4Meter = DBToAmp(-24);

  double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	int spacing = floor(overallscale); //should give us working basic scaling, usually 2 or 4
	if (spacing < 1) spacing = 1; if (spacing > 16) spacing = 16;
  
  pressureSettings[0].threshold = 1.0 - (GetParam(kPressureL)->Value() * 0.0095);
  pressureSettings[1].threshold = 1.0 - (GetParam(kPressureR)->Value() * 0.0095);

	double muMakeupGainL = 1.0 / pressureSettings[0].threshold ;
	double muMakeupGainR = 1.0 / pressureSettings[1].threshold;
	//gain settings around threshold
	pressureSettings[0].release = pow((1.28-GetParam(kSpeedL)->Value()/100.),5)*32768.0;
	pressureSettings[1].release = pow((1.28-GetParam(kSpeedR)->Value()/100.),5)*32768.0;

pressureSettings[0].fastest = sqrt(pressureSettings[0].release);
pressureSettings[1].fastest = sqrt(pressureSettings[1].release);

pressureSettings[0].release /= overallscale;
pressureSettings[1].release /= overallscale;

pressureSettings[0].fastest /= overallscale;
pressureSettings[1].fastest /= overallscale;

	pressureSettings[0].mewiness = GetParam(kMewinessL)->Value()/100.;
	pressureSettings[1].mewiness= GetParam(kMewinessR)->Value()/100.;
  pressureSettings[0].pawClaw = -(GetParam(kPawClawL)->Value()/100. -0.5)*1.618033988749894848204586;
  pressureSettings[1].pawClaw = -(GetParam(kPawClawL)->Value()/100. -0.5)*1.618033988749894848204586;
	// µ µ µ µ µ µ µ µ µ µ µ µ is the kitten song o/~
	//double outputGain = pow(GetParam(kOutputLevel)->Value()*2.0,2); //max 4.0 gain
  double outputGainL = DBToAmp(GetParam(kMakeupGainL)->Value());
  double outputGainR = DBToAmp(GetParam(kMakeupGainR)->Value());
	double wet = GetParam(kDryWet)->Value() / 100;
  
  double maxGainRedCoeffL; 
  double maxGainRedCoeffR;

  if (GRLimitedL)
      maxGainRedCoeffL = 1.0 / DBToAmp(GetParam(kMaxGainReductL)->Value());
  if (GRLimitedR)
      maxGainRedCoeffR = 1.0 / DBToAmp(GetParam(kMaxGainReductR)->Value());

  bool midSideMode = GetParam(kMidSideMode)->Value();
  
  double coefficient;
	
	fixA[fix_freq] = 24000.0 / GetSampleRate();
  fixA[fix_reso] = 0.7071; //butterworth Q
	double K = tan(M_PI * fixA[fix_freq]);
	double norm = 1.0 / (1.0 + K / fixA[fix_reso] + K * K);
	fixA[fix_a0] = K * K * norm;
	fixA[fix_a1] = 2.0 * fixA[fix_a0];
	fixA[fix_a2] = fixA[fix_a0];
	fixA[fix_b1] = 2.0 * (K * K - 1.0) * norm;
	fixA[fix_b2] = (1.0 - K / fixA[fix_reso] + K * K) * norm;
	//for the fixed-position biquad filter
	for (int x = 0; x < fix_sL1; x++) fixB[x] = fixA[x];
	//make the second filter same as the first, don't use sample slots

  debug1 = 0.;
  debug2 = 0.;
  for (int s = 0; s < nFrames; s++) {
    inputSampleL = inputs[0][s];
    inputSampleR = inputs[1][s];
    drySampleL = inputs[0][s];;
		drySampleR = inputs[1][s];;


    if (sideChainDetect)
    {
        detectSampleL = inputs[2][s];
        detectSampleR = inputs[3][s];
    }
    else
    {
        detectSampleL = inputSampleL;
        detectSampleR = inputSampleR;
    }

    if (midSideMode)
    {
      // decode to mid/side
      convSampleL = 0.5 * (detectSampleL + detectSampleR);
      convSampleR = 0.5 * (detectSampleL - detectSampleR);
      detectSampleL = convSampleL;
      detectSampleR = convSampleR;
          
      convSampleL = 0.5 * (inputSampleL + inputSampleR);
      convSampleR = 0.5 * (inputSampleL - inputSampleR);
      inputSampleL = convSampleL;
      inputSampleR = convSampleR;
      
    }


   	if (fabs(detectSampleL)<1.18e-23) detectSampleL = fpdL * 1.18e-17;
		if (fabs(detectSampleR)<1.18e-23) detectSampleR = fpdR * 1.18e-17;

		detectSampleL *= muMakeupGainL;
		detectSampleR *= muMakeupGainR;
		

    ultrasonicFilter(detectSampleL, detectSampleR, fixA);

    if (sideChainHPFOn)
    {
        filter.updateCoefficients(GetParam(kHighpassSC)->Value(),0.5,SvfLinearTrapOptimised2::HIGH_PASS_FILTER,GetSampleRate());
        detectSampleL = filter.tick(detectSampleL);
        detectSampleR = filter.tick(detectSampleR);
     }


    coefficient = calcGainRedCoeff(detectSampleL, pressureSettings[0]);

    if (GRLimitedL)
        gainRedL = std::max(coefficient, maxGainRedCoeffL);
    else
        gainRedL = coefficient;
    
    debug1 += gainRedL;
   	inputSampleL *= gainRedL;
    

    coefficient = calcGainRedCoeff(detectSampleR, pressureSettings[1]);

    if (GRLimitedR)  
        gainRedR = std::max(coefficient, maxGainRedCoeffR);
    else
        gainRedR = coefficient;

    debug2 += gainRedR;
	  inputSampleR *= gainRedR;

    grBuf[0][s] = (drySampleL - inputSampleL) * (GRScale4Meter / drySampleL);
    grBuf[1][s] =  (drySampleR - inputSampleR) * (GRScale4Meter / drySampleR);


		//applied compression with vari-vari-µ-µ-µ-µ-µ-µ-is-the-kitten-song o/~
		//applied gain correction to control output level- tends to constrain sound rather than inflate it
		//mInMeterSender.PushData({kInMeter, {(float)gainRedL,(float)gainRedR}});
		if (outputGainL != 1.0) inputSampleL *= outputGainL;
    if (outputGainR != 1.0) inputSampleR *= outputGainR;
		
     flip = !flip;
		
    ultrasonicFilter(inputSampleL, inputSampleR, fixB);
		
     if (midSideMode)
    {
      // rencode mid/side to stereo
      convSampleL = inputSampleL + inputSampleR;
      convSampleR = inputSampleL - inputSampleR;
      inputSampleL = convSampleL;
      inputSampleR = convSampleR;
    }

    double finalOutputGain = DBToAmp(GetParam(kMainOutput)->Value());
    inputSampleL *= finalOutputGain;
    inputSampleR *= finalOutputGain;

		if (wet != 1.0) {
			inputSampleL = (inputSampleL * wet) + (drySampleL * (1.0-wet));
			inputSampleR = (inputSampleR * wet) + (drySampleR * (1.0-wet));
		}
		//Dry/Wet control, BEFORE ClipOnly
		
    if (!sideEnabledL)
    {
      inputSampleL = drySampleL;
    }

    if (!sideEnabledR)
    {
      inputSampleR = drySampleR;
    }
   		


    pressureSettings[0].slewMax = fabs(inputSampleL - lastSampleL);
    pressureSettings[1].slewMax = fabs(inputSampleR - lastSampleR);

    // Note we still want to apply clipping if sides are disabled, although the bypass button also bypasses the clipper.
    if (clipperIsOn)
    {
      clippedThisBlock =  clipOnly2(inputSampleL, clipOnly2Settings[0], spacing);
      clippedThisBlock = (clippedThisBlock || clipOnly2(inputSampleR, clipOnly2Settings[1], spacing));
    }

    outputs[0][s] = inputSampleL;
    outputs[1][s] = inputSampleR;
        
  }

  debug1 /= nFrames;
  debug2 /= nFrames;

  if (debug1 != 0.)
      debug1 = AmpToDB(1/debug1);
  if (debug2 != 0.)
      debug2 = AmpToDB(1/debug2);
  debug3 = debug1 + debug2;

  mOutMeterSender.ProcessBlock(outputs, nFrames, kOutputMeter);
  mInMeterSender.ProcessBlock(inputs, nFrames,kInMeter);
  mGRMeterSender.ProcessBlock(grBuf, nFrames, kGRMeter);

  if (clipperIsOn) {
      if (clippedThisBlock) {
          clipIndicator = true;
      }
      else {
          clipIndicator = false;
      }
      updateLED(); 
  }

}

void ComPressure::updateLED() 
{
  if (bypassed)
  {
      SendControlValueFromDelegate(kLimiterLED,false);
      SendControlValueFromDelegate(kClipIndicatorLED, false);
      SendControlValueFromDelegate(kBypassIndicatorLED,true);
      SendControlValueFromDelegate(kLinkModeLED, false);
      SendControlValueFromDelegate(kSidechainLED, false);
      SendControlValueFromDelegate(kMidSideLED,false);
      SendControlValueFromDelegate(kSideEnableLLED,false);
      SendControlValueFromDelegate(kSideEnableRLED,false);
      SendControlValueFromDelegate(kGRLEnableLLED,false);
      SendControlValueFromDelegate(kGRLEnableRLED,false);
  }
  else
  {
      SendControlValueFromDelegate(kLimiterLED,GetParam(kLimiter)->Value());
      SendControlValueFromDelegate(kClipIndicatorLED, clipIndicator);
      SendControlValueFromDelegate(kBypassIndicatorLED,false);
      SendControlValueFromDelegate(kLinkModeLED, GetParam(kLinkSides)->Value());
      SendControlValueFromDelegate(kSidechainLED, sideChainOn);
      SendControlValueFromDelegate(kMidSideLED,GetParam(kMidSideMode)->Value());
      SendControlValueFromDelegate(kSideEnableLLED,!GetParam(kSideDisableL)->Value());
      SendControlValueFromDelegate(kSideEnableRLED,!GetParam(kSideDisableR)->Value());
      SendControlValueFromDelegate(kGRLEnableLLED,GRLimitedL);
      SendControlValueFromDelegate(kGRLEnableRLED,GRLimitedR);
  }



}
