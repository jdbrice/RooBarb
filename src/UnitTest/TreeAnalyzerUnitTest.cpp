//
// Created by James Brandenburg on 5/30/15.
//



#include <jdb/TreeAnalyzer.h>
using namespace jdb;
#include "UnitTest/TreeAnalyzerUnitTest.h"


TreeAnalyzerUnitTest::TreeAnalyzerUnitTest() {


}

TreeAnalyzerUnitTest::~TreeAnalyzerUnitTest() {

}

void TreeAnalyzerUnitTest::test( string testName, int argc, char * argv[]) {
    if ( argc >= 2 ){ // try to load a config for testing

        XmlConfig cfg( argv[1] );

        TreeAnalyzer ta( &cfg, cfg.getString( "job" ) );

    } else { // configless test


    }

    {

        int arrayLength = 100;
        unique_ptr<int[]> dynamicArray = unique_ptr<int[]>(new int[arrayLength]);

        // use it here
        for (int i = 0; i < arrayLength; ++i) {
            dynamicArray[i] = 10;
        }
    }

    cout << "for breakpoint" << endl;

}