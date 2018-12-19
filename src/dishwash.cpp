#include <signal.h>
#include <iostream>

// TODO
#include <curses.h>

#include "dishwash.h"

using namespace std;

Config config;

std::atomic<bool> keepRunning;

void signalHandler(int s) {
    keepRunning.store(false);
}

Dishwasher::Dishwasher() :
    input(*this),
    staticError(*this),
    logic(*this),
    display(*this),
    automat(*this),
    output(*this),
    components{&input, &staticError, &logic, &display, &automat, &output}
#ifndef NDEBUG
  , log("log.txt")
#endif
{
    keepRunning.store(true);
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);
}

Dishwasher::~Dishwasher() {
}

void Dishwasher::doIt() {
    int i;
    try {
        for(i = 0; i < NUMCOMPONENTS; ++i) {
            components[i]->start();
        }
    }
    catch(exception &e) {
        cerr << e.what() << endl;
    }
    if(i == NUMCOMPONENTS) {
        while(keepRunning.load()) {

int ch = getch();
cerr << "-ch: " << ch << endl;
//printw("wewee");
            this_thread::sleep_for(1s);
        }
    }
    for(--i; i >= 0; --i) {
        components[i]->stop();
    }
}

void Dishwasher::send(const Component &c, const Event &e) noexcept {
    cerr << "erer" << endl;
#ifndef NDBEUG
    log << string("Message: ") << static_cast<string>(e) << endl;
#endif
    for(int i = 0; i < NUMCOMPONENTS; i++) {
        if(&c != dynamic_cast<Component*>(components[i])) {
            components[i]->queueEvent(e);
        }
    }
}
