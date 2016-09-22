//
// Created by James Brandenburg on 7/28/15.
//

#include <vector>
#include "UnitTest/XmlConfigUnitTest.h"

#include "UnitTest/UnitTest.h"
#include "XmlConfig.h"
#include "Logger.h"
using namespace jdb;

XmlConfigUnitTest::XmlConfigUnitTest(){


}

XmlConfigUnitTest::~XmlConfigUnitTest() {


}


bool XmlConfigUnitTest::test(string testName, int argc, char * argv[]) {

    Logger l(Logger::logLevelFromString("all"));
    Logger::setGlobalLogLevel(Logger::logLevelFromString("all"));

    l.info() << "Creating XmlConfig" << endl;

    if ( argc >= 2 ) {
        XmlConfig cfg(argv[1]);


        INFO( cfg.getString( "EventCuts.triggerMask" ) )



    } else {
        INFO( "Please provide a config file as second CL parameter" );
    }
}