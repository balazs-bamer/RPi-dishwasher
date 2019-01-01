#include "display.h"

#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <cctype>

constexpr char Display::errorMessages[][12];
constexpr char Display::programNames[][6];
constexpr char Display::stateNames[][6];

Display::Display() : Component() {
  ::setlocale(LC_ALL, "");
  ::initscr();
  ::cbreak();
  ::nodelay(stdscr, true);
  ::noecho();
  ::nonl();
  ::intrflush(stdscr, FALSE);
  ::keypad(stdscr, TRUE);
}

Display::~Display() noexcept {
  ::endwin();
}

void Display::refresh() noexcept {
  int keyPressed = ::getch();
  if(keyPressed != ERR && isalnum(keyPressed)) {
    send(EventType::KeyPressed, static_cast<int32_t>(keyPressed));
  }
  else { // nothing to do
  }
  if(mNeedsRefresh) { // Will be enough to display the remaining time as well, because there are frequent measurements.
    // mvprintw(y, x, string);/* Move to (y, x) then print string     */
    // mvaddstr(y, x, string)
    ::refresh();
    mNeedsRefresh = false; // TODO handle countdown
  }
}
