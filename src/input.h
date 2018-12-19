#ifndef DISHWASHER_INPUT_INCLUDED
#define DISHWASHER_INPUT_INCLUDED

#include "base.h"

/** Reads and interprets sensor values and sends them as measurements.
 * Watches user input. In test version, instead of actual sensors it
 * simulates them using actuator switch outputs. */
class Input final : public Component {
private:
    enum Expired : int32_t { Invalid = -1 };

public:
    Input(Dishwasher &d); // TODO this should take a mutex for I2C access control
    virtual ~Input() noexcept;

    // TODO if door is open on startup, must send Event. Default is closed.
    virtual void run() noexcept { doRun(GENERAL_POLL_PERIOD); }

    /** Test version accepts actuate, real one nothing. */
    virtual bool shouldBeQueued(const Event &e) const noexcept { return false; }

private:
    virtual void processAtEachPoll() noexcept;

    virtual void process(const Event &event) noexcept {}

    virtual void process(Error error) noexcept {}

};

#endif // DISHWASHER_INPUT_INCLUDED
