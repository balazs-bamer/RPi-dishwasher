#include "automat.h"
#include "dishwash.h"

using namespace std;

bool Automat::shouldBeQueued(const Event &e) const noexcept {
    switch(e.getType()) {
    case EventType::MSpray:
    case EventType::MWaterLevel:
    case EventType::MTemperature:
    case EventType::MCircCurrent:
    case EventType::MDrainCurrent:
    case EventType::DSpray:
    case EventType::DCirc:
    case EventType::DWaterLevel:
    case EventType::DTemperature:
    case EventType::DResinWash:
        return true;
    default:
        return false;
    }
}

void Automat::doResinWashSwitch(ResinWashState desired) noexcept {
    desiredResinWash = desired;
    if(desired == ResinWashState::On) {
        sprayChangeTransition = true;
        // great time to calibrate the spray changer mechanism
        timedEvents.schedule(FinishSearchSelectPosition, SELECT_SEARCH);
        send(Actuate::Spray1);
        measureStart = chrono::system_clock::now();
        if(waterLevel < WATER_LEVEL_FULL) {
            send(Actuate::Fill1);
        }
        send(Actuate::Drain1);
    }
    else {
        desiredWaterLevel = 0;
        desiredTemperature = 0;
        desiredCirculate = CirculateState::Off;
        desiredSprayChange = SprayChangeState::Off;
        send(Actuate::Fill0);
        send(Actuate::Drain0);
    }
}

void Automat::doResinWashWaterLevel(uint16_t level) noexcept {
    waterLevel = level;
    if(waterLevel < WATER_LEVEL_HALF) {
        send(Actuate::Fill1);
    }
    if(waterLevel >= WATER_LEVEL_FULL) {
        send(Actuate::Fill0);
    }
}

void Automat::doResinWashSpray(SprayChangeState spray) noexcept {
    if(sprayPosition == SprayChangeState::Invalid) {
        chrono::system_clock::time_point now = chrono::system_clock::now();
        raise(measuredTimeCount < maxMeasuredTimeCount && spray != sprayContact);
        sprayChangeTimes[measuredTimeCount++] = (chrono::duration_cast<chrono::milliseconds>(measureStart - now)).count();
        measureStart = now;
        sprayContact = spray;
    }
}

void Automat::doResinWashExpired(int32_t expired) noexcept {
    if(expired == FinishSearchSelectPosition) {
        timedEvents.schedule(DecelerateSearchSelectPosition, SELECT_DECELERATION);
    }
    else if(expired == DecelerateSearchSelectPosition) {
        if(measuredTimeCount < SELECT_SEARCH / 30000 * 6) {
            raise(Error::SpraySelect);
        }
        int shortPos = 0;
        while(shortPos < measuredTimeCount) {
            if(sprayChangeTimes[shortPos] < SELECT_UP_ON + SELECT_TOLERANCE) {
                break;
            }
            shortPos++;
        }
        int i;
        for(i = 0; shortPos < measuredTimeCount; ++i, ++shortPos) {
            sprayChangeTimes[i] = sprayChangeTimes[shortPos];
        }
        measuredTimeCount = i;
        if(measuredTimeCount >= 12) {
            for(i = 0; i < 6; i++) {
                sprayChangeTimes[i] = (sprayChangeTimes[i] + sprayChangeTimes[i + 6]) / 2;
            }
        }
        int rawPositionIndex = (measuredTimeCount - 1) % 6;
        if(raise(1 - rawPositionIndex % 2 == static_cast<int>(sprayContact))) {
          return;
        }
        switch(rawPositionIndex) {
        case 0:
        case 1:
            sprayPosition = SprayChangeState::Upper;
            break;
        case 2:
        case 3:
            sprayPosition = SprayChangeState::Lower;
            break;
        case 4:
        case 5:
            sprayPosition = SprayChangeState::Both;
        }
        sprayChangeTransition = false;
    }
}

void Automat::doResinWash(const Event &event) noexcept {
    EventType type = event.getType();
    if(type == EventType::DResinWash) {
        doResinWashSwitch(event.getResinWash());
    }
    else if(type == EventType::MWaterLevel) {
        doResinWashWaterLevel(event.getWaterLevel());
    }
    else if(type == EventType::MSpray) {
        doResinWashSpray(event.getSpray());
    }
    else if(type == EventType::Timer) {
        doResinWashExpired(event.getExpired());
    }
}

