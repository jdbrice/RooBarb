//
// Created by James Brandenburg on 6/3/15.
//

#include <vector>
#include "UnitTest/HistoBinsUnitTest.h"

#include "UnitTest/UnitTest.h"
#include "HistoBins.h"
#include "Logger.h"
using namespace jdb;

HistoBinsUnitTest::HistoBinsUnitTest(){


}

HistoBinsUnitTest::~HistoBinsUnitTest() {


}


bool HistoBinsUnitTest::test(string testName, int argc, char * argv[]) {

    Logger l( Logger::logLevelFromString( "all" ) );
    Logger::setGlobalLogLevel( Logger::llAll );

    INFO( "HistoBinsUnitTest", "Creating HistoBins"  );

    vector<double> bins = { 0, 1.2, 5.0, 6.0, 9.0 };

    HistoBins hb( bins );
    INFO( "Created : " << hb.toString()  );
    double val = 5.0;

    string test = UT_EQ( hb.findBin( val ), 2);
    INFO( "findBin( " << val <<" ) ==> " << test  );

    test = UT_EQ( hb.findBin( val, BinEdge::upper ), 1);
    INFO( "findBin( " << val <<" ) ==> " << test  );

    val = 0.0;
    test = UT_EQ( hb.findBin( val ), 0);
    INFO( "findBin( " << val <<" ) ==> " << test  );


    INFO( "Testing Overflow : " );
    val = 1000;
    test = UT_EQ( hb.findBin( val ), -2);
    INFO( "findBin( " << val <<" ) ==> " << test  );

    INFO( "Testing Underflow : "  );
    val = -100;
    test = UT_EQ( hb.findBin( val ), -1);
    INFO( "findBin( " << val <<" ) ==> " << test  );

    INFO( endl << "Testing Creating From Config : " );
    DEBUG( UT_EQ( -2, -2) );

    return true;
}