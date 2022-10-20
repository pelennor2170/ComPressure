#include "ComPressure.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

ComPressure::ComPressure(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kPressureL)->InitPercentage("Pressure", 0.);
  GetParam(kSpeedL)->InitPercentage("Speed", 25.);
  GetParam(kMewinessL)->InitPercentage("Mewiness", 100.);
  GetParam(kPawClawL)->InitPercentage("PawClaw", 50.);
  GetParam(kOutputLevelL)->InitGain("Makeup Gain", 0., 0., 18.0, 0.1);
  GetParam(kMaxGainReductL)->InitGain("GRLimit", 21, 0, 21, 0.1);

  GetParam(kPressureR)->InitPercentage("Pressure", 0.);
  GetParam(kSpeedR)->InitPercentage("Speed", 25.);
  GetParam(kMewinessR)->InitPercentage("Mewiness",100.);
  GetParam(kPawClawR)->InitPercentage("PawClaw", 50.);
  GetParam(kOutputLevelR)->InitGain("Makeup Gain", 0., 0., 18.0, 0.1);
  GetParam(kMaxGainReductR)->InitGain("GRLimit", 21, 0, 21, 0.1);


  GetParam(kDryWet)->InitPercentage("DryWet", 100.);
  GetParam(kMidSideMode)->InitBool("", false);
  GetParam(kSideDisableL)->InitBool("", false);
  GetParam(kSideDisableR)->InitBool("", false);
  GetParam(kSidechainOn)->InitBool("", false);
  GetParam(kHighpassSC)->InitFrequency("Highpass SC", 20., 20., 666., 1.);

  GetParam(kLinkSides)->InitBool("", false);

  GetParam(kBypass)->InitBool("", false);
  GetParam(kLimiter)->InitBool("", true);

  clipIndicator = false;
  clipperIsOn = true;
  sideEnabledL = true;
  sideEnabledR = true;
  bypassed = false;
  sideChainOn = false;
  sideChainHPFOn = false;


  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(IColor(255,35,35,35));
    //pGraphics->AttachBackground(BACKGROUND_FN);

    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->LoadFont("FreeSans", FONTTEST_FN);


    const IBitmap knobBitmap = pGraphics->LoadBitmap(KNOB_FN, 101);

      const ISVG powerOffSVG = pGraphics->LoadSVG(POWEROFF_FN);
      const ISVG powerOnSVG = pGraphics->LoadSVG(POWERON_FN);


    //const IBitmap knobTestBitmap = pGraphics->LoadBitmap(KNOB_TEST_FN, 101);

    //const IBitmap gainKnobBitmap = pGraphics->LoadBitmap(GAINKNOB_FN, 101);
    const IBitmap sideDisableBitmap = pGraphics->LoadBitmap(SIDESWITCH_FN, 2);
    const IBitmap switchBitmap = pGraphics->LoadBitmap(SWITCH_FN, 2);
    const IBitmap switchNoLedBitmap = pGraphics->LoadBitmap(SWITCH_NOLED_FN,2);
    const IBitmap bypassBitmap = pGraphics->LoadBitmap(SWITCH_BYPASS_FN, 2);

    std::initializer_list<int> custom_markers = {0, -6, -18, -30, -42};
    std::initializer_list<const char*> emptyTrackNames = {};
    

    const IRECT b = pGraphics->GetBounds();

    const IColor CP_COLOR_GOLD(255, 246,213,49);
    const IColor CP_COLOR_GREEN(255,78,143,0);
    const IColor CP_COLOR_ORANGE(255,255,147,0);

    //const IText valueCSK {12, CP_COLOR_GOLD, "FreeSans"};

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
      DEFAULT_LABEL_TEXT.WithFGColor(CP_COLOR_GOLD).WithFont("FreeSans"),
      DEFAULT_VALUE_TEXT.WithFGColor(CP_COLOR_GOLD).WithFont("FreeSans")
    };

    const IVStyle styleTogButts = styleKnobs.WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(CP_COLOR_GOLD).WithFont("FreeSans").WithSize(18.));

     const IVStyle styleOutMeter {
      true, // Show label
      true, // Show value
      {
        DEFAULT_BGCOLOR, // Background
        CP_COLOR_GREEN, // Foreground
        DEFAULT_PRCOLOR, // Pressed
        COLOR_GRAY, // Frame
        COLOR_WHITE, // Highlight
        COLOR_WHITE, // Shadow
        CP_COLOR_GOLD, // Extra 1
        DEFAULT_X2COLOR, // Extra 2
        DEFAULT_X3COLOR  // Extra 3
      }, // Colors
      DEFAULT_LABEL_TEXT.WithFGColor(CP_COLOR_GOLD).WithFont("FreeSans"),
      DEFAULT_VALUE_TEXT.WithFGColor(COLOR_WHITE).WithFont("FreeSans").WithSize(12)
    }; 

  const IVStyle styleGRMeter = styleOutMeter.WithColor(kFG, CP_COLOR_ORANGE);

    //const IVStyle styleOutMeter = DEFAULT_STYLE.WithColor(kFG, COLOR_GREEN).WithColor(kFR, COLOR_WHITE).WithColor(kHL, COLOR_WHITE).WithColor(kX1, CP_COLOR_GOLD);

    //const IVStyle knobWhiteText = DEFAULT_STYLE.WithColor(kFG, COLOR_WHITE.WithOpacity(0.1f));
    //pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello ComPressure!", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(-350), kPressureL, "Pressure", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(-250), kSpeedL, "Speed", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(-350), kMewinessL, "Mewiness", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(-250), kPawClawL, "PawClaw", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(-350), kOutputLevelL, "Makeup", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(-250), kMaxGainReductL, "GR Limit", styleKnobs));

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(60,36).GetVShifted(110).GetHShifted(-300), kSideDisableL,"" , styleTogButts, "IN", "IN"));


    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(150), kPressureR, "Pressure", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(250), kSpeedR, "Speed", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(150), kMewinessR, "Mewiness", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(-100).GetHShifted(250), kPawClawR, "PawClaw", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(150), kOutputLevelR, "Makeup", styleKnobs));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(20).GetHShifted(250), kMaxGainReductR, "GR Limit", styleKnobs));

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(60,36).GetVShifted(110).GetHShifted(200), kSideDisableR,"" , styleTogButts, "IN", "IN"));

    //pGraphics->AttachControl(new IBSwitchControl(b.GetCentredInside(90).GetVShifted(15).GetHShifted(-50), switchNoLedBitmap, kLinkSides));
    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(60,36).GetVShifted(110).GetHShifted(-50), kLinkSides,"" , styleTogButts, "Link", "Link"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(78).GetHShifted(-48), CP_COLOR_GOLD), kLinkModeLED);

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(60,36).GetVShifted(-220).GetHShifted(345), kBypass,"" , styleTogButts, "Bypass", "Bypass"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(-252).GetHShifted(347), COLOR_RED), kBypassIndicatorLED);

    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(90,36).GetVShifted(147).GetHShifted(345), kLimiter, "", styleTogButts, "Soft Clip","Soft Clip"));
    
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(115).GetHShifted(347), CP_COLOR_GOLD), kLimiterLED);
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(173).GetHShifted(347), COLOR_RED), kClipIndicatorLED);

      
    //pGraphics->AttachControl(new IBKnobControl(b.GetCentredInside(75).GetVShifted(190).GetHShifted(-200), knobBitmap, kDryWet));
    const IColor charcoal(255,30,30,30);

    //pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(86).GetVShifted(180).GetHShifted(-200), kHighpassSC, "", DEFAULT_STYLE.WithColor(kFG, charcoal).WithColor(kFR, COLOR_WHITE).WithColor(kX1, COLOR_YELLOW).WithColor(kPR, COLOR_DARK_GRAY).WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(COLOR_YELLOW)), true));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(190).GetHShifted(-200), kHighpassSC, "SC Filter", styleKnobs));  
    
    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(90,36).GetVShifted(205).GetHShifted(-100),kSidechainOn,"" , styleTogButts, "Sidechain", "Sidechain"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(173).GetHShifted(-98), CP_COLOR_GOLD), kSidechainLED);

      
    pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(90,36).GetVShifted(205).GetHShifted(0),kMidSideMode,"" , styleTogButts, "Mid / Side", "Mid / Side"));
    pGraphics->AttachControl(new ILEDControl(b.GetCentredInside(25).GetVShifted(173).GetHShifted(2), CP_COLOR_GOLD), kMidSideLED);

    //pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(190).GetHShifted(100), kDryWet, "Dry / Wet", DEFAULT_STYLE.WithColor(kFG, COLOR_BLACK).WithColor(kFR, IColor(255, 246,213,49)).WithColor(kX1, COLOR_WHITE).WithColor(kPR, COLOR_DARK_GRAY).WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(IColor(255, 246,213,49))).WithLabelText(DEFAULT_LABEL_TEXT.WithFGColor(IColor(255, 246,213,49)))));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(90).GetVShifted(190).GetHShifted(100), kDryWet, "Dry / Wet", styleKnobs));
    //GetParam(kDryWet)->Set(100.);
    //SendCurrentParamValuesFromDelegate();



    //pGraphics->AttachControl(new IVToggleControl(b.GetCentredInside(50,36).GetVShifted(-220).GetHShifted(0), kTestSwitch, "", styleTogButts, "Link", "Link"));
    //pGraphics->AttachControl(new IVSwitchControl(b.GetCentredInside(90).GetVShifted(-220).GetHShifted(0), kLinkSides, "Link", styleKnobs));

    /* auto button1action = [pGraphics](IControl* pCaller) {
      
    };

    pGraphics->AttachControl(new ISVGButtonControl(b.GetCentredInside(40).GetVShifted(-220).GetHShifted(0), button1action, powerOffSVG, powerOnSVG));


     */
    //pGraphics->AttachControl(new IVLEDMeterControl<2>(b.GetCentredInside(100).GetVShifted(-200)), kOutputMeter);

    //pGraphics->AttachControl(new IVDisplayControl(b.GetCentredInside(45,180).GetVShifted(-160).GetHShifted(-148), "IVDisplayControl", DEFAULT_STYLE.WithColor(kFG, COLOR_ORANGE), EDirection::Horizontal, 0., 15., 0., 512), kGRMeter);
 
    pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(b.GetCentredInside(300,45).GetVShifted(-210).GetHShifted(-50), "Gain Reduction", styleGRMeter, EDirection::Horizontal, emptyTrackNames, 0, -50.f, 0.f, custom_markers), kGRMeter);


    pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(b.GetCentredInside(300,45).GetVShifted(-120).GetHShifted(-50), "Output", styleOutMeter, EDirection::Horizontal, emptyTrackNames, 0, -50.f, 0.f, custom_markers), kOutputMeter);