void Automat::doRegularExpired(int32_t expired) noexcept {
    if(expired == SprayChangeStop) {
        sprayChangeTransition = false;
        send(Actuate::Spray0);
        if(desiredCirculate == CirculateState::On) {
            send(Actuate::Circ1);
        }
        if(desiredSprayChange == SprayChangeState::On) {
            timedEvents.schedule(SprayChangePause, SELECT_KEEP_POSITION);
        }
    }
    else if(expired == SprayChangePause) {
        if(desiredSprayChange == SprayChangeState::On) {
            if(raise(((sprayPosition == SprayChangeState::Upper ||
                          sprayPosition == SprayChangeState::Lower ||
                          sprayPosition == SprayChangeState::Both) &&
                          sprayContact == SprayChangeState::On)) {
               return;
            }
            send(Actuate::Spray1);
            send(Actuate::Circ0);  // prevent circulation during transition
            sprayChangeTransition = true;
        }
    }
}

void Automat::doRegularWaterLevel(const Event &event) noexcept {
    // TODO DrainCurrent
    EventType type = event.getType();
    if(type == EventType::DWaterLevel) {
        if(raise(desiredCirculate != CirculateState::On)) {
          return;
        }
        desiredWaterLevel = event.getWaterLevel();
        if(waterLevel < desiredWaterLevel - WATER_LEVEL_HISTERESIS) {
            send(Actuate::Drain0);
            send(Actuate::Fill1);
            // TODO timeout
        }
        if(waterLevel > desiredWaterLevel + WATER_LEVEL_HISTERESIS) {
            send(Actuate::Drain1);
            send(Actuate::Fill0);
            // TODO timeout
        }
    }
    else if(type == EventType::MWaterLevel) {
        waterLevel = event.getWaterLevel();
        if(desiredCirculate == CirculateState::On) {
            return;
        }
        if(waterLevel < desiredWaterLevel - WATER_LEVEL_HISTERESIS) {
            send(Actuate::Drain0);
            send(Actuate::Fill1);
        }
        else if(waterLevel > desiredWaterLevel + WATER_LEVEL_HISTERESIS) {
            send(Actuate::Drain1);
            send(Actuate::Fill0);
        }
        else {
            // TODO cancel
        }
    }
}

void Automat::doRegularTemperature(const Event &event) noexcept {
    EventType type = event.getType();
    if(type == EventType::DTemperature) {
        desiredTemperature = event.getTemperature();
        if(temperature < desiredTemperature - TEMP_HISTERESIS) {
            send(Actuate::Heat1);
            // TODO timeout
        }
        if(temperature > desiredTemperature + TEMP_HISTERESIS) {
            send(Actuate::Heat0);
            // TODO timeout
        }
    }
    else if(type == EventType::MTemperature) {
        temperature = event.getTemperature();
        if(temperature < desiredTemperature - TEMP_HISTERESIS) {
            send(Actuate::Heat1);
        }
        else if(temperature > desiredTemperature + TEMP_HISTERESIS) {
            send(Actuate::Heat0);
        }
        else {
            // TODO cancel
        }
    }
}

// TODO later PWM
void Automat::doRegularCirculate(const Event &event) noexcept {
    EventType type = event.getType();
    if(type == EventType::DCirc) {
        desiredCirculate = event.getCirc();
        if(desiredCirculate == CirculateState::On) {
            if(sprayChangeTransition == false) {
                send(Actuate::Circ1);
            }
        }
        else {
            send(Actuate::Circ0);
        }
    }
}

void Automat::doRegularSpray(const Event &event) noexcept {
    EventType type = event.getType();
    if(type == EventType::DSpray) {
        SprayChangeState desired = event.getSpray();
        if(desired != SprayChangeState::On && desired != SprayChangeState::Off) {
            return;
        }
        desiredSprayChange = desired;
        if(desiredSprayChange == SprayChangeState::On) {
            if(raise((sprayPosition == SprayChangeState::Upper ||
                          sprayPosition == SprayChangeState::Lower ||
                          sprayPosition == SprayChangeState::Both) &&
                          sprayContact == SprayChangeState::On)) {
              return;
            }
            send(Actuate::Spray1);
            send(Actuate::Circ0);  // prevent circulation during transition
            sprayChangeTransition = true;
        }
    }
    else if(type == EventType::MSpray) {
        sprayContact = event.getSpray();
        if(sprayContact == SprayChangeState::On) {
            if(sprayPosition == SprayChangeState::Upper) {
                timedEvents.schedule(SprayChangeStop, SELECT_DOWN_ON / 2);
                sprayPosition = SprayChangeState::Lower;
            }
            else if(sprayPosition == SprayChangeState::Lower) {
                timedEvents.schedule(SprayChangeStop, SELECT_BOTH_ON / 2);
                sprayPosition = SprayChangeState::Both;
            }
            else if(sprayPosition == SprayChangeState::Both) {
                timedEvents.schedule(SprayChangeStop, SELECT_UP_ON / 2);
                sprayPosition = SprayChangeState::Upper;
            }
        }
    }
}

void Automat::doRegular(const Event &event) noexcept {
    EventType type = event.getType();
    if(type == EventType::Timer) {
        doRegularExpired(event.getExpired());
    }
    else if(type == EventType::DWaterLevel || type == EventType::MWaterLevel || type == EventType::MDrainCurrent) {
        doRegularWaterLevel(event);
    }
    else if(type == EventType::DTemperature || type == EventType::MTemperature) {
        doRegularTemperature(event);
    }
    else if(type == EventType::DCirc || type == EventType::MCircCurrent) {
        doRegularCirculate(event);
    }
    else if(type == EventType::DSpray || type == EventType::MSpray) {
        doRegularSpray(event);
    }
}

void Automat::process(const Event &event) noexcept {
    if(error.load() != 0) {
        return;
    }
    if(event.getType() == EventType::DResinWash || desiredResinWash == ResinWashState::On) {
        doResinWash(event);
    }
    else {
        doRegular(event);
    }
}

void Automat::process(Error error) noexcept {
}
