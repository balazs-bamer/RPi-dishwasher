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
}

void Input::process(Event const &aEvent) noexcept {
}

void Input::process(int32_t const aTimeEvent) noexcept {
}
