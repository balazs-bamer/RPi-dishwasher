#include <curses.h>

#include "input.h"
#include "dishwash.h"

// TODO
#include <iostream>

using namespace std;

Input::Input() : Component() {
}

Input::~Input() noexcept {
}

bool Input::shouldBeQueued(Event const &aEvent) const noexcept {
  switch(aEvent.getType()) {
  case EventType::KeyPressed:
      return true;
  default:
      return false;
  }
}

static constexpr int32_t cKeyQuit = 'Q';

void Input::process(Event const &aEvent) noexcept {
  EventType type = aEvent.getType();
  if(type == EventType::KeyPressed) {
    int32_t keyPressed = aEvent.getIntValue();
    if(keyPressed == cKeyQuit) {
      raise(Error::Quit);
    }
    else { // nothing to do
    }
  }
  else { // nothing to do
  }
}

void Input::process(int32_t const aTimeEvent) noexcept {
}
