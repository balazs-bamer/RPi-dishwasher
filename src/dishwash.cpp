#include "dishwash.h"
#include <signal.h>

void signalHandler(int s) {
  Dishwasher::stop();
}

std::atomic<bool> Dishwasher::sKeepRunning;

Dishwasher::Dishwasher(std::initializer_list<Component*> aComponents)
  : mComponents(aComponents) {
  sKeepRunning.store(true);
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
}

void Dishwasher::run() {
  uint32_t startCount = 0;
  try {
    for(auto i : mComponents) {
      i->start(this);
      ++startCount;
    }
  }
  catch(std::exception &e) {
// TODO log
  }
  if(startCount == mComponents.size()) {
    while(sKeepRunning.load()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  while(startCount > 0) {
    mComponents[--startCount]->stop();
  }
}

void Dishwasher::send(Component *aOrigin, Event const &aEvent) noexcept {
  for(auto i : mComponents) {
    if(i != aOrigin) {
      i->queueEvent(aEvent);
    }
    else { // nothing to do
    }
  }
}
