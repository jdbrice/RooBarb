#include "XmlString.h"
#include "XmlConfig.h"


namespace jdb{

	XmlString::XmlString() {}
	XmlString::~XmlString() {}

	string XmlString::TOKEN_START = "{";
	string XmlString::TOKEN_STOP = "}";


	void XmlString::replace_token( const XmlConfig &cfg, string &_s, string &_key, int index, int len, bool _clobber ) {

		if ( kv.count( _key ) >= 1 ){
			DEBUG( classname(), "map" );
			string rv = kv[ _key ];
			_s.replace( index, len, rv );
		} else if ( getenv( _key.c_str() ) ) {
			DEBUG( classname(), "ENV" );
			string env = getenv( _key.c_str() );
			_s.replace( index, len, env );
		} else if ( cfg.exists( _key ) ) {
			DEBUG( classname(), "CFG" );
			string rv = cfg.getXString( _key ); // careful this could cause infinite recursion
			_s.replace( index, len, rv );
		} else if ( _clobber ){
			DEBUG( classname(), "CLOBBER" );
			_s.replace( index, len, "" );
		}
	}

	bool XmlString::replace_token( string &_s, string &_key, int index, int len, bool _clobber ) {

		if ( kv.count( _key ) >= 1 ){
			string rv = kv[ _key ];
			_s.replace( index, len, rv );
			return true;
		} else if ( getenv( _key.c_str() ) ) {
			string env = getenv( _key.c_str() );
			_s.replace( index, len, env );
			return true;
		} else if ( _clobber ){
			_s.replace( index, len, "" );
			return true;
		}

		return false;
	}
}
