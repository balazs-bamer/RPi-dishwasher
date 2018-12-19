#include"diagnostic.h"

std::string event2string(Event const &aEvent); {
  string result(aEvent.getTypeConstStr());
  result += ": ";
  if(aEvent.getType() == EventType::MeasuredCircCurrent ||
     aEvent.getType() == EventType::MeasuredDrainCurrent ||
     aEvent.getType() == EventType::MeasuredWaterLevel ||
     aEvent.getType() == EventType::MeasuredTemperature ||
     aEvent.getType() == EventType::DesiredWaterLevel ||
     aEvent.getType() == EventType::DesiredTemperature ||
     aEvent.getType() == EventType::RemainingTime ||
     aEvent.getType() == EventType::TimerFactorChanged) {
    result += to_string(aEvent.getIntValue();
  }
  else {
    result += aEvent.getValueConstStr();
  }
  return result;
}
