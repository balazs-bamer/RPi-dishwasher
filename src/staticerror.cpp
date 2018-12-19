#include "staticerror.h"
#include "dishwash.h"

using namespace std;

bool StaticError::shouldBeQueued(const Event &e) const noexcept {
    switch(e.getType()) {
    case EventType::MLeak:
    case EventType::MCircCurrent:
    case EventType::MDrainCurrent:
    case EventType::MWaterLevel:
    case EventType::MTemperature:
        return true;
    default:
        return false;
    }
}

void StaticError::process(const Event &event) noexcept {

}

void StaticError::process(Error error) noexcept {

}
