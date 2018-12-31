#include "display.h"
#include "dishwash.h"

using namespace std;

constexpr char Display::errorMessages[][cStrLengthLong];
constexpr char Display::programNames[][cStrLengthShort];
constexpr char Display::stateNames[][cStrLengthShort];

void Display::process(Event const &aEvent) noexcept {

}

void Display::process(int32_t const aExpired) noexcept {


