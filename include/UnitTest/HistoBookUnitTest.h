#ifndef HISTO_BOOK_UNIT_TEST_H
#define HISTO_BOOK_UNIT_TEST_H


#include <string>
using namespace std;

class HistoBookUnitTest {

public:
    HistoBookUnitTest(  );
    ~HistoBookUnitTest();

    // Tests should be enumerated here
    bool test(string testName, int argc, char* argv[]);

};


#endif