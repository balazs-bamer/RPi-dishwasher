#ifndef DISHWASHER_DISHWASH_INCLUDED
#define DISHWASHER_DISHWASH_INCLUDED


#ifndef NDEBUG
#include <fstream>
#endif

#include <array>

#include "base.h"
#include "input.h"
#include "staticerror.h"
#include "logic.h"
#include "display.h"
#include "automat.h"
#include "output.h"

#define NUMCOMPONENTS 6

extern std::atomic<bool> keepRunning;

struct Config {
public:
    static constexpr uint32_t DEFAULTSLEEPNOMINATOR = 1;
    static constexpr uint32_t DEFAULTTIMENOMINATOR = 1;

    uint32_t sleepNominator = DEFAULTSLEEPNOMINATOR;
    uint32_t timeNominator = DEFAULTTIMENOMINATOR;

    uint32_t getPollPeriod(uint32_t t) noexcept { return t / sleepNominator; }
    uint32_t getTime(uint32_t t) noexcept { return t / timeNominator; }
};
extern Config config;

class Dishwasher final {
private:
    Input input;
    StaticError staticError;
    Logic logic;
    Display display;
    Automat automat;
    Output output;
    Component* const components[NUMCOMPONENTS];

#ifndef NDEBUG
    std::ofstream log;
#endif

public:
    /** This may throw exception if some library or hardware component fails. */
    Dishwasher();
    ~Dishwasher();

    Dishwasher(const Dishwasher &i) = delete;
    Dishwasher(Dishwasher &&i) = delete;
    Dishwasher& operator=(const Dishwasher &i) = delete;
    Dishwasher& operator=(Dishwasher &&i) = delete;

    /** This may throw exceptions if thread creation fails. If every one succeeds,
    all the subsequent functions have no-throw guarantee. */
    void doIt();

    /** Sends e to all components except for the originating. */
    void send(const Component &c, const Event &e) noexcept;
};

#endif // DISHWASHER_DISHWASH_INCLUDED
