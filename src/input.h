#ifndef DISHWASHER_INPUT_INCLUDED
#define DISHWASHER_INPUT_INCLUDED

#include "base.h"

/** Reads and interprets sensor values and sends them as measurements.
 * Watches user input. In test version, instead of actual sensors it
 * simulates them using actuator switch outputs. */
class Input final : public Component {
public:
  Input(Dishwasher &d); // TODO this should take a mutex for I2C access control
  virtual ~Input() noexcept;

  virtual bool shouldHaltOnError() const noexcept {
    return true;
  }

  /** Test version accepts actuate, real one nothing. */
  virtual bool shouldBeQueued(Event const &aEvent) const noexcept {
    return false;
  }

private:
  virtual void process(Event const &aEvent) noexcept {}

  virtual void process(int32_t const aTimeEvent) noexcept {}

};

#endif // DISHWASHER_INPUT_INCLUDED
