#include "logic.h"
#include "dishwash.h"

using namespace std;

constexpr uint16_t Logic::cTemperatures[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(MachineState::Count)];
constexpr uint16_t Logic::cWaitMinutes[static_cast<int32_t>(Program::Count)][static_cast<int32_t>(MachineState::Count)];

bool Logic::shouldBeQueued(Event const &aEvent) const noexcept {
  switch(aEvent.getType()) {
  case EventType::MeasuredWaterLevel:
  case EventType::MeasuredDoor:
  case EventType::Program:
    return true;
  default:
    return false;
  }
}

void Logic::turnOffAll() noexcept {
  send(Event(EventType::DesiredResinWash, OnOffState::Off));
  send(Event(EventType::DesiredCirc, OnOffState::Off));
  send(EventType::DesiredWaterLevel, 0);
  send(EventType::DesiredTemperature, 0);
  send(SprayChangeState::Off);
  send(Actuate::Detergent0);
  send(Actuate::Regenerate0);
  send(Actuate::Shutdown0);
}

bool Logic::handleDoor(Event const &aEvent) noexcept {
  if(aEvent.getType() == EventType::MeasuredDoor) {
    if(aEvent.getDoor() == DoorState::Open) {
      mTimerManager.pause();
      mDoorOpen = true;
    }
    else if(aEvent.getDoor() == DoorState::Closed) {
      mTimerManager.resume();
      mDoorOpen = false;
    }
    return true;
  }
  return false;
}

void Logic::nextState() noexcept {
  do {
    mState = static_cast<MachineState>(static_cast<int32_t>(mState) + 1);
  } while(mState != MachineState::Count && cTemperatures[static_cast<int>(mProgram)][static_cast<int>(mState)] == No);
  if(mState == MachineState::Count) {
    mState = MachineState::Idle;
    mProgram = Program::None;
  }
  else { // nothing to do
  }
  mTargetTemperature = cTemperatures[static_cast<int>(mProgram)][static_cast<int>(mState)];
  mTargetTime = cWaitMinutes[static_cast<int>(mProgram)][static_cast<int>(mState)] * cUsInMinute;
  send(mState);
  turnOffAll();
  mTimerManager.schedule(Config::cSleepBeforeNextStep, cTimerBeforeNextStep);
}

void Logic::process(Program const aProgram) noexcept {
  if(mState == MachineState::Idle) {
    if(aProgram != Program::Stop) {
      mProgram = aProgram;
      nextState();
      int32_t remainingMilliseconds = 0;
      MachineState state = MachineState::Drain;
      do {
        remainingMilliseconds += cMsInMinute * cWaitMinutes[static_cast<int>(mProgram)][static_cast<int>(state)];
        if(state != MachineState::Dry) {
          remainingMilliseconds += cMsInSecond * Config::cAverageFillDrainSeconds;
        }
        else { // nothing to do
        }
        do {
          state = static_cast<MachineState>(static_cast<int32_t>(state) + 1);
        } while(state != MachineState::Shutdown && cWaitMinutes[static_cast<int>(mProgram)][static_cast<int>(state)] == No);
      } while(state != MachineState::Shutdown);
      send(EventType::RemainingTime, remainingMilliseconds);
    }
    else { // nothing to do
    }
  }
  else {
    if(aProgram == Program::Stop) {
      mProgram = Program::None;
      mState = MachineState::Idle;
      turnOffAll();
      mTimerManager.cancelAll();
    }
    else { // nothing to do
    }
  }
}

void Logic::doIdle(Event const &aEvent) noexcept {
  if(aEvent.getType() == EventType::Program) {
    process(aEvent.getProgram());
  }
}

void Logic::doDrain(int32_t const aExpired) noexcept {
  if(aExpired == cTimerBeforeNextStep) {
    send(EventType::DesiredWaterLevel, 0);
  }
  else { // nothing to do
  }
}

void Logic::doDrain(const Event &aEvent) noexcept {
  if(aEvent.getType() == EventType::Program) {
    process(aEvent.getProgram());
  }
  else if(aEvent.getType() == EventType::MeasuredWaterLevel && aEvent.getIntValue() <= Config::cWaterLevelHisteresis) {
    nextState();
  }
  else { // nothing to do
  }
}

void Logic::doResinWash(int32_t const aExpired) noexcept {
  if(aExpired == cTimerBeforeNextStep) {
    send(Event(EventType::DesiredResinWash, OnOffState::On));
    mTimerManager.schedule(Config::cResinWashTime, cTimerResinWashReady);
    mResinStopProgramWhenReady = false;
    mResinWashReady = false;
  }
  else if(aExpired == cTimerResinWashReady) {
    send(Event(EventType::DesiredResinWash, OnOffState::Off));
    mResinWashReady = true;
    if(mResinStopProgramWhenReady) {
      process(Program::Stop);
      return;
    }
    else { // nothing to do
    }
  }
  else { // nothing to do
  }
}

