#ifndef DISHWASHER_DISHWASH_INCLUDED
#define DISHWASHER_DISHWASH_INCLUDED

#include "base.h"
#include <vector>

extern std::atomic<bool> keepRunning;

class Dishwasher final : public BanCopyMove {
  // us
  static constexpr int32_t cSleepWait   = 1000000;
  static constexpr int32_t cSleepFinish = 2000000;

  static std::atomic<bool> sKeepRunning;
  std::vector<Component*> mComponents;

public:
  /** This may throw exception if some library or hardware component fails. */
  Dishwasher(std::initializer_list<Component*>);

  ~Dishwasher() {
  }

  static void stop() noexcept {
    Log::i() << "Exiting..." << Log::end;
    sKeepRunning.store(false);
  }

  /** This may throw exceptions if thread creation fails. If every one succeeds,
  all the subsequent functions have no-throw guarantee. */
  void run();

  /** Sends the event to all components except for the originating one. */
  void send(Component *aOrigin, Event const &aEvent) noexcept;
};

#endif // DISHWASHER_DISHWASH_INCLUDED
