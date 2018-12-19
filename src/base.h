#ifndef  DISHWASHER_BASE_INCLUDED
#define  DISHWASHER_BASE_INCLUDED

#include "dishwash-config.h"
#include "bancopymove.h"
#include "timer.h"

#include <chrono>
#include <atomic>
#include <thread>
#include <functional>
#include <condition_variable>
#include <boost/lockfree/queue.hpp>

enum class DoorState : int32_t { Invalid = -1, Open, Closed };

/** Used to turn on/off regular spray change and sign spray contact state.
 * Used internally to remember position. */
enum class SprayChangeState : int32_t { Invalid = -1, Upper, Lower, Both };

enum class OnOffState : int32_t { Invalid = -1, Off, On };

// TODO first check bare logic operation, and if it is correct, implement error handling
enum class Error : int32_t {     // raised in
  Invalid         = -1,        // -
  None            =  0,        // -
  I2C             =  1 <<  0u, // Output
  Programmer      =  1 <<  1u, // several
  Queue           =  1 <<  2u, // Component
  NoWater         =  1 <<  3u, // Automat
  OverFill        =  1 <<  4u, // StaticError
  NoDrain         =  1 <<  5u, // Automat
  Leak            =  1 <<  6u, // StaticError
  NoSignal        =  1 <<  7u, // Input
  InvalidSignal   =  1 <<  8u, // Input
  UnstableSignal  =  1 <<  9u, // Input
  CircOverload    =  1 << 10u, // StaticError
  CircConnector   =  1 << 11u, // Automat
  CircRelayStuck  =  1 << 12u, // Automat
  DrainOverload   =  1 << 13u, // StaticError
  DrainConnector  =  1 << 14u, // Automat
  DrainRelayStuck =  1 << 15u, // Automat
  NoHeat          =  1 << 16u, // Automat
  Overheat        =  1 << 17u, // StaticError
  InvalidTemp     =  1 << 18u, // Input
  SpraySelect     =  1 << 19u  // Automat
};

enum class Actuate : int32_t { // comes from
  Invalid         =    -1,
  Shutdown0, Shutdown1,      // Logic
  Heat0, Heat1,              // Automat
  Drain0, Drain1,            // Automat
  Fill0, Fill1,              // Automat
  Regenerate0, Regenerate1,  // Automat
  Detergent0, Detergent1,    // Logic
  Circ0, Circ1,              // Automat
  Spray0, Spray1             // Automat
};

enum class Program : int32_t {
  Invalid = -1, None, Stop, Drain, Rinse, Fast, FastDry, Middle, All, Hot, Intensive, Cook, Count
};

enum class MachineState : int32_t {
  Invalid = -1, Idle, Drain, Resin, PreWash, Wash, Rinse1, Rinse2, Rinse3, Dry, Shutdown, Count
};

enum class EventType : int32_t {
  Invalid = -1,         // [sent by]                     [value type]
  MeasuredDoor,         // Input                         DoorState
  MeasuredSalt,         // Input                         OnOffState
  MeasuredSpray,        // Input                         OnOffState
  MeasuredLeak,         // Input                         OnOffState
  MeasuredCircCurrent,  // Input                         int32_t
  MeasuredDrainCurrent, // Input                         int32_t
  MeasuredWaterLevel,   // Input                         int32_t
  MeasuredTemperature,  // Input                         int32_t
  Error,                // any                           Error
  DesiredSpray,         // Logic                         SprayChangeState
  DesiredCirc,          // Logic                         OnOffState
  DesiredWaterLevel,    // Logic                         int32_t
  DesiredTemperature,   // Logic                         int32_t
  DesiredResinWash,     // Logic                         OnOffState
  Actuate,              // Logic, Automat (see above)    Actuate
  Program,              // Input                         Program
  MachineState,         // Logic                         MachineState
  RemainingTime,        // Logic only on program start   int32_t
  TimeFactorChanged,    // Dishwasher                    int32_t
  Pause                 // Input                         OnOffState
};

class Event final {
public:
  static constexpr int32_t cStringSize = 16;
  static constexpr int32_t cInvalid    = -1;

  static constexpr char cStrInvalid[cStringSize] = "Invalid";
  static constexpr char cStrDoor[][cStringSize] = {"Invalid", "Open", "Closed"};
  static constexpr char cStrSprayChangeState[][cStringSize] = { "Invalid", "Upper", "Lower", "Both" };
  static constexpr char cStrOnOffState[][cStringSize] = { "Invalid", "Off", "On" };

  static constexpr char cStrActuate[][cStringSize] = { "Invalid", "Shutdown0", "Shutdown1", "Heat0", "Heat1",
                              "Drain0", "Drain1", "Fill0", "Fill1", "Regenerate0", "Regenerate1", "Detergent0",
                              "Detergent1", "Circ0", "Circ1", "Spray0", "Spray1" };

  static constexpr char cStrProgram[][cStringSize] = { "Invalid", "None", "Stop", "Drain", "Rinse", "Fast",
                              "FastDry", "Middle", "All", "Hot", "Intensive", "Cook", "Count" };

  static constexpr char cStrMachineState[][cStringSize] = { "Invalid", "Idle", "Drain", "Resin", "PreWash", "Wash",
                              "Rinse1", "Rinse2", "Rinse3", "Dry", "Shutdown", "Count" };

  static constexpr char cStrError[][cStringSize] = { "Invalid", "None", "I2C", "Programmer", "Queue", "NoWater",
                              "OverFill", "NoDrain", "Leak", "NoSignal", "InvalidSignal", "UnstableSignal",
                              "CircOverload", "CircConnector", "CircRelayStuck", "DrainOverload", "DrainConnector",
                              "DrainRelayStuck", "NoHeat", "Overheat", "InvalidTemp", "SpraySelect" };

