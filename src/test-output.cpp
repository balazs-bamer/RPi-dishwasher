#include "output.h"
#include "dishwash.h"

using namespace std;

bool Output::shouldBeQueued(const Event &e) const noexcept {
    switch(e.getType()) {
    case EventType::MeasuredDoor:
    case EventType::Actuate:
        return true;
    default:
        return false;
    }
}

void Output::process(const Event &event) noexcept {

}

void Output::process(Error error) noexcept {

}
