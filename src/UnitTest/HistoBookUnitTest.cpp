#include "UnitTest/HistoBookUnitTest.h"

#include <vector>

#include "UnitTest/UnitTest.h"
#include "HistoBook.h"
#include "Logger.h"
using namespace jdb;

HistoBookUnitTest::HistoBookUnitTest(){


}

HistoBookUnitTest::~HistoBookUnitTest() {


}


bool HistoBookUnitTest::test(string testName, int argc, char * argv[]) {

    Logger::setGlobalLogLevel( "all" );
    string tag = "HistoBookUnitTest";
    INFO( tag, "Starting" );


    XmlConfig cfg( argv[1] );
    HistoBook * book = new HistoBook( "test", &cfg );

    book->makeAll( "histograms" );

    INFO( tag, "Trying to access and fill" );

    book->fill( "events1", 10 );
    book->fill( "events2", 10 );
    book->fill( "events2D", 10, 0, .5 );
    book->fill( "events3D", 10, 0, 1, .5 );



    book->ls();
    delete book;

    

    return true;
}