#include "display.h"
#include "dishwash.h"
#include "test-keyboard.h"

#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <cctype>
#include <algorithm>

constexpr char Display::errorMessages[][12];
constexpr char Display::programNames[][6];
constexpr char Display::stateNames[][6];

int32_t const cMinColumns       = 100;
int32_t const cMinRows          =  31;
int32_t const cSensorCount      =   8;
int32_t const cTimerFactorCount =  10;
int32_t const cFaultCount       =  27;

struct Location {
  int32_t x, y;
};

Location const cStartSensorTexts        = {  3,  4 };
Location const cStartSensorValues       = { 17,  4 };
Location const cStartActuators          = {  3, 14 };
Location const cStartStateValues        = { 16, 28 };
Location const cStartErrorValues        = { 28,  4 };
Location const cStartProgramTexts       = { 48,  4 };
Location const cStartProgramButtons     = { 58,  4 };
Location const cStartTimerFactorTexts   = { 48, 17 };
Location const cStartTimerFactorButtons = { 58, 17 };
Location const cStartFaultTexts         = { 63,  2 };
Location const cStartFaultButtons       = { 98,  2 };

struct LocationText {
  int32_t x, y;
  char const * text;
};

LocationText const cTitleMain         = { 10,  1, "Dishwasher emulator" };
LocationText const cTitleSensor       = {  1,  3, "Sensors" };
LocationText const cTitleActuator     = {  1, 13, "Actuators" };
LocationText const cTitleStates       = {  1, 27, "Machine states" };
LocationText const cTitleErrors       = { 25,  3, "Errors" };
LocationText const cTitlePrograms     = { 45,  3, "Programs" };
LocationText const cTitleTimerFactors = { 45, 16, "Speedup factors" };
LocationText const cTitleFaults       = { 60,  1, "Fault injection" };
LocationText const cStatesProgram     = {  3, 28, "Program:" };
LocationText const cStatesState       = {  3, 29, "State:" };
LocationText const cStatesRemaining   = {  3, 30, "Mins left:" };
LocationText const cStatesTimerFactor = {  3, 31, "Speedup:" };

char const cErrorMessages[][20] = {
                                  "I2C communication  ",
                                  "Programmer         ",
                                  "Queue full         ",
                                  "No water           ",
                                  "Overfill           ",
                                  "Drain fail         ",
                                  "Leakage            ",
                                  "No pressure signal ",
                                  "Bad pressure signal",
                                  "Unstable pressure  ",
                                  "Circulate overload ",
                                  "Circulate connect  ",
                                  "Circulate stuck    ",
                                  "Drain overload     ",
                                  "Drain connection   ",
                                  "Drain stuck        ",
                                  "No heating         ",
                                  "Overheat           ",
                                  "Bad temperature    ",
                                  "Spray selector     "
                                };
char const cFaultNames[cFaultCount][40] = {
                                    "Door sensor stuck at 0           ", // Q
                                    "Door sensor stuck at 1           ", // W
                                    "Salt sensor stuck at 0           ", // E
                                    "Salt sensor stuck at 1           ", // R
                                    "Spray sensor stuck at 0          ", // T
                                    "Spray sensor stuck at 1          ", // Y
                                    "Circulate current sensor stuck at", // U
                                    "Drain current sensor stuck at    ", // I
                                    "Water level sensor stuck at      ", // O
                                    "Temperature sensor stuck at      ", // P
                                    "I2C communication failure        ", // {
                                    "Shutdown actuator stuck at 0     ", // }
                                    "Shutdown actuator stuck at 1     ", // A
                                    "Heat actuator stuck at 0         ", // S
                                    "Heat actuator stuck at 1         ", // D
                                    "Drain actuator stuck at 0        ", // F
                                    "Drain actuator stuck at 1        ", // G
                                    "Fill actuator stuck at 0         ", // H
                                    "Fill actuator stuck at 1         ", // J
                                    "Regenerate actuator stuck at 0   ", // K
                                    "Regenerate actuator stuck at 1   ", // L
                                    "Detergent actuator stuck at 0    ", // :
                                    "Detergent actuator stuck at 1    ", // "
                                    "Circulate actuator stuck at 0    ", // Z
                                    "Circulate actuator stuck at 1    ", // X
                                    "Spray actuator stuck at 0        ", // C
                                    "Spray actuator stuck at 1        "  // V
                                  };
char const cSensorNames[cSensorCount][20] = {
                                  "Temperature:",
                                  "Drain current:",
                                  "Water level:",
                                  "Salt:",
                                  "Circ current:",
                                  "Spray:",
                                  "Door:",
                                  "Leak:"
                                };

char const cButtonsProgram[cButtonsProgramCount + 1]         = "  sdrfymahi";
char const cButtonsFault[cButtonsFaultCount + 1]             = "QWERTYUIOP{}ASDFGHJKL:\"ZXCV";
char const cButtonsTimerFactor[cButtonsTimerFactorCount + 1] = "1234567890";
int32_t const cTimerFactors[cButtonsTimerFactorCount]        = { 1, 2, 3, 5, 8, 13, 22, 36, 60, 100 }; // available via buttons from 1-9, 0