//  IVMeterControl(const IRECT& bounds, const char* label, const fgu8i96& style = DEFAULT_STYLE, EDirection dir = EDirection::Vertical, std::initializer_list<const char*> trackNames = {}, int totalNSegs = 0, 
// EResponse response = EResponse::Linear, float lowRangeDB = -72.f, float highRangeDB = 12.f, std::initializer_list<int> markers = {0, -6, -12, -24, -48})


    //pGraphics->AttachControl(new IVMeterControl<2>(b.GetCentredInside(45,180).GetVShifted(-160).GetHShifted(-148), "", DEFAULT_STYLE.WithColor(kFG, COLOR_ORANGE)), kGRMeter);
    // IVDisplayControl (const IRECT &bounds, const char *label="", const IVStyle &style=DEFAULT_STYLE, EDirection dir=EDirection::Horizontal, float lo=0., float hi=1.f, float defaultVal=0., uint32_t bufferSize=100, float strokeThickness=2.f)
 
   // pGraphics->AttachControl(new IVDisplayControl<2>(b.GetCentredInside(45,180).GetVShifted(-160).GetHShifted(-148),"", DEFAULT_STYLE.WithColor(kFG, COLOR_ORANGE), EDirection::Vertical, 0., 15., 0., 512), kGRMeter);
    
      
    // pGraphics->AttachControl(new IVDisplayControl(nextCell(), "IVDisplayControl", style, EDirection::Horizontal, -1., 1., 0., 512), kCtrlTagDisplay, "vcontrols");
    
    updateLED();



    
  };

}

