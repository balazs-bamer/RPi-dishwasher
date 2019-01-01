#ifndef DISHWASHER_DISPLAY_INCLUDED
#define DISHWASHER_DISPLAY_INCLUDED

#include "base.h"

/** Displays all interesting information including program, current step, error,
 * door and salt state, actuator switch states, pump currents, water level and temperature.
 * Calculates the remaining time itself. */
class Display final : public Component {
private:
  static constexpr int32_t cStrLengthLong = 12;
  static constexpr int32_t cStrLengthShort = 6;
  static constexpr char errorMessages[][cStrLengthLong] = {
    "  Invalid  ",
    "    OK     ",
    "    I2C    ",
    "Programmer ",
    "   Queue   ",
    " No water  ",
    " Overfill  ",
    "Drain fail ",
    "   Leak    ",
    " No signal ",
    "Bad signal ",
    "Unst signal",
    "Circ ovload",
    " Circ conn ",
    "Circ stuck ",
    "Drain oload",
    "Drain conn ",
    "Drain stuck",
    "  No heat  ",
    " Overheat  ",
    " Bad temp  ",
    " Spray sel "
  };
  static constexpr char programNames[][cStrLengthShort] = {
    "Inval",
    "None ",
    "Stop ",
    "Drain",
    "Rinse",
    "Fast ",
    "FastD",
    "Middl",
    " All ",
    " Hot ",
    "Inten",
    "Cook "
  };
  static constexpr char stateNames[][cStrLengthShort] = {
    "Inval",
    "Idle ",
    "Drain",
    "Resin",
    " Pre ",
    "Wash ",
    "Rins1",
    "Rins2",
    "Rins3",
    " Dry ",
    "Shutd"
  };

  DoorState    mDoor          = DoorState::Invalid;
  OnOffState   mSalt          = OnOffState::Invalid;
  OnOffState   mSprayContact  = OnOffState::Invalid;
  OnOffState   mLeak          = OnOffState::Invalid;
  int16_t      mCircCurrent   = 0u;
  int16_t      mDrainCurrent  = 0u;
  int16_t      mWaterLevel    = 0u;
  int16_t      mTemperature   = 0u;
  // we already have mErrorSoFar
  uint8_t      mActuate       = 0;
  Program      mProgram       = Program::None;
  MachineState mState         = MachineState::Idle;
  int32_t      mRemainingTime = 0;
  int32_t      mTimerFactor   = 1;

  bool         mNeedsRefresh  = false;

public:
  Display(); // TODO this should take a mutex for I2C access control
  virtual ~Display() noexcept;

protected:
  virtual char const * getTaskName() const noexcept override {
    return "display";
  }

  virtual bool shouldHaltOnError() const noexcept override {
    return false;
  }

  virtual bool shouldBeQueued(Event const &aEvent) const noexcept override {
    return true;
  }

private:
  virtual void refresh() noexcept override;

  // We keep this here to let the two implementations share it.
  virtual void process(Event const &aEvent) noexcept override {
    EventType type = aEvent.getType();
    if(type == EventType::MeasuredDoor) {
      mDoor = aEvent.getDoor();
    }
    else if(type == EventType::MeasuredSalt) {
      mSalt = aEvent.getOnOff();
    }
    else if(type == EventType::MeasuredSpray) {
      mSprayContact = aEvent.getOnOff();
    }
    else if(type == EventType::MeasuredLeak) {
      mLeak = aEvent.getOnOff();
    }
    else if(type == EventType::MeasuredCircCurrent) {
      mCircCurrent = aEvent.getIntValue();
    }
    else if(type == EventType::MeasuredDrainCurrent) {
      mDrainCurrent = aEvent.getIntValue();
    }
    else if(type == EventType::MeasuredWaterLevel) {
      mWaterLevel = aEvent.getIntValue();
    }
    else if(type == EventType::MeasuredTemperature) {
      mTemperature = aEvent.getIntValue();
    }
    else if(type == EventType::Actuate) {
      uint32_t raw = static_cast<uint32_t>(aEvent.getActuate());
      if(raw > 0) {
        if(raw & 1) { // turn on
          mActuate |= 1 << (raw / 2);
        }
        else { // turn off
          mActuate &= ~static_cast<uint8_t>(1u << (raw / 2u));
        }
      }
      else { // nothing to do
      }
    }
    else if(type == EventType::Program) {
      mProgram = aEvent.getProgram();
    }
    else if(type == EventType::MachineState) {
      mState = aEvent.getMachineState();
    }
    else if(type == EventType::RemainingTime) {
      mRemainingTime = aEvent.getIntValue();
    }
    else if(type == EventType::TimeFactorChanged) {
      mTimerFactor = aEvent.getIntValue();
    }
    else { // nothing to do
    }
    mNeedsRefresh = true;
  }

  virtual void process(int32_t const aExpired) noexcept override {
  }
};

#endif // DISHWASHER_DISPLAY_INCLUDED
