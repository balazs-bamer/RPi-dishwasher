#include "dishwash-config.h"
#include "automat.h"
#include "dishwash.h"

using namespace std;

bool Automat::shouldBeQueued(Event const &aEvent) const noexcept {
  switch(aEvent.getType()) {
  case EventType::MeasuredSpray:
  case EventType::MeasuredWaterLevel:
  case EventType::MeasuredTemperature:
  case EventType::MeasuredCircCurrent:
  case EventType::MeasuredDrainCurrent:
  case EventType::DesiredSpray:
  case EventType::DesiredCirc:
  case EventType::DesiredWaterLevel:
  case EventType::DesiredTemperature:
  case EventType::DesiredResinWash:
    return true;
  default:
    return false;
  }
}

void Automat::doResinWashSwitch(ResinWashState const aDesired) noexcept {
  mDesiredResinWash = aDesired;
  if(mDesiredResinWash == ResinWashState::On) {
    mSprayChangeTransition = true;
    // great time to calibrate the spray changer mechanism
    mTimerManager.schedule(Config::cSprayChangeSearch, cTimerFinishSearchSelectPosition);
    send(Actuate::Spray1);
    mMeasureStart = mTimerManager.now();
    if(waterLevel < Config::cWaterLevelFull) {
      send(Actuate::Fill1);
    }
    else { // nothing to do
    }
    send(Actuate::Drain1);
  }
  else {
    mDesiredWaterLevel = 0;
    mDesiredTemperature = 0;
    mDesiredCirculate = OnOffState::Off;
    mDesiredSprayChange = OnOffState::Off;
    send(Actuate::Fill0);
    send(Actuate::Drain0);
  }
}

void Automat::doResinWashWaterLevel(uint16_t const aLevel) noexcept {
  mWaterLevel = aLevel;
  if(mWaterLevel < Config::cWaterLevelHalf) {
    send(Actuate::Fill1);
  }
  else { // nothing to do
  }
  if(mWaterLevel >= Config::cWaterLevelFull) {
    send(Actuate::Fill0);
  }
  else { // nothing to do
  }
}

void Automat::doResinWashSpray(SprayChangeState const aSpray) noexcept {
  if(mSprayPosition == SprayChangeState::Invalid) {
    int64_t now = mTimerManager.now();
    assert(mMeasuredTimeCount < cSprayChangeMaxMeasuredTimeCount && aSpray != mSprayContact);
    mmSprayChangeTimes[mMeasuredTimeCount++] = mMeasureStart - now;
    mMeasureStart = now;
    mSprayContact = aSpray;
  }
  else { // nothing to do
  }
}

void Automat::doResinWash(const Event &aEvent) noexcept {
  EventType type = aEvent.getType();
  if(type == EventType::DesiredResinWash) {
    doResinWashSwitch(event.getResinWash());
  }
  else if(type == EventType::MeasuredWaterLevel) {
    doResinWashWaterLevel(event.getWaterLevel());
  }
  else if(type == EventType::MeasuredSpray) {
    doResinWashSpray(event.getSpray());
  }
  else { // nothing to do
  }
}

void Automat::doWaterLevel(Event const &aEvent) noexcept {
  // TODO DrainCurrent
  EventType type = aEvent.getType();
  if(type == EventType::DesiredWaterLevel) {
    assert(mDesiredCirculate == OnOffState::On);
    mDesiredWaterLevel = aEvent.getWaterLevel();
    if(mWaterLevel < mDesiredWaterLevel - Config::cWaterLevelHisteresis) {
      send(Actuate::Drain0);
      send(Actuate::Fill1);
      // TODO timeout
    }
    else { // nothing to do
    }
    if(mWaterLevel > mDesiredWaterLevel + Config::cWaterLevelHisteresis) {
      send(Actuate::Drain1);
      send(Actuate::Fill0);
      // TODO timeout
    }
    else { // nothing to do
    }
  }
  else if(type == EventType::MeasuredWaterLevel) {
    mWaterLevel = event.getWaterLevel();
    if(mDesiredCirculate == CirculateState::On) {
      return;
    }
    else { // nothing to do
    }
    if(mWaterLevel < mDesiredWaterLevel - Config::cWaterLevelHisteresis) {
      send(Actuate::Drain0);
      send(Actuate::Fill1);
    }
    else if(mWaterLevel > mWesiredWaterLevel + Config::cWaterLevelHisteresis) {
      send(Actuate::Drain1);
      send(Actuate::Fill0);
    }
    else {
      // TODO cancel
    }
  }
  else { // nothing to do
  }
}

