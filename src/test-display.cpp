#include <curses.h>
#include <string.h>

#include "display.h"

using namespace std;

constexpr char Display::errorMessages[][12];
constexpr char Display::programNames[][6];
constexpr char Display::stateNames[][6];

Display::Display() : Component() {
}

Display::~Display() noexcept {
}
