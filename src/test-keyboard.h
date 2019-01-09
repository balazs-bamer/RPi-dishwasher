#ifndef DISHWASHER_TEST_KEYBOARD_INCLUDED
#define DISHWASHER_TEST_KEYBOARD_INCLUDED

constexpr int32_t cButtonsProgramCount     = 11;
constexpr int32_t cButtonsFaultCount       = 27;
constexpr int32_t cButtonsTimerFactorCount = 10;

extern char const cButtonsProgram[cButtonsProgramCount + 1];
extern char const cButtonsFault[cButtonsFaultCount + 1];
extern char const cButtonsTimerFactor[cButtonsTimerFactorCount + 1];
extern int32_t const cTimerFactors[cButtonsTimerFactorCount];

#endif
