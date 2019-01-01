#ifndef DISHWASHER_STATICERROR_INCLUDED
#define DISHWASHER_STATICERROR_INCLUDED

#include "base.h"

class StaticError final : public Component {
protected:
  virtual char const * getTaskName() const noexcept override {
    return "s_error";
  }

  virtual bool shouldHaltOnError() const noexcept override {
    return false;
  }

  virtual bool shouldBeQueued(Event const &aEvent) const noexcept override;

private:
  virtual void process(Event const &aEvent) noexcept override;

  virtual void process(int32_t const aExpired) noexcept override;
};

#endif // DISHWASHER_STATICERROR_INCLUDED
