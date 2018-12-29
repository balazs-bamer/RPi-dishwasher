#ifndef DISHWASHER_LOGIC_INCLUDED
#define DISHWASHER_LOGIC_INCLUDED


#include "base.h"

/** Performs the program logic using internal timer and measured values.
 * Sends actoator commands or desired values. */
class Logic final : public Component {
  static constexpr int32_t cTimerBeforeNextStep = 0;
  static constexpr int32_t cTimerResinWashReady = 1;
  static constexpr int32_t cTimerWashDetergent  = 2;
  static constexpr int32_t cTimerWashWash       = 3;
  static constexpr int32_t cTimerDryRegenerate  = 4;
  static constexpr int32_t cTimerDryReady       = 5;
  static constexpr int32_t cTimerShutdownReady  = 6;

  enum What : uint16_t {
    No  = 0,  // not performed
    Yes = 1  // done with cold water
    // other value is the target temperature or time
  };

  static constexpr uint16_t cTemperatures[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(MachineState::Count)] = {
//           Idle, Drain, Resin, PreWash, Wash, Rinse1, Rinse2, Rinse3, Dry, Shutdown
/*None*/   { No,   No,    No,    No,      No,   No,     No,     No,     No,  Yes },
/*Stop*/   { No,   No,    No,    No,      No,   No,     No,     No,     No,  Yes },
/*Drain*/  { No,   Yes,   No,    No,      No,   No,     No,     No,     No,  No  },
/*Rinse*/  { No,   Yes,   No,    No,      No,   Yes,    No,     No,     No,  No  },
/*Fast*/   { No,   Yes,   Yes,   No,      40,   Yes,    No,     Yes,    No,  Yes },
/*FastDry*/{ No,   Yes,   Yes,   No,      40,   Yes,    No,     55,     Yes, Yes },
/*Middle*/ { No,   Yes,   Yes,   Yes,     50,   Yes,    No,     65,     Yes, Yes },
/*All*/    { No,   Yes,   Yes,   Yes,     50,   Yes,    Yes,    65,     Yes, Yes },
/*Hot*/    { No,   Yes,   Yes,   Yes,     65,   Yes,    Yes,    65,     Yes, Yes },
/*Intens.*/{ No,   Yes,   Yes,   40,      65,   Yes,    Yes,    65,     Yes, Yes },
/*Cook*/   { No,   Yes,   No,    65,      No,   No,     No,     No,     Yes, No  }
  };

  static constexpr uint16_t cWaitMinutes[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(MachineState::Count)] = {
//                         ? TODO measure water quantity
//           Idle, Drain, Resin, PreWash, Wash, Rinse1, Rinse2, Rinse3, Dry, Shutdown
/*None*/   { No,   No,    No,    No,      No,   No,     No,     No,     No,  Yes },
/*Stop*/   { No,   No,    No,    No,      No,   No,     No,     No,     No,  Yes },
/*Drain*/  { No,   Yes,   No,    No,      No,   No,     No,     No,     No,  No  },
/*Rinse*/  { No,   Yes,   No,    No,      No,   10,     No,     No,     No,  No  },
/*Fast*/   { No,   Yes,   5,     No,      30,   10,     No,     10,     No,  Yes },
/*FastDry*/{ No,   Yes,   5,     No,      30,   10,     No,     20,     50,  Yes },
/*Middle*/ { No,   Yes,   5,     10,      60,   10,     No,     30,     50,  Yes },
/*All*/    { No,   Yes,   5,     10,      60,   10,     10,     30,     50,  Yes },
/*Hot*/    { No,   Yes,   5,     10,      60,   10,     10,     30,     50,  Yes },
/*Intens.*/{ No,   Yes,   5,     10,      60,   10,     10,     30,     50,  Yes },
/*Cook*/   { No,   Yes,   No,    60,      No,   No,     No,     No,     50,  No  }
  };

  MachineState mState                     = MachineState::Idle;
  Program      mProgram                   = Program::None;
  bool         mNeedDetergent             = false;
  bool         mDoorOpen                  = false;
  bool         mResinWashReady            = false;
  bool         mResinStopProgramWhenReady = false;
  bool         mWashWaterFill             = false;
  bool         mWashWaterDrain            = false;

  /** Temperature to reach in this state, if applicable. 0 is no heating.*/
  int16_t mTargetTemperature;

  /** Time to wait in this step, if applicable. */
  int64_t mTargetTime;               // us

protected:
  virtual bool shouldHaltOnError() const noexcept {
    return true;
  }

  virtual bool shouldBeQueued(Event const &aEvent) const noexcept;

private:
  void turnOffAll() noexcept;
  bool handleDoor(Event const &aEvent) noexcept;

  /** MachineState transition according to program.
   * Every do* function must assure that all signals are shut down before this occurs. */
  void nextState() noexcept;

  /** Stops it only if prg is Program::Stop */
  void process(Program const aProgram) noexcept;

  /** Only allows a program to be chosen. */
  void doIdle(Event const &aEvent) noexcept;

  void doDrain(int32_t const aExpired) noexcept;

  /** Drains the water if initially present. */
  void doDrain(Event const &aEvent) noexcept;

  void doResinWash(int32_t const aExpired) noexcept;

  /** Starts the drain pump and lets 2l fresh water in
   * to wash them resin from the salt. */
  void doResinWash(Event const &aEvent) noexcept;

  void doWash(int32_t const aExpired) noexcept;

  /** Fills in water. Starts to circulate it, heats if needed and opens the detergent lid if needed.
   * Continues doing so for the predetermined time.
   * After finishing shuts off heating and drains the water. */
  void doWash(Event const &aEvent) noexcept;

  void doDry(int32_t const aExpired) noexcept;

  /** In the beginning it opens the regeneration valve for some time then closes it.
   * After it simply does nothing, and the water evaoprates from the hot dishes and
   * condenses in the duct. */
  void doDry(Event const &aEvent) noexcept;

  /** Shuts down the machine if needed. */
  void doShutdown(int32_t const aEvent) noexcept;

  /** When the door is open, all the events except of errors and door events are discarded.
   * Timer is also paused. Output turns off all physical outputs as well, so nothing will be missed. */
  virtual void process(Event const &aEvent) noexcept;

  virtual void process(int32_t const aExpired) noexcept;
};

#endif // DISHWASHER_LOGIC_INCLUDED
