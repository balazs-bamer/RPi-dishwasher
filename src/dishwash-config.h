#ifndef  DISHWASHER_DISHWASH_CONFIG_INCLUDED
#define  DISHWASHER_DISHWASH_CONFIG_INCLUDED

#define DISHWASH_VERSION_MAJOR @DISHWASH_VERSION_MAJOR@
#define DISHWASH_VERSION_MINOR @DISHWASH_VERSION_MINOR@

// all times are defined and stored in ms

// TODO define mm=function(temp, freq)
#define WATER_LEVEL_FULL          100 // mm TODO
#define WATER_LEVEL_HALF           50 // mm TODO
#define WATER_LEVEL_HISTERESIS      2 // mm TODO
#define WATER_LEVEL_MAX           110 // mm TODO
#define WATER_LEVEL_RANGE_MIN      -1 // mm TODO
#define WATER_LEVEL_RANGE_MAX     120 // mm TODO

// TODO define mA=function(volt)
#define CIRC_CURRENT_MAX          400 // mA TODO
#define CIRC_CURRENT_MIN          100 // mA TODO

// TODO define mA=function(volt)
#define DRAIN_CURRENT_MAX         150 // mA TODO
#define DRAIN_CURRENT_MIN          40 // mA TODO

#define CURRENT_SETTLE_TIME      1000 // ms

// TODO define deg=function(volt)
#define SECONDS_PER_DEG_RISE   180000 // ms TODO
#define TEMP_MAX                   75
#define TEMP_HISTERESIS             2 // TODO
#define TEMP_RANGE_MIN              5
#define TEMP_RANGE_MAX            100

#define SELECT_UP_ON             1000 // ms TODO...
#define SELECT_UP_OFF            6500
#define SELECT_DOWN_ON           5500
#define SELECT_DOWN_OFF          3000
#define SELECT_BOTH_ON           3000
#define SELECT_BOTH_OFF          5000
#define SELECT_TOLERANCE          400
#define SELECT_SEARCH           90000
#define SELECT_DECELERATION      1000
#define SELECT_KEEP_POSITION    20000

#define SLEEP_BEFORE_NEXT_STEP   5000 // ms, allows time for initial measurements
#define AVERAGE_FILL_DRAIN_MIN      2 // min - needed to add to program step durations to approxiomate program time
#define REGENERATE_VALVE_TIME  180000 // ms TODO
#define RESIN_WASH_TIME        120000 // ms must be longer than SELECT_SEARCH
#define WASH_DETERGENT_OPEN_TIME  200 // ms
#define SHUTDOWN_RELAY_ON_TIME     50 // ms

#define CIRC_OFF_TIME            1000 // ms
#define CIRC_ON_TIME             2000 // ms

#endif // DISHWASHER_DISHWASH_CONFIG_INCLUDED
