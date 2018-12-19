#ifndef DISHWASHER_AUTOMAT_INCLUDED
#define DISHWASHER_AUTOMAT_INCLUDED

#include "base.h"

/** Manages water level, temperature, circulation and spray selector
 * based on measured values and desired values. */
class Automat final : public Component {
private:
    enum Expired : int32_t { Invalid = -1, FinishSearchSelectPosition, DecelerateSearchSelectPosition, SprayChangeStop, SprayChangePause };

    ResinWashState desiredResinWash = ResinWashState::Off;
    int16_t desiredTemperature = 0;
    int16_t desiredWaterLevel = 0;
    CirculateState desiredCirculate = CirculateState::Off;
    SprayChangeState desiredSprayChange = SprayChangeState::Off;

    uint16_t waterLevel = 0;
    uint16_t temperature = 0;
    uint16_t circCurrent = 0;
    uint16_t drainCurrent = 0;
    SprayChangeState sprayContact = SprayChangeState::Invalid;
    /** Valid if sprayContact is on. Signs the previous state if it is off. */
    SprayChangeState sprayPosition = SprayChangeState::Invalid;
    bool sprayChangeTransition = false;

    static constexpr int maxMeasuredTimeCount = 30;
    int measuredTimeCount = 0;
    uint32_t sprayChangeTimes[maxMeasuredTimeCount];
    std::chrono::system_clock::time_point measureStart;

public:
    Automat(Dishwasher &d) : Component(d) {}
    virtual ~Automat() noexcept {}

    virtual void run() noexcept { doRun(GENERAL_POLL_PERIOD); }

    virtual bool shouldBeQueued(const Event &e) const noexcept;

private:
    void doResinWashSwitch(ResinWashState desired) noexcept;
    void doResinWashWaterLevel(uint16_t level) noexcept;
    void doResinWashSpray(SprayChangeState spray) noexcept;
    void doResinWashExpired(int32_t expired) noexcept;

    /** Controls only resin wash. After turning it off, switches every other
     * controllable parameter off so that if resin wash finishes, doRegular
     * won't make anything unexpected.
     * Meanwhile it calibrates the spray change system. */
    void doResinWash(const Event &event) noexcept;

    void doRegularExpired(int32_t expired) noexcept;
    void doRegularWaterLevel(const Event &event) noexcept;
    void doRegularTemperature(const Event &event) noexcept;
    void doRegularCirculate(const Event &event) noexcept;
    void doRegularSpray(const Event &event) noexcept;

    /** Controls circulate, spray change, water level and temperature. */
    void doRegular(const Event &event) noexcept;

    /** Won't continuously adjust for DWaterLevel. Once the wish arrives,
     * drains / fills water until it is fulfilled, and then abandons it,
     * because circulation will schrink the water level in the sump.
     * When the door is open, all the events are processed as usual, the timer remains on.
     * However, Output turns off all the pysical signals. Automatics will catch on if they return. */
    virtual void process(const Event &event) noexcept;

    virtual void process(Error error) noexcept;
};

#endif // DISHWASHER_AUTOMAT_INCLUDED
