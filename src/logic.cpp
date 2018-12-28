#include "logic.h"
#include "dishwash.h"

using namespace std;

constexpr uint16_t Logic::temperatures[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(State::Count)];
constexpr uint16_t Logic::waitMinutes[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(State::Count)];

bool Logic::shouldBeQueued(const Event &e) const noexcept {
    switch(e.getType()) {
    case EventType::MeasuredWaterLevel:
    case EventType::MeasuredDoor:
    case EventType::Program:
        return true;
    default:
        return false;
    }
}

void Logic::turnOffAll() noexcept {
    send(ResinWashState::Off);
    send(CirculateState::Off);
    send(EventType::DesiredWaterLevel, 0);
    send(EventType::DesiredTemperature, 0);
    send(Event(EventType::DesiredSpray, SprayChangeState::Off));
    send(Actuate::Detergent0);
    send(Actuate::Regenerate0);
    send(Actuate::Shutdown0);
}

bool Logic::handleDoor(const Event &event) noexcept {
    if(event.getType() == EventType::MeasuredDoor) {
        if(event.getDoor() == DoorState::Open) {
            mTimerManager.pause();
            doorOpen = true;
        }
        else if(event.getDoor() == DoorState::Closed) {
            mTimerManager.resume();
            doorOpen = false;
        }
        return true;
    }
    return false;
}

void Logic::nextState() noexcept {
    do {
        state = static_cast<State>(static_cast<int32_t>(state) + 1);
    } while(state != State::Count && temperatures[static_cast<int>(program)][static_cast<int>(state)] == No);
    if(state == State::Count) {
        state = State::Idle;
        program = Program::None;
    }
    targetTemperature = temperatures[static_cast<int>(program)][static_cast<int>(state)];
    targetTime = waitMinutes[static_cast<int>(program)][static_cast<int>(state)] * MS_IN_MIN;
    send(state);
    turnOffAll();
    mTimerManager.schedule(StartStep, SLEEP_BEFORE_NEXT_STEP);
}

void Logic::process(Program prg) noexcept {
    if(state == State::Idle) {
        if(prg != Program::Stop) {
            program = prg;
            nextState();
            remainingTime = 0;
            State st = State::Drain;
            do {
                remainingTime += MS_IN_MIN * waitMinutes[static_cast<int>(program)][static_cast<int>(st)];
                if(st != State::Dry) {
                    remainingTime += MS_IN_MIN * AVERAGE_FILL_DRAIN_MIN;
                }
                do {
                    st = static_cast<State>(static_cast<int32_t>(st) + 1);
                } while(st != State::Shutdown && waitMinutes[static_cast<int>(program)][static_cast<int>(st)] == No);
            } while(st != State::Shutdown);
            send(EventType::RemainingTime, remainingTime);
        }
    }
    else {
        if(prg == Program::Stop) {
            program = Program::None;
            state = State::Idle;
            turnOffAll();
            mTimerManager.cancelAll();
        }
    }
}

void Logic::doIdle(const Event &event) noexcept {
    if(event.getType() == EventType::Program) {
        process(event.getProgram());
    }
}

void Logic::doDrainExpired(int32_t exp) noexcept {
    if(exp == StartStep) {
        send(EventType::DesiredWaterLevel, 0);
    }
}

void Logic::doDrainMeasured(const Event &event) noexcept {
    if(event.getType() == EventType::MeasuredWaterLevel) {
        if(event.getWaterLevel() <= WATER_LEVEL_HISTERESIS) {
            nextState();
        }
    }
}

void Logic::doDrain(const Event &event) noexcept {
    if(event.getType() == EventType::Timer) {
        doDrainExpired(event.getExpired());
    }
    else if(event.getType() == EventType::Program) {
        process(event.getProgram());
    }
    else {
        doDrainMeasured(event);
    }
}

void Logic::doResinWashExpired(int32_t exp) noexcept {
    if(exp == StartStep) {
        send(ResinWashState::On);
        mTimerManager.schedule(ResinWashReady, RESIN_WASH_TIME);
        resinStopProgramWhenReady = false;
        resinWashReady = false;
    }
    else if(exp == ResinWashReady) {
        send(ResinWashState::Off);
        resinWashReady = true;
        if(resinStopProgramWhenReady) {
            process(Program::Stop);
            return;
        }
    }
}

