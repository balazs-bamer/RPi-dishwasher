#ifndef DISHWASHER_AUTOMAT_INCLUDED
#define DISHWASHER_AUTOMAT_INCLUDED

#include "base.h"

/** Manages water level, temperature, circulation and spray selector
 * based on measured values and desired values. */
class Automat final : public Component {
  enum class SprayChangeState : int32_t { Invalid = -1, Upper, Lower, Both };

  static constexpr int32_t cSprayChangeMaxMeasuredTimeCount          = 30;
  static constexpr int32_t cExpectedSprayChangeTimeCount             = 18;
  static constexpr int32_t cSprayChangeCycle                         =  6;

  static constexpr int32_t cTimerFinishSearchSprayChangePosition     =  0;
  static constexpr int32_t cTimerDecelerateSearchSprayChangePosition =  1;
  static constexpr int32_t cTimerSprayChangeStop                     =  2;
  static constexpr int32_t cTimerSprayChangePause                    =  3;

  OnOffState mDesiredResinWash   = OnOffState::Off;
  int16_t    mDesiredTemperature = 0;
  int16_t    mDesiredWaterLevel  = 0;
  OnOffState mDesiredCirculate   = OnOffState::Off;
  OnOffState mDesiredSprayChange = OnOffState::Off;

  uint16_t         mWaterLevel = 0;
  uint16_t         mTemperature = 0;
  OnOffState       mSprayContact = OnOffState::Invalid;
  /** Valid if sprayContact is on. Signs the previous state if it is off. */
  SprayChangeState mSprayPosition = SprayChangeState::Invalid;
  bool             mSprayChangeTransition = false;

  int              mMeasuredTimeCount = 0;
  int64_t          mSprayChangeTimes[cSprayChangeMaxMeasuredTimeCount];
  int64_t          mMeasureStart;

protected:
  virtual char const * getTaskName() const noexcept override {
    return "automat";
  }

  virtual bool shouldHaltOnError() const noexcept override {
    return true;
  }

  virtual bool shouldBeQueued(const Event &e) const noexcept override;

private:
  void doResinWashSwitch(OnOffState const aDesired) noexcept;
  void doResinWashWaterLevel(uint16_t const aLvel) noexcept;
  void doResinWashSpray(OnOffState const aSpray) noexcept;

  /** Controls only resin wash. After turning it off, switches every other
   * controllable parameter off so that if resin wash finishes, other parts make anything unexpected.
   * Meanwhile it calibrates the spray change system. */
  void doResinWash(Event const &aEvent) noexcept;
  void doWaterLevel(Event const &aEvent) noexcept;
  void doTemperature(Event const &aEvent) noexcept;
  void doCirculate(Event const &event) noexcept;
  void doSpray(Event const &event) noexcept;

  /** Won't continuously adjust for DesiredWaterLevel. Once the wish arrives,
   * drains / fills water until it is fulfilled, and then abandons it,
   * because circulation will schrink the water level in the sump.
   * When the door is open, all the events are processed as usual, the timer remains on.
   * However, Output turns off all the pysical signals. Automatics will catch on if they return. */
  virtual void process(Event const &event) noexcept override;

  virtual void process(int32_t const aTimerEvent) noexcept override;
};

#endif // DISHWASHER_AUTOMAT_INCLUDED
