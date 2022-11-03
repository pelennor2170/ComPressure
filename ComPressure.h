#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "SvfLinearTrapOptimised2.hpp"

const int kNumPresets = 1;

bool flip; 

struct pressureVars {
    double muVary;
    double muAttack;
    double muNewSpeed;
    double muSpeedA;
    double muSpeedB;
    double muCoefficientA;
    double muCoefficientB;
    double slewMax;
    double mewiness;
    double pawClaw;
    double threshold;
    double release;
    double fastest;
};

struct pressureVars pressureSettings[2];


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

struct clipOnly2Vars {
  double lastSample;
  double intermediate[16];
  bool wasPosClip;
  bool wasNegClip;
};

struct clipOnly2Vars clipOnly2Settings[2];

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
  kMakeupGainL,
  kMaxGainReductL,
  kPressureR,
  kSpeedR,
  kMewinessR,
  kPawClawR,
  kMakeupGainR,
  kMaxGainReductR,
  kDryWet,
  kSidechainOn,
  kHighpassSC,
  kMidSideMode,
  kOutputMeter,
  kInMeter,
  kGRMeter,
  kSideDisableL,
  kSideDisableR,
  kLinkSides,
  kBypass,
  kLimiter,
  kMainOutput,
  kTextCtrl,
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

double debug1 = 6.66 ;
double debug2 = 6.66 ;
double debug3 = 6.66 ;


using namespace iplug;
using namespace igraphics;

class ComPressure final : public Plugin
{
public:
  ComPressure(const InstanceInfo& info);
  double calcGainRedCoeff(double inputSense, struct pressureVars& ps);

  void ultrasonicFilter(double& spL, double& spR, double* fix);

  bool clipOnly2(double& spl, struct clipOnly2Vars& cov, int spacing);

  void updateLED();
  void ResetMainSettings();

  void OnReset() override; 
  void OnActivate(bool active) override;
  void OnIdle() override;
  void OnParamChange(int paramIdx) override;

  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;

private:
  IPeakAvgSender<2> mInMeterSender;
  IPeakAvgSender<2> mOutMeterSender;
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
  bool GRLimitedL;
  bool GRLimitedR;
  bool mInputChansConnected[4] = {};
  SvfLinearTrapOptimised2 filter;

};
