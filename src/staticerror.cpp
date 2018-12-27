#include "staticerror.h"
#include "dishwash.h"

using namespace std;

bool StaticError::shouldBeQueued(const Event &e) const noexcept {
    switch(e.getType()) {
    case EventType::MeasuredLeak:
    case EventType::MeasuredCircCurrent:
    case EventType::MeasuredDrainCurrent:
    case EventType::MeasuredWaterLevel:
    case EventType::MeasuredTemperature:
        return true;
    default:
        return false;
    }
}

void StaticError::process(const Event &event) noexcept {

}

void StaticError::process(Error error) noexcept {

}
