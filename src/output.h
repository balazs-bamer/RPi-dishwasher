#ifndef DISHWASHER_OUTPUT_INCLUDED
#define DISHWASHER_OUTPUT_INCLUDED

#include "base.h"

/** Turns actuator commands into physical signals or Physical events in the test version.
 * Also accumulates errors and defies the actual actuator levels to hold the dishwasher
 * in a safe physical state if possible. */
class Output final : public Component {
public:
  Output();

  virtual ~Output() noexcept;

protected:
  virtual char const * getTaskName() const noexcept override {
    return "output ";
  }

  virtual bool shouldHaltOnError() const noexcept override {
    return false;
  }

  virtual bool shouldBeQueued(Event const &aEvent) const noexcept override {
    switch(aEvent.getType()) {
    case EventType::MeasuredDoor:
    case EventType::Actuate:
        return true;
    default:
        return false;
    }
  }

private:
  virtual void process(Event const &aEvent) noexcept override;

  virtual void process(int32_t const aExpired) noexcept override;
};

#endif // DISHWASHER_OUTPUT_INCLUDED