void ComPressure::OnReset()
{
  highpassSampleAA = 0.0;
	highpassSampleAB = 0.0;
	highpassSampleBA = 0.0;
	highpassSampleBB = 0.0;
	highpassSampleCA = 0.0;
	highpassSampleCB = 0.0;
	highpassSampleDA = 0.0;
	highpassSampleDB = 0.0;
	highpassSampleE = 0.0;
	highpassSampleF = 0.0;

  muSpeedAL = 10000;
  muSpeedAR = 10000;
	muSpeedBL = 10000;
	muSpeedBR = 10000;
	muCoefficientAL = 1;
	muCoefficientAR = 1;
	muCoefficientBL = 1;
	muCoefficientBR = 1;
	muVaryL = 1;
  muVaryR = 1;
	flip = false;
  for (int x = 0; x < fix_total; x++) {fixA[x] = 0.0; fixB[x] = 0.0;}
	lastSampleL = 0.0;
  lastSampleR = 0.0;
	wasPosClipL = false;
	wasNegClipL = false;
	wasPosClipR = false;
	wasNegClipR = false;
	for (int x = 0; x < 16; x++) {intermediateL[x] = 0.0; intermediateR[x] = 0.0;}
	slewMaxL = 0.0;
  slewMaxR = 0.0;

	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	//this is reset: values being initialized only once. Startup values, whatever they are.

  clipIndicator = false;
  updateLED();
  


  mMeterSender.Reset(GetSampleRate());
}