void Automat::doTemperature(Event const &aEvent) noexcept {
  EventType type = aEvent.getType();
  if(type == EventType::DesiredTemperature) {
    mDesiredTemperature = event.getTemperature();
    if(mTemperature < mDesiredTemperature - Config::cTempHisteresis) {
      send(Actuate::Heat1);
        // TODO timeout
    }
    else { // nothing to do
    }
    if(mTemperature > mDesiredTemperature + Config::cTempHisteresis) {
      send(Actuate::Heat0);
        // TODO timeout
    }
    else { // nothing to do
    }
  }
  else if(type == EventType::MeasuredTemperature) {
    mTemperature = event.getTemperature();
    if(mTemperature < mDesiredTemperature - Config::cTempHisteresis) {
      send(Actuate::Heat1);
    }
    else if(mTemperature > mDesiredTemperature + Config::cTempHisteresis) {
      send(Actuate::Heat0);
    }
    else {
        // TODO cancel
    }
  }
  else { // nothing to do
  }
}

// TODO later PWM
void Automat::doCirculate(Event const &aEvent) noexcept {
  EventType type = event.getType();
  if(type == EventType::DesiredCirc) {
    mDesiredCirculate = event.getCirc();
    if(mDesiredCirculate == CirculateState::On) {
      if(mSprayChangeTransition == false) {
        send(Actuate::Circ1);
      }
      else { // nothing to do
      }
    }
    else {
      send(Actuate::Circ0);
    }
  }
  else { // nothing to do
  }
}

void Automat::doSpray(Event const &event) noexcept {
  EventType type = event.getType();
  if(type == EventType::DesiredSpray) {
    SprayChangeState desired = event.getSpray();
    if(desired != SprayChangeState::On && desired != SprayChangeState::Off) {
      return;
    }
    else { // nothing to do
    }
    mDesiredSprayChange = desired;
    if(mDesiredSprayChange == SprayChangeState::On) {
      assert((mSprayPosition == SprayChangeState::Upper ||
              mSprayPosition == SprayChangeState::Lower ||
              mSprayPosition == SprayChangeState::Both) &&
              mSprayContact == SprayChangeState::On));
      send(Actuate::Spray1);
      send(Actuate::Circ0);  // prevent circulation during transition
      mSprayChangeTransition = true;
    }
    else { // nothing to do
    }
  }
  else if(type == EventType::MeasuredSpray) {
    mSprayContact = event.getSpray();
    if(mSprayContact == SprayChangeState::On) {
      if(mSprayPosition == SprayChangeState::Upper) {
        mTimerManager.schedule(Config::cSprayChangeDownOn, cTimerSprayChangeStop);
        mSprayPosition = SprayChangeState::Lower;
      }
      else if(mSprayPosition == SprayChangeState::Lower) {
        mTimerManager.schedule(Config::cSprayChangeBothOn, cTimerSprayChangeStop);
        mSprayPosition = SprayChangeState::Both;
      }
      else if(mSprayPosition == SprayChangeState::Both) {
        mTimerManager.schedule(Config::cSprayChangeUpOn, cTimerSprayChangeStop);
        mSprayPosition = SprayChangeState::Upper;
      }
      else { // nothing to do
      }
    }
    else { // nothing to do
    }
  }
  else { // nothing to do
  }
}

