#include "timer.h"
#include <cmath>
#include <algorithm>

TimerManager::TimerManager(int32_t const aMaxLength, int32_t const aWatchdogLength) noexcept
  : mWatchdogLength(aWatchdogLength)
  , mMaxLength(aMaxLength){
  mTimers = new Timer[aMaxLength];
  std::optional<int32_t> highResolutionDelay = measureShortestThreadSleep<std::chrono::high_resolution_clock>();
  std::optional<int32_t> steadyDelay = measureShortestThreadSleep<std::chrono::steady_clock>();
  mClockToUse = (highResolutionDelay && highResolutionDelay.value() < steadyDelay.value() ? ClockToUse::HighResolution : ClockToUse::Steady);
  sTimeDividor = cRealtime;
  mWatchdogStart = now();
}

std::optional<int64_t> getEarliestValidTimeoutLength() const noexcept {
  std::optional<int64_t> result;
  int64_t now = now();
  int32_t currentIndex = mStartIndex;
  for(int32_t i = 0; i < mLength; ++i) {
    double expires = mTimers[currentIndex].getExpiration();
    if(!mPaused && expires > now) {
      result = expires - now;
      break;
    }
    else { // nothing to do
    }
    currentIndex = mTimers[currentIndex].nextIndex;
  }
  if(!result || result < mWatchdogStart + mWatchdogLength - now) {
    result = mWatchdogStart + mWatchdogLength - now;
  }
  else { // nothing to do
  }
  return result;
}

void TimerManager::setTimeDividor(double const aTimeDividor) noexcept {
  if(aTimeDividor >= cRealtime) {
    sTimeDividor = aTimeDividor;
    std::sort(mTimers, mTimers + mLength);
    mStartIndex = 0;
    mEndIndex = mLength - 1;
    for(int32_t i = 0; i < mLength; ++i) {
      mTimers[i].prevIndex = i - 1;
      mTimers[i].prevIndex = (i < mLength - 1 ? i + 1 : cEmptyIndex);
    }
  }
  else { // nothing to do
  }
}

void TimerManager::resume() noexcept {
  int64_t shift = now() - mPauseStart;
  mPauseStart.reset();
  for(int32_t i = 0; i < mLength; ++i) {
    mTimers[i] += shift;
  }
  setTimeDividor(sTimeDividor);
}

int64_t TimerManager::now() noexcept {
  int64_t result;
  if(mClockToUse == ClockToUse::HighResolution) {
    result = std::duration_cast<std::chrono::microseconds>(std::chrono::high_precision_clock.now().time_since_epoch()).count();
  }
  else {
    result = std::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock.now().time_since_epoch()).count();
  }
  return result;
}

void TimerManager::keepPattingWatchdog() noexcept {
  mWatchdogStart = now();
}

std::optional<int32_t> TimerManager::pop() {
  std::optional<int32_t> result;
  if(!mPaused && mLength > 0 && now() >= mTimers[mStartIndex].getExpiration()) {
    result = mTimers[mStartIndex].action;
    if(mLength == 1) {
      mStartIndex = mEndIndex = cEmptyIndex;
    }
    else {
      if(mStartIndex < mLength - 1) {
        int32_t nextStart = mTimers[mStartIndex].nextIndex;
        mTimers[mStartIndex] = mTimers[mLength - 1];
        if(mTimers[mStartIndex].nextIndex != cEmptyIndex) {
          mTimers[mTimers[mStartIndex].nextIndex].prevIndex = mStartIndex;
        }
        else { // nothing to do
        }
        if(mTimers[mStartIndex].prevIndex != cEmptyIndex) {
          mTimers[mTimers[mStartIndex].prevIndex].nextIndex = mStartIndex;
        }
        else { // nothing to do
        }
        mStartIndex = nextStart;
      }
      else {
        mStartIndex = mTimers[mStartIndex].nextIndex;
      }
      mTimers[mStartIndex].prevIndex = cEmptyIndex;
    }
    --mLength;
  }
  return result;
}

bool TimerManager::schedule(Timer const &aTimer) noexcept {
  bool result;
  if(mLength < tListSize) {
    mTimers[mLength] = aTimer;
    if(mLength == 0) {
      mStartIndex = mEndIndex = 0;
    }
    else {
      mTimers[mEndIndex].nextIndex = mLength;
      mTimers[mLength].prevIndex = mEndIndex;
      mEndIndex = mLength;
    }
    ++mLength;
    result = true;
  }
  else {
    result = false;
  }
  return result;
}
