#include "XmlString.h"
#include "XmlConfig.h"


namespace jdb{

	XmlString::XmlString() {}
	XmlString::~XmlString() {}

	string XmlString::TOKEN_START = "{";
	string XmlString::TOKEN_STOP = "}";


	void XmlString::replace_token( const XmlConfig &cfg, string &_s, string &_key, int index, int len ) {

		if ( kv.count( _key ) >= 1 ){
			string rv = kv[ _key ];
			_s.replace( index, len, rv );
		} else if ( getenv( _key.c_str() ) ) {
			string env = getenv( _key.c_str() );
			_s.replace( index, len, env );
		} else if ( cfg.exists( _key ) ) {
			string rv = cfg.getXString( _key ); // careful this could cause infinite recursion
			_s.replace( index, len, rv );
		} else {
			_s.replace( index, len, "" );
		}
	}
}