void Automat::process(Event const &aEvent) noexcept {
  if(error.load() == static_case<int32_t>(Error::None)) {
    return;
  }
  EventType type = event.getType();
  if(type == EventType::DesiredResinWash || desiredResinWash == ResinWashState::On) {
    doResinWash(event);
  }
  else if(type == EventType::Timer) {
    doExpired(event.getExpired());
  }
  else if(type == EventType::DesiredWaterLevel || type == EventType::MeasuredWaterLevel || type == EventType::MeasuredDrainCurrent) {
    doWaterLevel(event);
  }
  else if(type == EventType::DesiredTemperature || type == EventType::MeasuredTemperature) {
    doTemperature(event);
  }
  else if(type == EventType::DesiredCirc || type == EventType::MeasuredCircCurrent) {
    doCirculate(event);
  }
  else if(type == EventType::DesiredSpray || type == EventType::MeasuredSpray) {
    doSpray(event);
  }
  else { // nothing to do
  }
}

void Automat::process(int32_t const aTimerEvent) noexcept {
  if(aTimerEvent == cTimerSprayChangeStop) {
    mSprayChangeTransition = false;
    send(Actuate::Spray0);
    if(mDesiredCirculate == OnOffState::On) {
      send(Actuate::Circ1);
    }
    else { // nothing to do
    }
    if(mDesiredSprayChange == OnOffState::On) {
      mTimerManager.schedule(Config::cSprayChangeKeepPosition, cTimerSprayChangePause);
    }
    else { // nothing to do
    }
  }
  else if(aTimerEvent == cTimerSprayChangePause) {
    if(mDesiredSprayChange == SprayChangeState::On) {
      assert(((mSprayPosition == SprayChangeState::Upper ||
               mSprayPosition == SprayChangeState::Lower ||
               mSprayPosition == SprayChangeState::Both) &&
               mSprayContact == OnOffState::On));
      send(Actuate::Spray1);
      send(Actuate::Circ0);  // prevent circulation during transition
      mSprayChangeTransition = true;
    }
    else { // nothing to do
    }
  }
  else if(aExpired == cTimerFinishSearchSelectPosition) {
    mTimerManager.schedule(Config::cSprayChangeDeceleration, cTimerDecelerateSearchSelectPosition);
  }
  else if(aExpired == cTimerDecelerateSearchSelectPosition) {
    if(mMeasuredTimeCount < cExpectedSprayChangeTimeCount) {
      raise(Error::SpraySelect);
    }
    else { // nothing to do
    }
    int shortPos = 0;
    while(shortPos < mMeasuredTimeCount) {
      if(mSprayChangeTimes[shortPos] < Config::cSprayChangeUpOn + Config::cSprayChangeTolerance) {
        break;
      }
      else { // nothing to do
      }
      shortPos++;
    }
    int i;
    for(i = 0; shortPos < mMeasuredTimeCount; ++i, ++shortPos) {
      mSprayChangeTimes[i] = mSprayChangeTimes[shortPos];
    }
    mMeasuredTimeCount = i;
    if(mMeasuredTimeCount >= cSprayChangeCycle * 2) {
      for(i = 0; i < cSprayChangeCycle; i++) {
        mSprayChangeTimes[i] = (mSprayChangeTimes[i] + mSprayChangeTimes[i + cSprayChangeCycle]) / 2;
      }
    }
    else { // nothing to do
    }
    int rawPositionIndex = (mMeasuredTimeCount - 1) % cSprayChangeCycle;
    assert(1 - rawPositionIndex % 2 == static_cast<int>(mSprayContact))) {
    if(rawPositionIndex == 0 || rawPositionIndex == 1) {
      sprayPosition = SprayChangeState::Upper;
    }
    else if(rawPositionIndex == 2 || rawPositionIndex == 3) {
      sprayPosition = SprayChangeState::Lower;
    }
    else if(rawPositionIndex == 4 || rawPositionIndex == 5) {
      sprayPosition = SprayChangeState::Both;
    }
    else { // nothing to do
    }
    mSprayChangeTransition = false;
  }
  else { // nothing to do
  }
}
