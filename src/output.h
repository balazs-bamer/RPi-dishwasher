#ifndef DISHWASHER_OUTPUT_INCLUDED
#define DISHWASHER_OUTPUT_INCLUDED

#include "base.h"

/** Turns actuator commands into physical signals or Physical events in the test version.
 * Also accumulates errors and defies the actual actuator levels to hold the dishwasher
 * in a safe physical state if possible. */
class Output final : public Component {
public:
    Output(Dishwasher &d) : Component(d) {
    }

    virtual ~Output() noexcept {
    }

protected:
    virtual bool shouldHaltOnError() const noexcept {
      return false;
    }

    virtual bool shouldBeQueued(Event const &aEvent) const noexcept;

private:
    virtual void process(Event const &aEvent) noexcept;

    virtual void process(int32_t const aExpired) noexcept;
};

#endif // DISHWASHER_OUTPUT_INCLUDED
