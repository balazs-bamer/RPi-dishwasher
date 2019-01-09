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
  for(auto i : mComponents) {
    i->start(this);
    ++startCount;
  }
  if(startCount == mComponents.size()) {
    while(sKeepRunning.load()) {
      std::this_thread::sleep_for(std::chrono::microseconds(cSleepWait));
    }
  }
  else { // nothing to do
  }
  while(startCount > 0) {
    mComponents[--startCount]->stop();
  }
  std::this_thread::sleep_for(std::chrono::microseconds(cSleepFinish));
}

void Dishwasher::send(Component *aOrigin, Event const &aEvent) noexcept {
  if(aEvent.getType() == EventType::KeyPressed) {
    Log::i() << nowtech::LogApp::cEvent << aEvent.getTypeConstStr() << ':' << aEvent.getValueConstStr() << " (" << static_cast<char>(aEvent.getIntValue()) << ')' << Log::end;
  }
  else {
    Log::i() << nowtech::LogApp::cEvent << aEvent.getTypeConstStr() << ':' << aEvent.getValueConstStr() << " (" << aEvent.getIntValue() << ')' << Log::end;
  }
  for(auto i : mComponents) {
    if(i != aOrigin) {
      i->queueEvent(aEvent);
    }
    else { // nothing to do
    }
  }
}
