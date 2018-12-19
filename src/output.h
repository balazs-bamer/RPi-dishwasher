#ifndef DISHWASHER_OUTPUT_INCLUDED
#define DISHWASHER_OUTPUT_INCLUDED

#include "base.h"

/** Turns actuator commands into physical signals or Physical events in the test version.
 * Also accumulates errors and difies the actual actuator levels to hold the dishwasher
 * in a safe physical state if possible. */
class Output final : public Component {
public:
    Output(Dishwasher &d) : Component(d) {}
    virtual ~Output() noexcept {}

    virtual void run() noexcept { doRun(GENERAL_POLL_PERIOD); }

    virtual bool shouldBeQueued(const Event &e) const noexcept;

private:
    virtual void process(const Event &event) noexcept;

    virtual void process(Error error) noexcept;
};

#endif // DISHWASHER_OUTPUT_INCLUDED
