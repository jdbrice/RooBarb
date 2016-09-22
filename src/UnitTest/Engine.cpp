#include <string>
#include <UnitTest/HistoBookUnitTest.h>

#include "UnitTest/TreeAnalyzerUnitTest.h"
#include "UnitTest/HistoBinsUnitTest.h"
#include "UnitTest/RooPlotterUnitTest.h"
#include "UnitTest/XmlConfigUnitTest.h"

#include "XmlConfig.h"
#include "XmlCanvas.h"

using namespace jdb;

#include "TH1D.h"

int main( int argc, char* argv[] ){

    Logger::setGlobalLogLevel( "all" );
    DEBUG("")

    // try loading a config in from the first argument
	if ( argc >= 2 ){
        XmlConfig cfg( argv[ 1 ] );


        string job = cfg.getString( "job", "default" );
        INFO( "Job " << job )
        if ( "TreeAnalyzerUnitTest" == job ){
            TreeAnalyzerUnitTest taut;
            taut.test("", argc, argv);
        } else if ("HistoBinsUnitTest" == job){
            HistoBinsUnitTest hbut;
            hbut.test( "General", argc, argv );
        } else if ( "RooPlotterUnitTest" == job ){
            RooPlotterUnitTest rput;
            rput.test( "General", argc, argv );
        } else if ( "XmlConfigUnitTest" == job ){
            XmlConfigUnitTest xcut;
            xcut.test( "General", argc, argv );
        } else if ("HistoBookUnitTest" == job){
            HistoBookUnitTest hbut;
            hbut.test( "General", argc, argv );
        }
    }

}