static void writeStaticContent() {
  if(getmaxy(stdscr) < cMinRows || getmaxx(stdscr) < cMinColumns) {
    throw std::out_of_range("Too small screen");
  }
  else { // nothing to do
  }
  mvaddstr(cTitleMain.y,         cTitleMain.x,         cTitleMain.text);
  mvaddstr(cTitleSensor.y,       cTitleSensor.x,       cTitleSensor.text);
  mvaddstr(cTitleActuator.y,     cTitleActuator.x,     cTitleActuator.text);
  mvaddstr(cTitleStates.y,       cTitleStates.x,       cTitleStates.text);
  mvaddstr(cTitleErrors.y,       cTitleErrors.x,       cTitleErrors.text);
  mvaddstr(cTitlePrograms.y,     cTitlePrograms.x,     cTitlePrograms.text);
  mvaddstr(cTitleTimerFactors.y, cTitleTimerFactors.x, cTitleTimerFactors.text);
  mvaddstr(cTitleFaults.y,       cTitleFaults.x,       cTitleFaults.text);
  mvaddstr(cStatesProgram.y,     cStatesProgram.x,     cStatesProgram.text);
  mvaddstr(cStatesState.y,       cStatesState.x,       cStatesState.text);
  mvaddstr(cStatesRemaining.y,   cStatesRemaining.x,   cStatesRemaining.text);
  mvaddstr(cStatesTimerFactor.y, cStatesTimerFactor.x, cStatesTimerFactor.text);
  for(int32_t i = 0; i < cSensorCount; ++i) {
    mvaddstr(cStartSensorTexts.y + i, cStartSensorTexts.x, cSensorNames[i]);
  }
  for(int32_t i = 0; i < static_cast<int32_t>(Program::Count); ++i) {
    mvaddstr(cStartProgramTexts.y + i, cStartProgramTexts.x, Event::cStrProgram[i]);
  }
  for(int32_t i = 0; i < static_cast<int32_t>(Program::Count); ++i) {
    mvaddch(cStartProgramButtons.y + i, cStartProgramButtons.x, cButtonsProgram[i]);
  }
  for(int32_t i = 0; i < cTimerFactorCount; ++i) {
    mvprintw(cStartTimerFactorTexts.y + i, cStartTimerFactorTexts.x, "%3d", cTimerFactors[i]);
  }
  for(int32_t i = 0; i < cTimerFactorCount; ++i) {
    mvaddch(cStartTimerFactorButtons.y + i, cStartTimerFactorButtons.x, cButtonsTimerFactor[i]);
  }
  for(int32_t i = 0; i < cFaultCount; ++i) {
    mvaddstr(cStartFaultTexts.y + i, cStartFaultTexts.x, cFaultNames[i]);
  }
  for(int32_t i = 0; i < cFaultCount; ++i) {
    mvaddch(cStartFaultButtons.y + i, cStartFaultButtons.x, cButtonsFault[i]);
  }
}

Display::Display() : Component() {
  ::setlocale(LC_ALL, "");
  ::initscr();
  ::raw();
  ::nodelay(stdscr, true);
  ::noecho();
  ::nonl();
  ::intrflush(stdscr, FALSE);
  ::keypad(stdscr, TRUE);
  ::writeStaticContent();
  refresh();
}

Display::~Display() noexcept {
  ::endwin();
}

void Display::refresh() noexcept {
  static constexpr int32_t cCtrlC = 3;
  int32_t keyPressed = ::getch();
  if(std::find(cButtonsProgram, cButtonsProgram + sizeof(cButtonsProgram), keyPressed) != cButtonsProgram + sizeof(cButtonsProgram)) {
    send(EventType::KeyPressed, static_cast<int32_t>(keyPressed));
  }
  else if(keyPressed == cCtrlC) {
    Dishwasher::stop();
  }
  else { // nothing to do
  }
  if(mNeedsRefresh) { // Will be enough to display the remaining time as well, because there are frequent measurements.
    mvaddstr(cStartSensorValues.y + 6, cStartSensorValues.x, Event::cStrDoorState[static_cast<int32_t>(mDoor) + 1]);
    mvaddstr(cStartSensorValues.y + 3, cStartSensorValues.x, Event::cStrOnOffState[static_cast<int32_t>(mSalt) + 1]);
    mvaddstr(cStartSensorValues.y + 5, cStartSensorValues.x, Event::cStrOnOffState[static_cast<int32_t>(mSprayContact) + 1]);
    mvaddstr(cStartSensorValues.y + 7, cStartSensorValues.x, Event::cStrOnOffState[static_cast<int32_t>(mLeak) + 1]);
    mvprintw(cStartSensorValues.y + 4, cStartSensorValues.x, "%3d", mCircCurrent);
    mvprintw(cStartSensorValues.y + 1, cStartSensorValues.x, "%3d", mDrainCurrent);
    mvprintw(cStartSensorValues.y + 2, cStartSensorValues.x, "%3d", mWaterLevel);
    mvprintw(cStartSensorValues.y, cStartSensorValues.x, "%3d", mTemperature);
    int32_t errorSoFar = mErrorSoFar.load();
    for(int32_t i = 0; i < 31; ++i) {
      if(errorSoFar & (1 << i)) {
        mvaddstr(cStartErrorValues.y + i, cStartErrorValues.x, cErrorMessages[i]);
      }
      else { // nothing to do
      }
    }
    for(uint8_t i = 0; i < 8; ++i) {
      mvaddstr(cStartActuators.y + i, cStartActuators.x, Event::cStrActuate[i * 2 + 1 + (mActuate & 1 << i ? 1 : 0)]);
    }
    mvaddstr(cStartStateValues.y, cStartStateValues.x, Event::cStrProgram[static_cast<int32_t>(mProgram) + 1]);
    mvaddstr(cStartStateValues.y + 1, cStartStateValues.x, Event::cStrMachineState[static_cast<int32_t>(mState) + 1]);
    mvprintw(cStartStateValues.y + 2, cStartStateValues.x, "%3d", mRemainingTime);
    mvprintw(cStartStateValues.y + 3, cStartStateValues.x, "%3d", mTimerFactor);
    ::refresh();
    mNeedsRefresh = false; // TODO handle countdown
  }
}
