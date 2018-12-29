#include "input.h"
#include "logic`.h"
#include "automat.h"
#include "display.h"
#include "staticerror.h"
#include "output.h"
#include "dishwash.h"

#include <curses.h>
#include <unistd.h>

using namespace std;

void init() noexcept {
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  nodelay(stdscr, true);
  noecho();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
}

void done() noexcept {
  endwin();
}

int main(int argc, char **argv) {
  init();
  try {
    Input input;
    Logic logic;
    Automat automat;
    Display display;
    StaticError staticError;
    Output output;
    Dishwasher dishwash{input, logic, automat, display, staticError, output};
    dishwash.run();
  }
  catch(exception &e) {
    cerr << "main: " << e.what() << endl;
  }
  done();
  return 0;
}