void Logic::doResinWashMeasured(const Event &event) noexcept {
    if(resinWashReady && event.getType() == EventType::MeasuredWaterLevel) {
        if(event.getWaterLevel() == 0) {
            nextState();
        }
    }
}

void Logic::doResinWash(const Event &event) noexcept {
    if(event.getType() == EventType::Timer) {
        doResinWashExpired(event.getExpired());
    }
    else if(event.getType() == EventType::Program) {
        if(event.getProgram() == Program::Stop) {
            if(!resinWashReady) {
                resinStopProgramWhenReady = true;
            }
            else {
                process(Program::Stop);
            }
        }       // does not allow instant stop to let the salty water completely out
    }
    else {
        doResinWashMeasured(event);
    }
}

void Logic::doWashExpired(int32_t exp) noexcept {
    if(exp == StartStep) {
        send(EventType::DesiredWaterLevel, WATER_LEVEL_FULL);
        washWaterFill = true;
        washWaterDrain = false;
    }
    else if(exp == WashDetergent) {
        send(Actuate::Detergent0);
    }
    else if(exp == WashWash) {
        send(CirculateState::Off);
        send(Event(EventType::DesiredSpray, SprayChangeState::Off));
        send(EventType::DesiredTemperature, 0);
        send(EventType::DesiredWaterLevel, 0);
        washWaterDrain = true;
    }
}

void Logic::doWashMeasured(const Event &event) noexcept {
    if(event.getType() == EventType::MeasuredWaterLevel) {
        if(washWaterFill == true && event.getWaterLevel() >= WATER_LEVEL_FULL) {
            washWaterFill = false;
            send(EventType::DesiredTemperature, targetTemperature);
            send(CirculateState::On);
            send(Event(EventType::DesiredSpray, SprayChangeState::On));
            if(needDetergent) {
                send(Actuate::Detergent1);
                mTimerManager.schedule(WashDetergent, WASH_DETERGENT_OPEN_TIME);
            }
            mTimerManager.schedule(WashWash, targetTime);
        }
        if(washWaterDrain == true && event.getWaterLevel() <= WATER_LEVEL_HISTERESIS) {
            nextState();
            washWaterDrain = false;
        }
    }
}

void Logic::doWash(const Event &event) noexcept {
    if(event.getType() == EventType::Timer) {
        doWashExpired(event.getExpired());
    }
    else if(event.getType() == EventType::Program) {
        process(event.getProgram());
    }
    else {
        doWashMeasured(event);
    }
}

void Logic::doDryExpired(int32_t exp) noexcept {
    if(exp == StartStep) {
        mTimerManager.schedule(DryReady, targetTime);
        mTimerManager.schedule(DryRegenerate, REGENERATE_VALVE_TIME);
        send(Actuate::Regenerate1);
    }
    else if(exp == DryRegenerate) {
        send(Actuate::Regenerate0);
    }
    else if(exp == DryReady) {
        nextState();
    }
}

void Logic::doDry(const Event &event) noexcept {
    if(event.getType() == EventType::Timer) {
        doDryExpired(event.getExpired());
    }
    else if(event.getType() == EventType::Program) {
        process(event.getProgram());
    }
}

void Logic::doShutdown(const Event &event) noexcept {
    if(event.getType() != EventType::Timer) {
        return;
    }
    int32_t exp = event.getExpired();
    if(exp == StartStep) {
        send(Actuate::Shutdown1);
        mTimerManager.schedule(ShutdownReady, SHUTDOWN_RELAY_ON_TIME);
    }
    else if(exp == ShutdownReady) {
        send(Actuate::Shutdown0);
    }
}

void Logic::process(const Event &event) noexcept {
    if(error.load() != static_case<int32_t>(Error::None)) {
        return; // abandon program to let the display sign the state when the error occured
    }
    if(handleDoor(event) || doorOpen) {
        return;
    }
    needDetergent = false;
    switch(state) {
    case State::Idle:
        doIdle(event);
        break;
    case State::Drain:
        doDrain(event);
        break;
    case State::Resin:
        doResinWash(event);
        break;
    case State::PreWash:
        doWash(event);
        break;
    case State::Wash:
        needDetergent = true;
        doWash(event);
        break;
    case State::Rinse1:
        doWash(event);
        break;
    case State::Rinse2:
        doWash(event);
        break;
    case State::Rinse3:
        doWash(event);
        break;
    case State::Dry:
        doDry(event);
        break;
    case State::Shutdown:
        doShutdown(event);
        break;
    default:
        ensure(false);
    }
}
