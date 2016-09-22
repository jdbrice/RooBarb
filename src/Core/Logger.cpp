#include "Logger.h"

#ifdef __CINT__
ClassImp( jdb::Logger );
#endif

namespace jdb {
	int Logger::llGlobal = Logger::llDefault;	
	bool Logger::showColors = false;
	Logger Logger::log;

	void Logger::setGlobalLogLevel( int ll ){
		Logger::llGlobal = ll;
        Logger::log.setLogLevel( ll );
	}

    void Logger::setGlobalLogLevel( string ll ){
        Logger::llGlobal = Logger::logLevelFromString( ll );
        Logger::log.setLogLevel( ll );
    }

	int Logger::getGlobalLogLevel() { return llGlobal; }

	void Logger::setGlobalColor( bool state  ){
		Logger::showColors = state;
	}
	bool Logger::getGlobalColor( ) { return Logger::showColors; }
}
