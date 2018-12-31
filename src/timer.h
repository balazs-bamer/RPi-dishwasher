#ifndef DISHWASHER_TIMER_INCLUDED
#define DISHWASHER_TIMER_INCLUDED

#include "bancopymove.h"
#include <array>
#include <thread>
#include <chrono>
#include <optional>

/// Class to manage action delays and changing realtime execution by dividing the delay length with a custom dividor.
/// The constructor will choose the most precise steady clock to use.
class TimerManager final : public BanCopyMove  {
  enum class ClockToUse : int32_t {
    HighResolution = 0,
    Steady         = 1
  };

  static constexpr int32_t cEmptyIndex                          = -1;
  static constexpr double  cRealtime                            =  1.0;
  static constexpr double  cEpsilon                             =  0.001;
  static constexpr double  cMeasureShortestThreadSleepExcess    =  1.05; // At most 5% error
  static constexpr int32_t cMeasureShortestThreadSleepRepeats   = 20;
  static constexpr int32_t cMeasureShortestThreadSleepMaxMillis = 50; // Must work for 50 ms

  struct Timer final {
    int64_t start;  /// us
    int64_t length; /// us, realtime without dividing
    int32_t action;

    /// cEmptyIndex for beginning
    int32_t prevIndex;

    /// cEmptyIndex for final
    int32_t nextIndex;

    Timer() : prevIndex(cEmptyIndex), nextIndex(cEmptyIndex) {
    }

    Timer(int64_t const aStart, int64_t const aLength, int32_t aAction) : start(aStart), length(aLength), action(aAction), prevIndex(cEmptyIndex), nextIndex(cEmptyIndex) {
    }

    double getExpiration() const noexcept {
      return start + length / sTimeDividor;
    }

    bool operator<(Timer const &other) const noexcept {
      return getExpiration() < other.getExpiration();
    }
  } *mTimers;

  ClockToUse mClockToUse = ClockToUse::Steady;

  int64_t mWatchdogStart;
  int64_t mWatchdogLength;

  /// Can be anything from 1 up
  static double sTimeDividor;

  /// True if only watchdog events are considered
  bool    mPaused     = false;

  std::optional<int64_t> mPauseStart;

  /// cEmptyIndex if the list is empty
  int32_t mStartIndex = cEmptyIndex;

  /// cEmptyIndex if the list is empty
  int32_t mEndIndex   = cEmptyIndex;
  int32_t mLength     = 0;
  int32_t mMaxLength  = 0;

  int32_t getLength() const noexcept {
    return mLength;
  }

public:
  TimerManager(int32_t const aMaxLength, int32_t const aWatchdogLength) noexcept;

  ~TimerManager() noexcept {
    delete[] mTimers;
  }

  operator bool() const noexcept {
    return mTimers != nullptr;
  }

  std::optional<int64_t> getEarliestValidTimeoutLength() const noexcept;

  void cancelAll() noexcept {
    mStartIndex = mEndIndex = cEmptyIndex;
    mLength = 0;
  }

  void setTimeDividor(double const aTimeDividor) noexcept;

  void pause() noexcept {
    if(!mPauseStart) {
      mPauseStart = now();
    }
    else { // nothing to do
    }
  }

  void resume() noexcept;

  int64_t now() const noexcept;

  void keepPattingWatchdog() noexcept;

  /// Creates an action timer from now on
  /// @param aLength planned delay in us
  /// @param aAction action as int to delay
  void schedule(int64_t const aLength, int32_t const aAction) {
    schedule(Timer(now(), aLength, aAction));
  }

  /// May only be called if the actual timer expired, so there is something to return.
  /// Pops the first event only if it is expired regarding now(). Otherwise the return optional is empty.
  /// cPatWatchdog return value means watchdog timer expiration.
  std::optional<int32_t> pop() noexcept;

private:
  void schedule(Timer const &aTimer);

  template <typename Chrono>
  std::optional<int32_t> measureShortestThreadSleep() noexcept {
    std::optional<int32_t> result;
    if(Chrono::is_steady) {
      int32_t max = 0;
      for(int i = 0; i < cMeasureShortestThreadSleepRepeats; ++i) {
        int32_t upper = std::micro::den / std::milli::den * cMeasureShortestThreadSleepMaxMillis;
        int32_t lower = std::micro::num;
        int32_t diff;
        while(upper > lower + 1) {
          int32_t delay = (upper + lower) / 2;
          typename Chrono::time_point before = Chrono::now();
          std::this_thread::sleep_for(std::chrono::microseconds(delay));
          typename Chrono::time_point after = Chrono::now();
          diff = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
          if(diff > delay * cMeasureShortestThreadSleepExcess) {
            lower = delay;
          }
          else {
            upper = delay;
          }
        }
        if(max < diff) {
          max = diff;
        }
      }
      result = max;
    }
    return result;
  }
};

#endif // DISHWASHER_TIMER_INCLUDED
