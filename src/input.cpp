#include "input.h"
#include "dishwash.h"

using namespace std;

Input::Input(Dishwasher &d) : Component(d) {

}

Input::~Input() noexcept {

}

void Input::processAtEachPoll() noexcept {

}