  static constexpr char cStrEventType[][cStringSize] = { "Invalid", "MeasuredDoor", "MeasuredSalt", "MeasuredSpray",
                              "MeasuredLeak", "MeasuredCrcCurr", "MeasuredDrnCurr", "MeasuredWtrLvl",
                              "MeasuredTemp", "Error", "DesiredSpray", "DesiredCirc", "DesiredWaterLvl",
                              "DesiredTemp", "DesiredResinWsh", "Actuate", "Program", "MachineState",
                              "RemainingTime", "TimeFactChanged", "Pause" };

private:
  EventType mType = EventType::Invalid;

  union {
    DoorState        mDoor;
    OnOffState       mOnOff;
    /// water level: mm, temperature: Celsius, current: mA, time: s
    int32_t          mIntValue;
    Error            mError;
    SprayChangeState mSprayChange;
    Actuate          mActuate;
    Program          mProgram;
    MachineState     mMachineState;
  };

public:
  Event() noexcept {}
    // if possible, allow calls like Component::send(DoorState::Open)
  Event(DoorState const aArg) noexcept : mType(EventType::MeasuredDoor), mDoor(aArg) {
  }

  Event(EventType const aType, OnOffState const aArg) noexcept;
  Event(EventType const aType, int32_t const aArg) noexcept;

  Event(Error const aArg) noexcept : mType(EventType::Error), mError(aArg) {
  }

  Event(SprayChangeState const aArg) noexcept : mType(EventType::DesiredSpray), mSprayChange(aArg) {
  }

  Event(Actuate const aArg) noexcept : mType(EventType::Actuate), mActuate(aArg) {
  }

  Event(Program const aArg) noexcept : mType(EventType::Program), mProgram(aArg) {
  }

  Event(MachineState const aArg) noexcept : mType(EventType::MachineState), mMachineState(aArg) {
  }

  EventType getType() const noexcept {
    return mType;
  }

  DoorState getDoor() const noexcept {
    return mType == EventType::MeasuredDoor ? mDoor : DoorState::Invalid;
  }

  OnOffState getOnOff() const noexcept;
  int32_t getIntValue() const noexcept;

  Error getError() const noexcept {
    return mType == EventType::Error ? mError : Error::Invalid;
  }

  SprayChangeState getSpray() const noexcept {
    return mType == EventType::DesiredSpray ? mSprayChange : SprayChangeState::Invalid;
  }

  Actuate getActuate() const noexcept {
    return mType == EventType::Actuate ? mActuate : Actuate::Invalid;
  }

  Program getProgram() const noexcept {
    return mType == EventType::Program ? mProgram : Program::Invalid;
  }

  MachineState getMachineState() const noexcept {
    return mType == EventType::MachineState ? mMachineState : MachineState::Invalid;
  }

  char const * getValueConstStr() const noexcept;

  char const * getTypeConstStr() const noexcept {
    return cStrEventType[static_cast<int32_t>(mType) + 1];
  }

private:
  int32_t getErrorStrIndex() const noexcept;
};


class Dishwasher;

class Component : public BanCopyMove {
  static constexpr int32_t cWatchdogPatInterval = 100000;  // 0.1s
  static constexpr int32_t cMessageQueueSize    =    128u;
  static constexpr int32_t cNoError             =      0;

  std::thread mThread;

  boost::lockfree::queue<Event> mQueue;

  /** All errors are ORed together here. */
  std::atomic<int32_t> mErrorSoFar = 0;

  /// A MachineState::Shutdown fill set it false if needed
  bool mKeepRunning;

  TimerManager mTimerManager;
  std::condition_variable mConditionVariable;

protected:
  Dishwasher &mDishwasher;

private:
  /** This and derived constructors may throw exception if some library or hardware component fails.
  This and derived constructors will initialize all the needed libraries and hardware. */
  Component(Dishwasher &aDishwasher) : mQueue(cMessageQueueSize), mTimerManager(getTimerCount()), mDishwasher(aDishwasher) {
    mKeepRunning = mQueue.is_lock_free() && mTimerManager;
  }

protected:
  virtual int32_t getTimerCount() = 0;

public:
  virtual ~Component() noexcept {}

  Component(Component const &) = delete;
  Component(Component &&) = default;
  Component& operator=(Component const &) = delete;
  Component& operator=(Component &&) = default;

  void start() {
    mThread = std::thread(&Component::run, this);
  }

  void stop() {
    mThread.join();
  }

  /// Queues the event if it is interesting for this class.
  void queueEvent(Event const &) noexcept;

protected:
  /// Returns true if the thread should exit on error, false otherwise.
  virtual bool shouldHaltOnError() const noexcept = 0;

  /// Does not have to check for errors or timed events, since they are handle independently.
  virtual bool shouldBeQueued(const Event &) const noexcept = 0;

  void schedule(int64_t const aDelay, int32_t const aTimerAction) noexcept {
    mKeepRunning = mKeepRunning && mTimerManager.push(aDelay, aTimerAction);
  }

  void send(Event const &) noexcept;

  void send(EventType aType, int32_t aValue) noexcept {
    send(Event(aType, aValue));
  }

  /// By the time we get here, all other components are initialized and ready to start.
  void run() noexcept;

  /// Processes an interesting message.
  virtual void process(Event const &) noexcept = 0;

  /// Processes a timer event. Implementation will cast it to the actual enum class.
  virtual void process(int32_t const &) noexcept = 0;

  /// Handles it here and sends to other components.
  void raise(Error const aError) noexcept;

  bool raise(bool const aCondition) noexcept;
};

#endif // DISHWASHER_BASE_INCLUDED
