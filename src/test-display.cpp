#include <curses.h>
#include <string.h>

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
    string del("                  ");
    mvaddstr(1, 1, programNames[static_cast<uint32_t>(program) + 1]);
    mvaddstr(3, 1, stateNames[static_cast<uint32_t>(state) + 1]);
    string buf("   Water level: ");
    buf += to_string(waterLevel) + del;
    mvaddstr(5, 1, buf.c_str());
    buf =      "   Temperature: ";
    buf += to_string(temperature) + del;
    mvaddstr(6, 1, buf.c_str());
    buf =      "  Circ current: ";
    buf += to_string(circCurrent) + del;
    mvaddstr(7, 1, buf.c_str());
    buf =      " Drain current: ";
    buf += to_string(drainCurrent) + del;
    mvaddstr(8, 1, buf.c_str());

    buf =      "          Door: ";
    buf += Event::strDoor[static_cast<uint32_t>(door) + 1] + del;
    mvaddstr(10, 1, buf.c_str());
    buf =      "          Salt: ";
    buf += Event::strDoor[static_cast<uint32_t>(salt) + 1] + del;
    mvaddstr(11, 1, buf.c_str());
    buf =      "          Leak: ";
    buf += Event::strDoor[static_cast<uint32_t>(leak) + 1] + del;
    mvaddstr(12, 1, buf.c_str());
    buf =      " Spray contact: ";
    buf += Event::strDoor[static_cast<uint32_t>(sprayContact) + 1] + del;
    mvaddstr(13, 1, buf.c_str());
    buf =      "Spray position: ";
    buf += Event::strDoor[static_cast<uint32_t>(sprayPosition) + 1] + del;
    mvaddstr(14, 1, buf.c_str());

/* TODO    uint8_t actuate = 0;
    uint32_t remainingTime = 0;*/
    refresh();
}

void Display::process(const Event &event) noexcept {
    switch(event.getType()) {
    case EventType::MeasuredDoor:
        door = event.getDoor();
        break;
    case EventType::MSalt:
        salt = event.getSalt();
        break;
    case EventType::MeasuredSpray: {
        SprayChangeState st = event.getSpray();
        if(st == SprayChangeState::Off || st == SprayChangeState::On) {
            sprayContact = st;
        }
        else {
            sprayPosition = st;
        }
        break;
    }
    case EventType::MeasuredLeak:
        leak = event.getLeak();
        break;
    case EventType::MeasuredCircCurrent:
        circCurrent = event.getCircCurrent();
        break;
    case EventType::MeasuredDrainCurrent:
        drainCurrent = event.getDrainCurrent();
        break;
    case EventType::MeasuredWaterLevel:
        waterLevel = event.getWaterLevel();
        break;
    case EventType::MeasuredTemperature:
        temperature = event.getTemperature();
        break;
    case EventType::Actuate: {
        int32_t raw = static_cast<int32_t>(event.getActuate());
        if(raw > 0) {
            if(raw & 1) { // turn on
                actuate |= 1 << (raw / 2);
            }
            else { // turn off
                actuate &= ~static_cast<uint8_t>(1 << (raw / 2));
            }
        }
        break;
    }
    case EventType::Program:
        program = event.getProgram();
        break;
    case EventType::State:
        state = event.getState();
        break;
    case EventType::RemainingTime:
        remainingTime = event.getTime();
        break;
    default:
        break;
    }
}
