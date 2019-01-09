#include <curses.h>

#include "input.h"
#include "dishwash.h"
#include "test-keyboard.h"

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

extern char const cButtonsProgram[];
extern char const cButtonsFault[];
extern char const cButtonsTimerFactor[];
extern int32_t const cTimerFactors[];

void Input::process(Event const &aEvent) noexcept {
  EventType type = aEvent.getType();
  if(type == EventType::KeyPressed) {
    int32_t keyPressed = aEvent.getIntValue();
    auto found = std::find(cButtonsProgram, cButtonsProgram + sizeof(cButtonsProgram), keyPressed);
    if(found < cButtonsProgram + sizeof(cButtonsProgram)) {
      send(Event(static_cast<Program>(found - cButtonsProgram)));
    }
    else { // nothing to do
    }
  }
  else { // nothing to do
  }
}

void Input::process(int32_t const aTimeEvent) noexcept {
}