void Logic::doResinWash(const Event &aEvent) noexcept {
  if(aEvent.getType() == EventType::Program) {
    if(aEvent.getProgram() == Program::Stop) {
      if(!mResinWashReady) {
        mResinStopProgramWhenReady = true;
      }
      else {
        process(Program::Stop);
      }
    }       // does not allow instant stop to let the salty water completely out
  }
  else if(mResinWashReady && aEvent.getType() == EventType::MeasuredWaterLevel && aEvent.getIntValue() == 0) {
    nextState();
  }
  else { // nothing to do
  }
}

void Logic::doWash(int32_t const aExpired) noexcept {
  if(aExpired == cTimerBeforeNextStep) {
    send(EventType::DesiredWaterLevel, Config::cWaterLevelFull);
    mWashWaterFill = true;
    mWashWaterDrain = false;
  }
  else if(aExpired == cTimerWashDetergent) {
    send(Actuate::Detergent0);
  }
  else if(aExpired == cTimerWashWash) {
    send(Event(EventType::DesiredCirc, OnOffState::Off));
    send(Event(SprayChangeState::Off));
    send(EventType::DesiredTemperature, 0);
    send(EventType::DesiredWaterLevel, 0);
    mWashWaterDrain = true;
  }
  else { // nothing to do
  }
}

void Logic::doWash(const Event &aEvent) noexcept {
  if(aEvent.getType() == EventType::Program) {
    process(aEvent.getProgram());
  }
  else if(aEvent.getType() == EventType::MeasuredWaterLevel) {
    if(mWashWaterFill == true && aEvent.getIntValue() >= Config::cWaterLevelFull) {
      mWashWaterFill = false;
      send(EventType::DesiredTemperature, mTargetTemperature);
      send(Event(EventType::DesiredCirc, OnOffState::On));
      send(Event(EventType::DesiredSpray, SprayChangeState::On));
      if(mNeedDetergent) {
        send(Actuate::Detergent1);
        mTimerManager.schedule(Config::cWashDetergentOpenTime, cTimerWashDetergent);
      }
      else { // nothing to do
      }
      mTimerManager.schedule(cTimerWashWash, mTargetTime);
    }
    else { // nothing to do
    }
    if(mWashWaterDrain == true && aEvent.getIntValue() <= Config::cWaterLevelHisteresis) {
      nextState();
      mWashWaterDrain = false;
    }
    else { // nothing to do
    }
  }
  else { // nothing to do
  }
}

void Logic::doDry(int32_t const aExpired) noexcept {
  if(aExpired == cTimerBeforeNextStep) {
    mTimerManager.schedule(mTargetTime, cTimerDryReady);
    mTimerManager.schedule(Config::cRegenerateValveTime, cTimerDryRegenerate);
    send(Actuate::Regenerate1);
  }
  else if(aExpired == cTimerDryRegenerate) {
    send(Actuate::Regenerate0);
  }
  else if(aExpired == cTimerDryReady) {
    nextState();
  }
  else { // nothing to do
  }
}

void Logic::doDry(const Event &aEvent) noexcept {
  if(aEvent.getType() == EventType::Program) {
    process(aEvent.getProgram());
  }
  else { // nothing to do
  }
}

void Logic::doShutdown(int32_t const aExpired) noexcept {
  if(aExpired == cTimerBeforeNextStep) {
    send(Actuate::Shutdown1);
    mTimerManager.schedule(Config::cShutdownRelayOnTime, cTimerShutdownReady);
  }
  else if(aExpired == cTimerShutdownReady) {
    send(Actuate::Shutdown0);
  }
  else { // nothing to do
  }
}

void Logic::process(const Event &aEvent) noexcept {
  if(mErrorSoFar.load() != static_cast<int32_t>(Error::None)) {
    return; // abandon mProgram to let the display sign the mState when the error occured
  }
  if(handleDoor(aEvent) || mDoorOpen) {
    return;
  }
  mNeedDetergent = false;
  if(mState == MachineState::Idle) {
    doIdle(aEvent);
  }
  else if(mState == MachineState::Drain) {
    doDrain(aEvent);
  }
  else if(mState == MachineState::Resin) {
    doResinWash(aEvent);
  }
  else if(mState == MachineState::PreWash
       || mState == MachineState::Rinse1
       || mState == MachineState::Rinse2
       || mState == MachineState::Rinse3) {
    doWash(aEvent);
  }
  else if(mState == MachineState::Wash) {
    mNeedDetergent = true;
    doWash(aEvent);
  }
  else if(mState == MachineState::Dry) {
    doDry(aEvent);
  }
  else if(mState == MachineState::Shutdown) { // nothing to do
  }
  else {
    ensure(false);
  }
}

void Logic::process(int32_t const aExpired) noexcept {
  if(mState == MachineState::Drain) {
    doDrain(aExpired);
  }
  else if(mState == MachineState::Resin) {
    doResinWash(aExpired);
  }
  else if(mState == MachineState::PreWash
       || mState == MachineState::Wash
       || mState == MachineState::Rinse1
       || mState == MachineState::Rinse2
       || mState == MachineState::Rinse3) {
    doWash(aExpired);
  }
  else if(mState == MachineState::Dry) {
    doDry(aExpired);
  }
  else if(mState == MachineState::Shutdown) {
    doShutdown(aExpired);
  }
  else {
    ensure(false);
  }
}
