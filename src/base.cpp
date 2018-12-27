#include "base.h"
#include "dishwash.h"
#include <mutex>


constexpr char Event::cStrInvalid[cStringSize];
constexpr char Event::cStrDoorState[][Event::cStringSize];
constexpr char Event::cStrSprayChangeState[][Event::cStringSize];
constexpr char Event::cStrOnOffState[][Event::cStringSize];
constexpr char Event::cStrActuate[][Event::cStringSize];
constexpr char Event::cStrProgram[][Event::cStringSize];
constexpr char Event::cStrState[][Event::cStringSize];
constexpr char Event::cStrError[][Event::cStringSize];
constexpr char Event::cStrEventType[][Event::cStringSize];

Event::Event(EventType const aType, OnOffState const aArg) noexcept : mType(aType), mOnOff(aArg) {
  if(aType != EventType::MeasuredSalt &&
     aType != EventType::MeasuredSpray &&
     aType != EventType::MeasuredLeak &&
     aType != EventType::DesiredCirc &&
     aType != EventType::DesiredResinWash &&
     aType != EventType::Pause) {
    mType = EventType::Error;
    mError = Error::Programmer;
  }
  else { // nothing to do
  }
}

Event::Event(EventType const aType, int32_t const aArg) noexcept : mType(aType), mIntValue(aArg) {
  if(aType != EventType::MeasuredCircCurrent &&
     aType != EventType::MeasuredDrainCurrent &&
     aType != EventType::MeasuredWaterLevel &&
     aType != EventType::MeasuredTemperature &&
     aType != EventType::DesiredWaterLevel &&
     aType != EventType::DesiredTemperature &&
     aType != EventType::RemainingTime &&
     aType != EventType::TimerFactorChanged)
    mType = EventType::Error;
    mError = Error::Programmer;
  }
  else { // nothing to do
  }
}

OnOffState Event::getOnOff() const noexcept {
  OnOffState ret;
  if(mType != EventType::MeasuredSalt &&
     mType != EventType::MeasuredSpray &&
     mType != EventType::MeasuredLeak &&
     mType != EventType::DesiredCirc &&
     mType != EventType::DesiredResinWash &&
     mType != EventType::Pause) {
    ret = OnOffState::Invalid;
  }
  else {
    ret = mOnOff;
  }
  return ret;
}

int32_t Event::getIntValue() const noexcept {
  int32_t ret;
  if(mType != EventType::MeasuredCircCurrent &&
     mType != EventType::MeasuredDrainCurrent &&
     mType != EventType::MeasuredWaterLevel &&
     mType != EventType::MeasuredTemperature &&
     mType != EventType::DesiredWaterLevel &&
     mType != EventType::DesiredTemperature &&
     mType != EventType::RemainingTime &&
     mType != EventType::TimerFactorChanged) {
    ret = cInvalid;
  }
  else {
    ret = mIntValue;
  }
  return ret;
}

char const * Event::getValueConstStr() const noexcept {
  char const * result = cStrInvalid;
  if(mType == EventType::MeasuredDoor) {
    result = cStrDoor[mIntValue + 1];
  }
  else if(mType == EventType::MeasuredSalt ||
          mType == EventType::MeasuredSpray ||
          mType == EventType::MeasuredLeak ||
          mType == EventType::DesiredCirc ||
          mType == EventType::DesiredResinWash ||
          mType == EventType::Pause) {
    result = cStrOnOffState[mIntValue + 1];
  }
  else if(mType == EventType::Error) {
    result = cStrError[getErrorStrIndex(mIntValue)];
  }
  else if(mType == EventType::DesiredSpray) {
    result = strSprayChangeState[mIntValue + 1];
  }
  else if(mType == EventType::Actuate) {
    result = cStrActuate[mIntValue + 1];
  }
  else if(mType == EventType::Program) {
    result = cStrProgram[mIntValue + 1];
  }
  else if(mType == EventType::MachineState) {
    result = cStrMachineState[mIntValue + 1];
  }
  else { // nothing to do
  }
  return result;
}

int32_t Event::getErrorStrIndex() const noexcept {
  int32_t result;
  if(mIntValue < 3) {
    result = mIntValue < -1 ? 0 : mIntValue + 1;
  }
  else {
    result = 2;
    for(uint32_t work = mIntValue; work & 1 == 0; work >>= 1u) {
      ++result;
    }
  }
  return result;
}


void Component::queueEvent(const Event &aEvent) noexcept {
  if(aEvent.getType() == EventType::Error) {
    mErrorSoFar |= static_cast<int32_t>(aEvent.getError());
  }
  else { // nothing to do
  }
  if(aEvent.getType() == EventType::Error || (mErrorSoFar.load() == cNoError && shouldBeQueued(aEvent))) {
    if(!queue.bounded_push(aEvent)) {
      raise(Error::Queue);
    }
    else {
      mConditionVariable.notify_one();
    }
  }
  else { // nothing to do
  }
}

void Component::send(Event const &aEvent) noexcept {
  if(aEvent.getType() == EventType::Error) {
    raise(aEvent.getError());
  }
  else {
    dishwasher.send(*this, aEvent);
  }
}

void Component::run() noexcept {
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  if(mKeepRunning) {
    mTimerManager.schedule(cWatchdogPatInterval);
  }
  else { // nothing to do
  }
  while(mKeepRunning) {
    std::optional<int64_t> nextTimeout = mTimerManager.getEarliestValidTimeoutLength();
    assert(nextTimeout);
    if(mConditionVariable.wait_for(lock, nextTimeout.value() * std::chrono::microsecond) == std::cv_status::timeout) {
      std::optional<int32_t> expiredAction = mTimerEvents.pop();
      while(expiredAction) {
        if(expiredAction.value() == TimerManager::cPatWatchdog) {
// TODO pat watchdog
          mTimerManager.schedule(cWatchdogPatInterval);
        }
        else {
          process(expiredAction.value());
        }
        expiredAction = mTimerEvents.pop();
      }
    }
    else { // nothing to do
    }
    while(queue.pop(event)) {
      if(mErrorSoFar.load() == cNoError || event.getType() == EventType::Error) {
        process(event);
      }
      else { // nothing to do
      }
    }
  }
}

void Component::raise(Error const aError) noexcept {
  mErrorSoFar |= static_cast<int32_t>(aError);
  mDishwasher.send(*this, Event(aError));
}

bool Component::assert(bool const aCondition) noexcept {
  if(aCondition) {
    raise(Error::Programmer);
  }
  else { // nothing to do
  }
  return aCondition;
}
