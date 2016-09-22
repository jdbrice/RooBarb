//
// Created by James Brandenburg on 5/30/15.
//

#ifndef ROOBARB_UNITTEST_H
#define ROOBARB_UNITTEST_H

// ROOBARB
#include "Utils.h"
#include "Logger.h"
using namespace jdb;

// STL
#include <string>
#include <sstream>
using namespace std;

// ROOT
// #include "TNamed.h"


// Unit testing macros
#define UT_EQ( x, y ) UnitTest::eq( x, y )
#define UT_R_NAMED( x, y ) UnitTest::root_named( x, y )
#define UT_TRUE( x ) UnitTest::eq_true( x )
#define UT_TEST( x, y ) INFO( "", x << " : " << y )

#define UT_PASSED UnitTest::passed
#define UT_FAILED !UnitTest::passed

#define UT_START(x) UnitTest::start( x )
#define UT_SUMMARY UnitTest::summary()


class UnitTest {

public:
    // state of the last run op
    static bool passed;
    static int nPassed;
    static int nFailed;
    static int nTotal;

    static void start( string msg = ""){
        nTotal = 0;
        nPassed = 0;
        nFailed = 0;
        INFO( "UnitTest", yellow("Starting Unit Tests for " + msg ) );
    }

    static string summary(){
        stringstream sstr;
        sstr << green( "PASSED " ) << nPassed << " of " << nTotal;
        if ( nFailed > 0 ){
            sstr << ", " << red( "FAILED " ) << nFailed << " of " << nTotal;
        }
        sstr << endl;
        return sstr.str();
    }

    static string green( string in ) {
        return "\033[1;32m" + in + "\033[0;m";
    }

    static string red( string in ) {
        return "\033[1;31m" + in + "\033[0;m";
    }

    static string yellow( string in ){
        return "\033[1;33m" + in + "\033[0;m";
    }

    static string eq( int life, int hope ){
        nTotal++;
        if ( life == hope ){
            passed = true;
            nPassed ++;
            return green( "PASS" );
        }
        else {
            passed = false;
            nFailed++;
            return  ts(life) + " != " + ts(hope) + red( " FAIL" );
        }
    }

    static string eq( double life, double hope, double tolerance = 0.001 /*percent of life */) {
        nTotal++;
        if ( fabs( life - hope ) < ( life * tolerance ) ){
            passed = true;
            nPassed ++;
            return green( "PASS" );
        }
        else {
            passed = false;
            nFailed++;
            return  dts(life) + " != " + dts(hope) + red( " FAIL" );
        }
    }

    static string eq( string life, string hope ){
        // 033[1;32m
        // 033[0;m
        nTotal++;
        if ( life == hope ){
            passed = true;
            nPassed ++;
            return green( "PASS" );
        }
        else {
            passed = false;
            nFailed++;
            return  "\"" + life + "\" != \"" + hope + "\"" + red( " FAIL" );
        }
    }

    static string nn( void * val ){
        // 033[1;32m
        // 033[0;m

        if ( NULL == val || nullptr == val )
            return  green("NULL/nullptr FAIL");
        else {
            return red("Valid Pointer PASS");
            
        }
    }

    // static string root_named( TNamed * obj, string name ){
    //     if ( NULL == obj || nullptr == obj )
    //         return nn( obj );

    //     if ( obj->GetName() == name )
    //         return green("Object " + name + " PASS");
    //     else
    //         return red("Object " + string(obj->GetName()) + " != " + name + " FAIL");
    // }

    static string eq_true( int life ){
        return eq( life, true );
    }


};


#endif //ROOBARB_UNITTEST_H


#ifdef UNITTEST_NO_CXX
    bool UnitTest::passed = false;
    int UnitTest::nPassed = 0;
    int UnitTest::nFailed = 0;
    int UnitTest::nTotal = 0;
#endif