void ComPressure::OnIdle()
{
    mMeterSender.TransmitData(*this);
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
              GetParam(kOutputLevelR)->Set(GetParam(kOutputLevelL)->Value());
              GetParam(kMaxGainReductR)->Set(GetParam(kMaxGainReductL)->Value());
              updateParms = true;
          }
          break;

        case kLimiter:
            clipIndicator = false;
            clipperIsOn = m_ParamValues[paramIdx];
            updateLEDs = true;
            break;
        
        case kBypass:
            bypassed = m_ParamValues[paramIdx];
            //mMeterSender.Reset(GetSampleRate());
            updateLEDs = true;
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

      case kOutputLevelL:
            if (sidesLinked)
              GetParam(kOutputLevelR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kOutputLevelR:
          if (sidesLinked)
              GetParam(kOutputLevelL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

      case kMaxGainReductL:
            if (sidesLinked)
              GetParam(kMaxGainReductR)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kMaxGainReductR:
          if (sidesLinked)
              GetParam(kMaxGainReductL)->Set(m_ParamValues[paramIdx]);
              updateParms = true;
              break;

        case kSidechainOn:
            sideChainOn = m_ParamValues[paramIdx];
            break;

        case kHighpassSC:
            if (m_ParamValues[kHighpassSC] > GetParam(kHighpassSC)->GetMin())
                sideChainHPFOn = true;
            else 
                sideChainHPFOn = false;
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


double ComPressure::calcGainRedCoeffL(double inputSense, double mewinessRef, double pawsClaws, double threshold, double release, double fastest)
{
    double mewiness = sin(mewinessRef + (slewMaxL * pawsClaws));
		bool positivemu = true; if (mewiness < 0) {positivemu = false; mewiness = -mewiness;}
		
		if (flip)
		{
			if (inputSense > threshold)
			{
				muVaryL = threshold / inputSense;
				muAttackL = sqrt(fabs(muSpeedAL));
				muCoefficientAL = muCoefficientAL * (muAttackL-1.0);
				if (muVaryL < threshold)
				{
					muCoefficientAL = muCoefficientAL + threshold;
				}
				else
				{
					muCoefficientAL = muCoefficientAL + muVaryL;
				}
				muCoefficientAL = muCoefficientAL / muAttackL;
			}
			else
			{
				muCoefficientAL = muCoefficientAL * ((muSpeedAL * muSpeedAL)-1.0);
				muCoefficientAL = muCoefficientAL + 1.0;
				muCoefficientAL = muCoefficientAL / (muSpeedAL * muSpeedAL);
			}
			muNewSpeedL = muSpeedAL * (muSpeedAL-1);
			muNewSpeedL = muNewSpeedL + fabs(inputSense*release)+fastest;
			muSpeedAL = muNewSpeedL / muSpeedAL;
		}
		else
		{
			if (inputSense > threshold)
			{
				muVaryL = threshold / inputSense;
				muAttackL = sqrt(fabs(muSpeedBL));
				muCoefficientBL = muCoefficientBL * (muAttackL-1);
				if (muVaryL < threshold)
				{
					muCoefficientBL = muCoefficientBL + threshold;
				}
				else
				{
					muCoefficientBL = muCoefficientBL + muVaryL;
				}
				muCoefficientBL = muCoefficientBL / muAttackL;
			}
			else
			{
				muCoefficientBL = muCoefficientBL * ((muSpeedBL * muSpeedBL)-1.0);
				muCoefficientBL = muCoefficientBL + 1.0;
				muCoefficientBL = muCoefficientBL / (muSpeedBL * muSpeedBL);
			}
			muNewSpeedL = muSpeedBL * (muSpeedBL-1);
			muNewSpeedL = muNewSpeedL + fabs(inputSense*release)+fastest;
            muSpeedBL = muNewSpeedL / muSpeedBL;
		}
		//got coefficients, adjusted speeds
		
		double coefficient;
		if (flip) {
			if (positivemu) coefficient = pow(muCoefficientAL,2);
			else coefficient = sqrt(muCoefficientAL);
			coefficient = (coefficient*mewiness)+(muCoefficientAL*(1.0-mewiness));
			//inputSampleL *= coefficient;
			//inputSampleR *= coefficient;
		} else {
			if (positivemu) coefficient = pow(muCoefficientBL,2);
			else coefficient = sqrt(muCoefficientBL);
			coefficient = (coefficient*mewiness)+(muCoefficientBL*(1.0-mewiness));
			
		}  
    return coefficient;
}

double ComPressure::calcGainRedCoeffR(double inputSense, double mewinessRef, double pawsClaws, double threshold, double release, double fastest)
{
    double mewiness = sin(mewinessRef + (slewMaxR * pawsClaws));
		bool positivemu = true; if (mewiness < 0) {positivemu = false; mewiness = -mewiness;}
		
		if (flip)
		{
			if (inputSense > threshold)
			{
				muVaryR = threshold / inputSense;
				muAttackR = sqrt(fabs(muSpeedAR));
				muCoefficientAR = muCoefficientAR * (muAttackR-1.0);
				if (muVaryR < threshold)
				{
					muCoefficientAR = muCoefficientAR + threshold;
				}
				else
				{
					muCoefficientAR = muCoefficientAR + muVaryR;
				}
				muCoefficientAR = muCoefficientAR / muAttackR;
			}
			else
			{
				muCoefficientAR = muCoefficientAR * ((muSpeedAR * muSpeedAR)-1.0);
				muCoefficientAR = muCoefficientAR + 1.0;
				muCoefficientAR = muCoefficientAR / (muSpeedAR * muSpeedAR);
			}
			muNewSpeedR = muSpeedAR * (muSpeedAR-1);
			muNewSpeedR = muNewSpeedR + fabs(inputSense*release)+fastest;
			muSpeedAR = muNewSpeedR / muSpeedAR;
		}
		else
		{
			if (inputSense > threshold)
			{
				muVaryR = threshold / inputSense;
				muAttackR = sqrt(fabs(muSpeedBR));
				muCoefficientBR = muCoefficientBR * (muAttackR-1);
				if (muVaryR < threshold)
				{
					muCoefficientBR = muCoefficientBR + threshold;
				}
				else
				{
					muCoefficientBR = muCoefficientBR + muVaryR;
				}
				muCoefficientBR = muCoefficientBR / muAttackR;
			}
			else
			{
				muCoefficientBR = muCoefficientBR * ((muSpeedBR * muSpeedBR)-1.0);
				muCoefficientBR = muCoefficientBR + 1.0;
				muCoefficientBR = muCoefficientBR / (muSpeedBR * muSpeedBR);
			}
			muNewSpeedR = muSpeedBR * (muSpeedBR-1);
			muNewSpeedR = muNewSpeedR + fabs(inputSense*release)+fastest;
            muSpeedBR = muNewSpeedR / muSpeedBR;
		}
		//got coefficients, adjusted speeds
		
		double coefficient;
		if (flip) {
			if (positivemu) coefficient = pow(muCoefficientAR,2);
			else coefficient = sqrt(muCoefficientAR);
			coefficient = (coefficient*mewiness)+(muCoefficientAR*(1.0-mewiness));
			//inputSampleL *= coefficient;
			//inputSampleR *= coefficient;
		} else {
			if (positivemu) coefficient = pow(muCoefficientBR,2);
			else coefficient = sqrt(muCoefficientBR);
			coefficient = (coefficient*mewiness)+(muCoefficientBR*(1.0-mewiness));
			
		}  
    return coefficient;
}

double ComPressure::HighpassFilter(double inSample, double iirAmountD)
{
// iirAmountD = highPassFreq / overallscale



  if (flip)
  {
    highpassSampleAA = (highpassSampleAA * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleAA;
    highpassSampleBA = (highpassSampleBA * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleBA;
    highpassSampleCA = (highpassSampleCA * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleCA;
    highpassSampleDA = (highpassSampleDA * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleDA;
  }
  else
  {
    highpassSampleAB = (highpassSampleAB * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleAB;
    highpassSampleBB = (highpassSampleBB * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleBB;
    highpassSampleCB = (highpassSampleCB * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleCB;
    highpassSampleDB = (highpassSampleDB * (1.0 - iirAmountD)) + (inSample * iirAmountD);
    inSample -= highpassSampleDB;
  }
  highpassSampleE = (highpassSampleE * (1.0 - iirAmountD)) + (inSample * iirAmountD);
  inSample -= highpassSampleE;
  highpassSampleF = (highpassSampleF * (1.0 - iirAmountD)) + (inSample * iirAmountD);
  inSample -= highpassSampleF;			
        


  return inSample;
}

void ComPressure::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  double inputSampleL;
  double inputSampleR;
  double convSampleL;
  double convSampleR;
  double unprocInputSampleL;
  double unprocInputSampleR;
  double gainRedL;
  double gainRedR;

  double inputNonSCL;
  double inputNonSCR;

  double scHPF;
  bool sideChainDetect = false;

  const int nChans = NOutChansConnected();
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
    mMeterSender.ProcessBlock(outputs, nFrames, kOutputMeter);
    return;
  }
    

  bool clippedThisBlock = false;

  double overallscale = 1.0;
	overallscale /= 44100.0;
	overallscale *= GetSampleRate();
	int spacing = floor(overallscale); //should give us working basic scaling, usually 2 or 4
	if (spacing < 1) spacing = 1; if (spacing > 16) spacing = 16;
  
  double thresholdL = 1.0 - (GetParam(kPressureL)->Value() * 0.0095);
  double thresholdR = 1.0 - (GetParam(kPressureR)->Value() * 0.0095);
	double muMakeupGainL = 1.0 / thresholdL;
	double muMakeupGainR = 1.0 / thresholdR;
	//gain settings around threshold
	double releaseL = pow((1.28-GetParam(kSpeedL)->Value()/100.),5)*32768.0;
	double releaseR = pow((1.28-GetParam(kSpeedR)->Value()/100.),5)*32768.0;
	double fastestL = sqrt(releaseL);
	double fastestR = sqrt(releaseR);
	releaseL /= overallscale;
	releaseR /= overallscale;
	fastestL /= overallscale;
	fastestR /= overallscale;
	//speed settings around release
	double mewinessRefL = GetParam(kMewinessL)->Value()/100.;
	double mewinessRefR = GetParam(kMewinessR)->Value()/100.;
	double pawsClawsL = -(GetParam(kPawClawL)->Value()/100. -0.5)*1.618033988749894848204586;
	double pawsClawsR = -(GetParam(kPawClawL)->Value()/100. -0.5)*1.618033988749894848204586;
	// µ µ µ µ µ µ µ µ µ µ µ µ is the kitten song o/~
	//double outputGain = pow(GetParam(kOutputLevel)->Value()*2.0,2); //max 4.0 gain
  double outputGainL = pow(10.0, (GetParam(kOutputLevelL)->Value() / 20.0));
  double outputGainR = pow(10.0, (GetParam(kOutputLevelR)->Value() / 20.0));
	double wet = GetParam(kDryWet)->Value() / 100;

  double maxGainRedCoeffL = 1.0 / pow(10.0, (GetParam(kMaxGainReductL)->Value() / 20.0));
  double maxGainRedCoeffR = 1.0 / pow(10.0, (GetParam(kMaxGainReductR)->Value() / 20.0));
  bool midSideMode = GetParam(kMidSideMode)->Value();

  if (sideChainDetect)
      scHPF = GetParam(kHighpassSC)->Value() / overallscale;

  //bool ClipperIsOn = GetParam(kLimiter)->Value();

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

  for (int s = 0; s < nFrames; s++) {
    if (sideChainDetect)
    {
        inputNonSCL = inputs[0][s] * muMakeupGainL;
        inputNonSCR = inputs[1][s] * muMakeupGainR;
        inputSampleL = inputs[2][s];
        inputSampleR = inputs[3][s];
    }
    else
    {
        inputSampleL = inputs[0][s];
        inputSampleR = inputs[1][s];
    }

    unprocInputSampleL = inputSampleL;
    unprocInputSampleR = inputSampleR;

    if (midSideMode)
    {
      // decode to mid/side
      convSampleL = inputSampleL + inputSampleR;
      convSampleR = inputSampleL - inputSampleR;
      inputSampleL = convSampleL;
      inputSampleR = convSampleR;

      if (sideChainDetect)
      {
          convSampleL = inputNonSCL + inputNonSCR;
          convSampleR = inputNonSCL - inputNonSCR;
          inputNonSCL = convSampleL;
          inputNonSCR = convSampleR;
      }
    }


   	if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;
		double drySampleL = inputSampleL;
		double drySampleR = inputSampleR;

   /*  if (sideChainDetect && sideChainHPFOn)
    {

    
        inputSampleL = HighpassFilter(inputSampleL, scHPF);
        inputSampleR =  HighpassFilter(inputSampleR, scHPF);
    } */
		
		inputSampleL = inputSampleL * muMakeupGainL;
		inputSampleR = inputSampleR * muMakeupGainR;
		
		if (fixA[fix_freq] < 0.4999) {
			double temp = (inputSampleL * fixA[fix_a0]) + fixA[fix_sL1];
			fixA[fix_sL1] = (inputSampleL * fixA[fix_a1]) - (temp * fixA[fix_b1]) + fixA[fix_sL2];
			fixA[fix_sL2] = (inputSampleL * fixA[fix_a2]) - (temp * fixA[fix_b2]);
			inputSampleL = temp;
			temp = (inputSampleR * fixA[fix_a0]) + fixA[fix_sR1];
			fixA[fix_sR1] = (inputSampleR * fixA[fix_a1]) - (temp * fixA[fix_b1]) + fixA[fix_sR2];
			fixA[fix_sR2] = (inputSampleR * fixA[fix_a2]) - (temp * fixA[fix_b2]);
			inputSampleR = temp; //fixed biquad filtering ultrasonics before Pressure
		}
		
		
    coefficient = calcGainRedCoeffL(inputSampleL, mewinessRefL, pawsClawsL, thresholdL, releaseL, fastestL);
    coefficient = calcGainRedCoeffR(inputSampleR, mewinessRefR, pawsClawsR, thresholdR, releaseR, fastestR);

    if (sideChainDetect)
    {
      inputSampleL = inputNonSCL;
      inputSampleR = inputNonSCR;
    }

    if (sideEnabledL)
    {
      gainRedL = std::max(coefficient, maxGainRedCoeffL);
   		inputSampleL *= gainRedL;
      gainRedL = AmpToDB(1/gainRedL);
    }
    else
      gainRedR = 0.;

    if (sideEnabledR)
    {
      gainRedR = std::max(coefficient, maxGainRedCoeffR);
   		inputSampleR *= gainRedR;
      gainRedR = AmpToDB(1/gainRedR);
    }
    else
      gainRedR = 0.;


    /* See Oli code for sample buffer management here.  Need to graph unProcInputSample - inputSample (note mid/side looks like it will have a bug)

// assume nFrames = 512, nChans = 2
sample smp[1024] = {0.0}; // an array of 1024 samples
sample* smpPtrs[2] = {&smp, &smp + 512}; // an array of two pointers to samples, pointing into smp

for (int s = 0; s < 512; s++) {
    for (int c = 0; c < 2; c++) {
        smpPtrs[c][s] = inputs[c][s]; // copy input sample
    }
}
    */

/*       if (thisGR != 0.)
          GRTotalR += AmpToDB(1/thisGR);

    } */


		//applied compression with vari-vari-µ-µ-µ-µ-µ-µ-is-the-kitten-song o/~
		//applied gain correction to control output level- tends to constrain sound rather than inflate it
		//mGRMeterSender.PushData({kGRMeter, {(float)gainRedL,(float)gainRedR}});
		if (outputGainL != 1.0) inputSampleL *= outputGainL;
    if (outputGainR != 1.0) inputSampleR *= outputGainR;
		
     flip = !flip;
		
		if (fixB[fix_freq] < 0.49999) {
			double temp = (inputSampleL * fixB[fix_a0]) + fixB[fix_sL1];
			fixB[fix_sL1] = (inputSampleL * fixB[fix_a1]) - (temp * fixB[fix_b1]) + fixB[fix_sL2];
			fixB[fix_sL2] = (inputSampleL * fixB[fix_a2]) - (temp * fixB[fix_b2]);
			inputSampleL = temp;
			temp = (inputSampleR * fixB[fix_a0]) + fixB[fix_sR1];
			fixB[fix_sR1] = (inputSampleR * fixB[fix_a1]) - (temp * fixB[fix_b1]) + fixB[fix_sR2];
			fixB[fix_sR2] = (inputSampleR * fixB[fix_a2]) - (temp * fixB[fix_b2]);
			inputSampleR = temp; //fixed biquad filtering ultrasonics between Pressure and ClipOnly
		}
		
 		inputSampleL /= muMakeupGainL;
		inputSampleR /=  muMakeupGainR;

		if (wet != 1.0) {
			inputSampleL = (inputSampleL * wet) + (drySampleL * (1.0-wet));
			inputSampleR = (inputSampleR * wet) + (drySampleR * (1.0-wet));
		}
		//Dry/Wet control, BEFORE ClipOnly
		
		slewMaxL = fabs(inputSampleL - lastSampleL);
		slewMaxR = fabs(inputSampleR - lastSampleR);

		//if (slewMaxL < fabs(inputSampleL - lastSampleL)) slewMaxL = fabs(inputSampleL - lastSampleL);
		//set up for fiddling with mewiness. Largest amount of slew in any direction

    if (midSideMode)
    {
      // rencode mid/side to stereo
      convSampleL = (inputSampleL + inputSampleR) / 2.0;
      convSampleR = (inputSampleL - inputSampleR) / 2.0;
      inputSampleL = convSampleL;
      inputSampleR = convSampleR;
    }

    if (!sideEnabledL)
    {
      inputSampleL = unprocInputSampleL;
    }

    if (!sideEnabledR)
    {
      inputSampleR = unprocInputSampleR;
    }

    // Note we still want to apply clipping if sides are disabled, although the bypass button also bypasses the clipper.
    if (clipperIsOn)
    {     

      //begin ClipOnly2 stereo as a little, compressed chunk that can be dropped into code
      if (inputSampleL > 4.0) inputSampleL = 4.0; if (inputSampleL < -4.0) inputSampleL = -4.0;
      if (wasPosClipL == true) { //current will be over
          if (inputSampleL<lastSampleL) lastSampleL=0.7058208+(inputSampleL*0.2609148);
          else lastSampleL = 0.2491717+(lastSampleL*0.7390851);
      } wasPosClipL = false;
      if (inputSampleL>0.9549925859) {wasPosClipL=true;inputSampleL=0.7058208+(lastSampleL*0.2609148);}
      if (wasNegClipL == true) { //current will be -over
          if (inputSampleL > lastSampleL) lastSampleL=-0.7058208+(inputSampleL*0.2609148);
          else lastSampleL=-0.2491717+(lastSampleL*0.7390851);
      } wasNegClipL = false;
      if (inputSampleL<-0.9549925859) {wasNegClipL=true;inputSampleL=-0.7058208+(lastSampleL*0.2609148);}
      intermediateL[spacing] = inputSampleL;
      inputSampleL = lastSampleL; //Latency is however many samples equals one 44.1k sample
      for (int x = spacing; x > 0; x--) intermediateL[x-1] = intermediateL[x];
      lastSampleL = intermediateL[0]; //run a little buffer to handle this
      
      if (inputSampleR > 4.0) inputSampleR = 4.0; if (inputSampleR < -4.0) inputSampleR = -4.0;
      if (wasPosClipR == true) { //current will be over
          if (inputSampleR<lastSampleR) lastSampleR=0.7058208+(inputSampleR*0.2609148);
          else lastSampleR = 0.2491717+(lastSampleR*0.7390851);
      } wasPosClipR = false;
      if (inputSampleR>0.9549925859) {wasPosClipR=true;inputSampleR=0.7058208+(lastSampleR*0.2609148);}
      if (wasNegClipR == true) { //current will be -over
          if (inputSampleR > lastSampleR) lastSampleR=-0.7058208+(inputSampleR*0.2609148);
          else lastSampleR=-0.2491717+(lastSampleR*0.7390851);
      } wasNegClipR = false;
      if (inputSampleR<-0.9549925859) {wasNegClipR=true;inputSampleR=-0.7058208+(lastSampleR*0.2609148);}
      intermediateR[spacing] = inputSampleR;
      inputSampleR = lastSampleR; //Latency is however many samples equals one 44.1k sample
      for (int x = spacing; x > 0; x--) intermediateR[x-1] = intermediateR[x];
      lastSampleR = intermediateR[0]; //run a little buffer to handle this
      //end ClipOnly2 stereo as a little, compressed chunk that can be dropped into code
        
      if (wasNegClipL || wasNegClipR || wasPosClipL || wasPosClipR)
      {
          clippedThisBlock = true;
      }
    }

    

    outputs[0][s] = inputSampleL;
    outputs[1][s] = inputSampleR;
        
  }

  mMeterSender.ProcessBlock(outputs, nFrames, kOutputMeter);
  mGRMeterSender.ProcessBlock(outputs, nFrames,kGRMeter);
  /* GRTotalL /= nFrames;
  GRTotalR /= nFrames;  */

  //mGRMeterSender.PushData({kGRMeter, {(float)GRTotalL,(float)GRTotalR}});

  //mMeterSender.ProcessBlock(outputs, nFrames, kGRMeter);

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
  }
  else
  {
      SendControlValueFromDelegate(kLimiterLED,GetParam(kLimiter)->Value());
      SendControlValueFromDelegate(kClipIndicatorLED, clipIndicator);
      SendControlValueFromDelegate(kBypassIndicatorLED,false);
      SendControlValueFromDelegate(kLinkModeLED, GetParam(kLinkSides)->Value());
      SendControlValueFromDelegate(kSidechainLED, sideChainOn);
      SendControlValueFromDelegate(kMidSideLED,GetParam(kMidSideMode)->Value());

  }



}




