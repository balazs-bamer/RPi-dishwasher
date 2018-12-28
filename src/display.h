#ifndef DISHWASHER_DISPLAY_INCLUDED
#define DISHWASHER_DISPLAY_INCLUDED

#include <thread>
#include <atomic>

#include "dishwash-config.h"
#include "input.h"
#include "logic.h"

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

    uint16_t waterLevel = 0;
    uint16_t temperature = 0;
    uint16_t circCurrent = 0;
    uint16_t drainCurrent = 0;
    DoorState door = DoorState::Invalid;
    OnOffState salt = OnOffState::Invalid;
    OnOffState leak = OnOffState::Invalid;
    SprayChangeState sprayContact = SprayChangeState::Invalid;
    SprayChangeState sprayPosition = SprayChangeState::Invalid;

    uint8_t actuate = 0;
    Program program = Program::None;
    MachineState state = MachineState::Idle;
    uint32_t remainingTime = 0;

public:
    Display(Dishwasher &d); // TODO this should take a mutex for I2C access control
    virtual ~Display() noexcept {}

    virtual bool shouldBeQueued(const Event &e) const noexcept;

private:
    virtual void processAtEachPoll() noexcept;

    virtual void process(const Event &event) noexcept;

    virtual void process(Error error) noexcept {}
};

#endif // DISHWASHER_DISPLAY_INCLUDED
