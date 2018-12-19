#ifndef DISHWASHER_STATICERROR_INCLUDED
#define DISHWASHER_STATICERROR_INCLUDED

#include "base.h"

class StaticError final : public Component {
private:
    enum Expired : int32_t { Invalid = -1 };

public:
    StaticError(Dishwasher &d) : Component(d) {}
    virtual ~StaticError() noexcept {}

    virtual void run() noexcept { doRun(GENERAL_POLL_PERIOD); }

    virtual bool shouldBeQueued(const Event &e) const noexcept;

private:
    virtual void process(const Event &event) noexcept;

    virtual void process(Error error) noexcept;
};

#endif // DISHWASHER_STATICERROR_INCLUDED
