//
// Created by James Brandenburg on 6/25/15.
//

#ifndef ROOBARB_ROOPLOTTERUNITTEST_H
#define ROOBARB_ROOPLOTTERUNITTEST_H

#include "UnitTest/UnitTest.h"
#include "Logger.h"
using namespace jdb;

class RooPlotterUnitTest {

public:
    RooPlotterUnitTest(  );
    ~RooPlotterUnitTest();

    // Tests should be enumerated here
    bool test(string testName, int argc, char* argv[]);

};


#endif //ROOBARB_ROOPLOTTERUNITTEST_H
