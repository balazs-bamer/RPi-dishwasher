#include "input.h"
#include "logic.h"
#include "automat.h"
#include "display.h"
#include "staticerror.h"
#include "output.h"
#include "dishwash.h"
#include "logstdthreadostream.h"

#include <fstream>

int main(int argc, char **argv) {
  char defaultLogFilename[] = "dishwasher.log";
  try {
    nowtech::LogConfig logConfig;
    logConfig.taskRepresentation = nowtech::LogConfig::TaskRepresentation::cName;
    logConfig.refreshPeriod      = 200u;
    std::ofstream logFile(argc == 1 ? defaultLogFilename : argv[1]);
    nowtech::LogStdThreadOstream osInterface(logFile, logConfig);
    nowtech::Log log(osInterface, logConfig);
    Log::registerApp(nowtech::LogApp::cSystem, "system");
    Log::registerCurrentTask("main   ");

    Input input;
    Logic logic;
    Automat automat;
    Display display;
    StaticError staticError;
    Output output;
    Dishwasher dishwash({&input, &logic, &automat, &display, &staticError, &output});
    dishwash.run();
  }
  catch(std::exception &e) {
    Log::i() << "exception: " << e.what() << Log::end;
  }
  return 0;
}
