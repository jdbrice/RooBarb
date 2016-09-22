//
// Created by James Brandenburg on 7/28/15.
//

#ifndef ROOBARB_XMLCONFIGUNITTEST_H
#define ROOBARB_XMLCONFIGUNITTEST_H

#include <string>
using namespace std;

#include "UnitTest/UnitTest.h"

class XmlConfigUnitTest {

public:
    XmlConfigUnitTest(  );
    ~XmlConfigUnitTest();

    // Tests should be enumerated here
    bool test(string testName, int argc, char* argv[]);

};


#endif //ROOBARB_XMLCONFIGUNITTEST_H
