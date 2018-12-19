#ifndef DISHWASHER_LOGIC_INCLUDED
#define DISHWASHER_LOGIC_INCLUDED


#include "base.h"

#define MS_IN_MIN 60000

/** Performs the program logic using internal timer and measured values.
 * Sends actoator commands or desired values. */
class Logic final : public Component {
private:
    enum Expired : int32_t { Invalid = -1, StartStep, ResinWashReady, WashDetergent, WashWash, DryRegenerate, DryReady, ShutdownReady };

    enum What : uint16_t {
        No = 0,  // not performed
        Yes  // done with cold water
        // other value is the target temperature or time
    };

    static constexpr uint16_t temperatures[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(State::Count)] = {
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

    static constexpr uint16_t waitMinutes[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(State::Count)] = {
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

    State state = State::Idle;
    Program program = Program::None;
    bool needDetergent = false;
    bool doorOpen = false;
    bool resinWashReady = false;
    bool resinStopProgramWhenReady = false;
    bool washWaterFill = false;
    bool washWaterDrain = false;

    /** Temperature to reach in this state, if applicable. 0 is no heating.*/
    int16_t targetTemperature;

    /** Time to wait in this step, if applicable. */
    int32_t targetTime;               // ms

    int32_t remainingTime;            // ms

public:
    Logic(Dishwasher &d) : Component(d) {}
    virtual ~Logic() noexcept {}

    virtual void run() noexcept { doRun(GENERAL_POLL_PERIOD, true); }

    virtual bool shouldBeQueued(const Event &e) const noexcept;

private:
    void turnOffAll() noexcept;
    bool handleDoor(const Event &event) noexcept;

    /** State transition according to program.
     * Every do* function must assure that all signals are shut down before this occurs. */
    void nextState() noexcept;

    /** Stops it only if prg is Program::Stop */
    void process(Program prg) noexcept;

    /** Only allows a program to be chosen. */
    void doIdle(const Event &event) noexcept;

    void doDrainExpired(int32_t exp) noexcept;
    void doDrainMeasured(const Event &event) noexcept;

    /** Drains the water if initially present. */
    void doDrain(const Event &event) noexcept;

    void doResinWashExpired(int32_t exp) noexcept;
    void doResinWashMeasured(const Event &event) noexcept;

    /** Starts the drain pump and lets 2l fresh water in
     * to wash them resin from the salt. */
    void doResinWash(const Event &event) noexcept;

    void doWashExpired(int32_t exp) noexcept;
    void doWashMeasured(const Event &event) noexcept;

    /** Fills in water. Starts to circulate it, heats if needed and opens the detergent lid if needed.
     * Continues doing so for the predetermined time.
     * After finishing shuts off heating and drains the water. */
    void doWash(const Event &event) noexcept;

    void doDryExpired(int32_t exp) noexcept;

    /** In the beginning it opens the regeneration valve for some time then closes it.
     * After it simply does nothing, and the water evaoprates from the hot dishes and
     * condenses in the duct. */
    void doDry(const Event &event) noexcept;

    /** Shuts down the machine if needed. */
    void doShutdown(const Event &event) noexcept;

    /** When the door is open, all the events except of errors and door events are discarded.
     * Timer is also paused. Output turns off all physical outputs as well, so nothing will be missed. */
    virtual void process(const Event &event) noexcept;

    virtual void process(Error error) noexcept {}
};

#endif // DISHWASHER_LOGIC_INCLUDED
