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

void Automat::doResinWashSwitch(OnOffState const aDesired) noexcept {
  mDesiredResinWash = aDesired;
  if(mDesiredResinWash == OnOffState::On) {
    mSprayChangeTransition = true;
    // great time to calibrate the spray changer mechanism
    mTimerManager.schedule(Config::cSprayChangeSearch, cTimerFinishSearchSprayChangePosition);
    send(Actuate::Spray1);
    mMeasureStart = mTimerManager.now();
    if(mWaterLevel < Config::cWaterLevelFull) {
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

void Automat::doResinWashSpray(OnOffState const aSpray) noexcept {
  if(mSprayContact == OnOffState::Invalid) {
    int64_t now = mTimerManager.now();
    ensure(mMeasuredTimeCount < cSprayChangeMaxMeasuredTimeCount);
    mSprayChangeTimes[mMeasuredTimeCount++] = now - mMeasureStart;
    mMeasureStart = now;
    mSprayContact = aSpray;
  }
  else { // nothing to do
  }
}

void Automat::doResinWash(const Event &aEvent) noexcept {
  EventType type = aEvent.getType();
  if(type == EventType::DesiredResinWash) {
    doResinWashSwitch(aEvent.getOnOff());
  }
  else if(type == EventType::MeasuredWaterLevel) {
    doResinWashWaterLevel(aEvent.getIntValue());
  }
  else if(type == EventType::MeasuredSpray) {
    doResinWashSpray(aEvent.getOnOff());
  }
  else { // nothing to do
  }
}

void Automat::doWaterLevel(Event const &aEvent) noexcept {
  // TODO DrainCurrent
  EventType type = aEvent.getType();
  if(type == EventType::DesiredWaterLevel) {
    ensure(mDesiredCirculate == OnOffState::On);
    mDesiredWaterLevel = aEvent.getIntValue();
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
    mWaterLevel = aEvent.getIntValue();
    if(mDesiredCirculate == OnOffState::On) {
      return;
    }
    else { // nothing to do
    }
    if(mWaterLevel < mDesiredWaterLevel - Config::cWaterLevelHisteresis) {
      send(Actuate::Drain0);
      send(Actuate::Fill1);
    }
    else if(mWaterLevel > mDesiredWaterLevel + Config::cWaterLevelHisteresis) {
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
    mDesiredTemperature = aEvent.getIntValue();
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
    mTemperature = aEvent.getIntValue();
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
  EventType type = aEvent.getType();
  if(type == EventType::DesiredCirc) {
    mDesiredCirculate = aEvent.getOnOff();
    if(mDesiredCirculate == OnOffState::On) {
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
    OnOffState desired = event.getOnOff();
    if(desired != OnOffState::On && desired != OnOffState::Off) {
      return;
    }
    else { // nothing to do
    }
    mDesiredSprayChange = desired;
    if(mDesiredSprayChange == OnOffState::On) {
      ensure((mSprayPosition == SprayChangeState::Upper ||
              mSprayPosition == SprayChangeState::Lower ||
              mSprayPosition == SprayChangeState::Both) &&
              mSprayContact == OnOffState::On);
      send(Actuate::Spray1);
      send(Actuate::Circ0);  // prevent circulation during transition
      mSprayChangeTransition = true;
    }
    else { // nothing to do
    }
  }
  else if(type == EventType::MeasuredSpray) {
    mSprayContact = event.getOnOff();
    if(mSprayContact == OnOffState::On) {
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
  if(mErrorSoFar.load() == static_cast<int32_t>(Error::None)) {
    return;
  }
  EventType type = aEvent.getType();
  if(type == EventType::DesiredResinWash || mDesiredResinWash == OnOffState::On) {
    doResinWash(aEvent);
  }
  else if(type == EventType::DesiredWaterLevel || type == EventType::MeasuredWaterLevel || type == EventType::MeasuredDrainCurrent) {
    doWaterLevel(aEvent);
  }
  else if(type == EventType::DesiredTemperature || type == EventType::MeasuredTemperature) {
    doTemperature(aEvent);
  }
  else if(type == EventType::DesiredCirc || type == EventType::MeasuredCircCurrent) {
    doCirculate(aEvent);
  }
  else if(type == EventType::DesiredSpray || type == EventType::MeasuredSpray) {
    doSpray(aEvent);
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
    if(mDesiredSprayChange == OnOffState::On) {
      ensure(((mSprayPosition == SprayChangeState::Upper ||
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
  else if(aTimerEvent == cTimerFinishSearchSprayChangePosition) {
    mTimerManager.schedule(Config::cSprayChangeDeceleration, cTimerDecelerateSearchSprayChangePosition);
  }
  else if(aTimerEvent == cTimerDecelerateSearchSprayChangePosition) {
    if(mMeasuredTimeCount < cExpectedSprayChangeTimeCount) {
      raise(Error::SpraySelect, "spray change timeout");
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
    ensure(1 - rawPositionIndex % 2 == static_cast<int>(mSprayContact));
    if(rawPositionIndex == 0 || rawPositionIndex == 1) {
      mSprayPosition = SprayChangeState::Upper;
    }
    else if(rawPositionIndex == 2 || rawPositionIndex == 3) {
      mSprayPosition = SprayChangeState::Lower;
    }
    else if(rawPositionIndex == 4 || rawPositionIndex == 5) {
      mSprayPosition = SprayChangeState::Both;
    }
    else { // nothing to do
    }
    mSprayChangeTransition = false;
  }
  else { // nothing to do
  }
}
