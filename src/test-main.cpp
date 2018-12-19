#include "timer.h"
#include "dishwash.h"

#include <curses.h>
#include <unistd.h>
#include <iostream>

using namespace std;

void init() noexcept {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    nodelay(stdscr, true);
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
}

void done() noexcept {
    endwin();
}

int main(int argc, char **argv) {
  std::optional<int32_t> result = measureShortestThreadSleep<std::chrono::high_resolution_clock>();
  if(result) {
    std::cout << "high_resolution_clock: " << result.value() << '\n';
  }
  result = measureShortestThreadSleep<std::chrono::steady_clock>();
  if(result) {
    std::cout << "steady_clock: " << result.value() << '\n';
  }
  result = measureShortestThreadSleep<std::chrono::system_clock>();
  if(result) {
    std::cout << "system_clock: " << result.value() << '\n';
  }
    /*try {
        bool good = true;
        int opt;
        while ((opt = getopt(argc, argv, "s:t:")) != -1) {
            switch (opt) {
            case 's':
                config.sleepNominator = stoul(optarg);
                if(config.sleepNominator == 0) {
                    good = false;
                }
                break;
            case 't':
                config.timeNominator = stoul(optarg);
                if(config.timeNominator == 0) {
                    good = false;
                }
                break;
            default: good = false;
            }
        }
        if(!good) {
            cerr << "Usage:\n" << argv[0] <<
                "\noptions:" <<
                "\n-h or -? show this help and exit" <<
                "\n-s thread sleep speedup factor, default: " << config.DEFAULTSLEEPNOMINATOR <<
                "\n-t general physics speedup factor, default: " << config.DEFAULTTIMENOMINATOR << endl;
            return 1;
        }
        Dishwasher dishwash;
        dishwash.doIt();
    }
    catch(exception &e) {
        cerr << "main: " << e.what() << endl;
    }
    done();*/
    return 0;
}
