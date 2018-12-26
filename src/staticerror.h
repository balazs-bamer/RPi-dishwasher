#ifndef DISHWASHER_STATICERROR_INCLUDED
#define DISHWASHER_STATICERROR_INCLUDED

#include "base.h"

class StaticError final : public Component {
public:
  StaticError(Dishwasher &d) : Component(d) {}
  virtual ~StaticError() noexcept {}

protected:
  virtual bool shouldHaltOnError() const noexcept {
    return false;
  }

  virtual bool shouldBeQueued(Event const &aEvent) const noexcept;

private:
  virtual void process(Event const &aEvent) noexcept;

  virtual void process(int32_t const aExpired) noexcept;
};

#endif // DISHWASHER_STATICERROR_INCLUDED
