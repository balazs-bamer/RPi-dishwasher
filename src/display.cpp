#include "display.h"
#include "dishwash.h"

using namespace std;

constexpr char Display::errorMessages[][12];
constexpr char Display::programNames[][6];
constexpr char Display::stateNames[][6];

Display::Display(Dishwasher &d) : Component(d) {
}

bool Display::shouldBeQueued(const Event &e) const noexcept {
    switch(e.getAggregateType()) {
    case EventType::Measured:
    case EventType::Actuate:
    case EventType::Program:
    case EventType::Display:
        return true;
    default:
        return false;
    }
}

void Display::processAtEachPoll() noexcept {

}

void Display::process(const Event &event) noexcept {

}
