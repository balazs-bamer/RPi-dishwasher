#include <curses.h>

#include "input.h"
#include "dishwash.h"

// TODO
#include <iostream>

using namespace std;

Input::Input(Dishwasher &d) : Component(d) {
}

Input::~Input() noexcept {
}

void Input::processAtEachPoll() noexcept {
    int ch = getch();
    if(ch == ERR) {
        return;
    }
    // Stop, Drain, Rinse, Fast, FastDry, Middle, All, Hot, Intensive, Cook,
    switch(ch) {
    case 's':
        send(Program::Stop);
        break;
    case 'd':
        send(Program::Drain);
        break;
    case 'r':
        send(Program::Rinse);
        break;
    case 'f':
        send(Program::Fast);
        break;
    case 'y':
        send(Program::FastDry);
        break;
    case 'm':
        send(Program::Middle);
        break;
    case 'a':
        send(Program::All);
        break;
    case 'h':
        send(Program::Hot);
        break;
    case 'i':
        send(Program::Intensive);
        break;
    case 'c':
        send(Program::Cook);
        break;
    }
}
