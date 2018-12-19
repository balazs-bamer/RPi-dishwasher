#include <iostream>

#include "dishwash.h"

using namespace std;

int main(int argc, char **argv) {
#ifdef NDEBUG
    Dishwasher dishwash;
    dishwash.doIt();
    return 0;
#else
    cerr << "Production version does not allow debug build, exiting.\n";
#endif
}
