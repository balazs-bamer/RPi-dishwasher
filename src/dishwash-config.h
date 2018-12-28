#ifndef  DISHWASHER_DISHWASH_CONFIG_INCLUDED
#define  DISHWASHER_DISHWASH_CONFIG_INCLUDED

#define DISHWASH_VERSION_MAJOR @DISHWASH_VERSION_MAJOR@
#define DISHWASH_VERSION_MINOR @DISHWASH_VERSION_MINOR@

#include<cstdint>

// all the
// times are in us
// heights in mm,
// currents in mA

class Config final {
public:
  // TODO define mm=function(temp, freq)
  static constexpr int32_t cWaterLevelFull         =    100;
  static constexpr int32_t cWaterLevelHalf         =     50;
  static constexpr int32_t cWaterLevelHisteresis   =      5;
  static constexpr int32_t cWaterLevelMax          =    110;
  static constexpr int32_t cWaterLevelRangeMin     =     -1;
  static constexpr int32_t cWaterLevelRangeMax     =    120;

  // TODO define mA=function(volt)
  static constexpr int32_t cCirculateCurrentMin    =    100;
  static constexpr int32_t cCirculateCurrentMax    =    400;

  // TODO define mA=function(volt)
  static constexpr int32_t cDrainCurrentMin        =     40;
  static constexpr int32_t cDrainCurrentMax        =    150;

  static constexpr int32_t cCurrentSettleTime      =   1000 * 1000;

  // TODO define deg=function(volt)
  static constexpr int32_t cTimePerDegRise         = 180000 * 1000; // TODO what was this for?

  static constexpr int32_t cTempMax                =     75;
  static constexpr int32_t cTempHisteresis         =      2;
  static constexpr int32_t cTempRangeMin           =      5;
  static constexpr int32_t cTempRangeMax           =    100;

  static constexpr int32_t cSprayChangeUpOn        =   1000 * 1000;
  static constexpr int32_t cSprayChangeUpOff       =   6500 * 1000;
  static constexpr int32_t cSprayChangeDownOn      =   5500 * 1000;
  static constexpr int32_t cSprayChangeDownOff     =   3000 * 1000;
  static constexpr int32_t cSprayChangeBothOn      =   3000 * 1000;
  static constexpr int32_t cSprayChangeBothOff     =   5000 * 1000;
  static constexpr int32_t cSprayChangeTolerance   =    400 * 1000;
  static constexpr int32_t cSprayChangeSearch      =  90000 * 1000;
  static constexpr int32_t cSprayChangeDeceleration =  1000 * 1000;
  static constexpr int32_t cSprayChangeKeepPosition = 20000 * 1000;

  static constexpr int32_t cCirculateOffTime       =   1000 * 1000;
  static constexpr int32_t cCirculateOnTime        =   2000 * 1000;

  static constexpr int32_t cSleepBeforeNextStep    =   5000 * 1000;
  static constexpr int32_t cAverageFillDrainMin    =      2;
  static constexpr int32_t cRegenerateValveTime    = 180000 * 1000;
  static constexpr int32_t cResinWashTime          = 120000 * 1000; // ms must be longer than cSprayChangeSearch
  static constexpr int32_t cWashDetergentOpenTime  =    200 * 1000;
  static constexpr int32_t cShutdownRelayOnTime    =     50 * 1000;
};

#endif // DISHWASHER_DISHWASH_CONFIG_INCLUDED
