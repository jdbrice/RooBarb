//
// Created by James Brandenburg on 6/25/15.
//

#include <jdb/RooPlotter.h>
#include "UnitTest/RooPlotterUnitTest.h"


RooPlotterUnitTest::RooPlotterUnitTest(){


}

RooPlotterUnitTest::~RooPlotterUnitTest() {


}


bool RooPlotterUnitTest::test(string testName, int argc, char * argv[]) {
    Logger::setGlobalLogLevel( Logger::llAll );
    bool pass = true;
    INFO( "Beginning Test : " << testName );
    if ( argc < 2 )
        ERROR( "Specify configuration as 1st argument" )

    RooPlotter plt( argv[1] );
    plt.makeReport( "spectra" );


}