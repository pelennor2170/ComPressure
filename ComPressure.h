#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

const int kNumPresets = 1;

double muVaryL;
double muAttackL;
double muNewSpeedL;
double muSpeedAL;
double muSpeedBL;
double muCoefficientAL;
double muCoefficientBL;
bool flip; 
double muVaryR;
double muAttackR;
double muNewSpeedR;
double muSpeedAR;
double muSpeedBR;
double muCoefficientAR;
double muCoefficientBR;


double highpassSampleAA;
double highpassSampleAB;
double highpassSampleBA;
double highpassSampleBB;
double highpassSampleCA;
double highpassSampleCB;
double highpassSampleDA;
double highpassSampleDB;
double highpassSampleE;
double highpassSampleF; 

//Pressure

enum {
  fix_freq,
  fix_reso,
  fix_a0,
  fix_a1,
  fix_a2,
  fix_b1,
  fix_b2,
  fix_sL1,
  fix_sL2,
  fix_sR1,
  fix_sR2,
  fix_lastSampleL,
  fix_lastSampleR,
  fix_total
};
double fixA[fix_total];
double fixB[fix_total]; //fixed frequency biquad filter for ultrasonics, stereo

double lastSampleL;
double intermediateL[16];
bool wasPosClipL;
bool wasNegClipL;
double lastSampleR;
double intermediateR[16];
bool wasPosClipR;
bool wasNegClipR; //Stereo ClipOnly2

double slewMaxL; //to adust mewiness
double slewMaxR;

uint32_t fpdL;
uint32_t fpdR;
//default stuff


enum EParams
{
  //kGain = 0,
  kPressureL = 0,
  kSpeedL,
  kMewinessL,
  kPawClawL,
  kOutputLevelL,
  kMaxGainReductL,
  kPressureR,
  kSpeedR,
  kMewinessR,
  kPawClawR,
  kOutputLevelR,
  kMaxGainReductR,
  kDryWet,
  kSidechainOn,
  kHighpassSC,
  kMidSideMode,
  kOutputMeter,
  kGRMeter,
  kSideDisableL,
  kSideDisableR,
  kLinkSides,
  kBypass,
  kLimiter,
  kTestSwitch,
  kNumParams
};

enum EControlTags {
  kLimiterLED = 0,
  kClipIndicatorLED,
  kBypassIndicatorLED,
  kSidechainLED,
  kMidSideLED,
  kLinkModeLED,
  kSideEnableLLED,
  kSideEnableRLED,
  kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class ComPressure final : public Plugin
{
public:
  ComPressure(const InstanceInfo& info);
  double calcGainRedCoeffL(double inputSense, double mewinessRef, double pawsClaws, double threshold, double release, double fastest);
  double calcGainRedCoeffR(double inputSense, double mewinessRef, double pawsClaws, double threshold, double release, double fastest);
  double HighpassFilter(double inSample,double iirAmountD);
  void updateLED();

  void OnReset() override; 
  void OnIdle() override;
  void OnParamChange(int paramIdx) override;        

  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;

private:
  IPeakAvgSender<2> mMeterSender;
  IPeakAvgSender<2> mGRMeterSender;
  double                m_ParamValues[kNumParams];
  bool clipIndicator;
  bool clipperIsOn;
  bool sideEnabledL;
  bool sideEnabledR;
  bool sidesLinked;
  bool bypassed;
  bool sideChainOn;
  bool sideChainHPFOn;
  bool mInputChansConnected[4] = {};

};
