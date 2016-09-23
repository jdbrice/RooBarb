#ifndef XML_STRING_H
#define XML_STRING_H


#include "Utils.h"

#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <string>
#include <map>
#include <cstdlib>

#include "XmlConfig.h"


using namespace std;

// Formats a string like
// "this is a {key1} and {path.to.node}"
namespace jdb{

	class XmlConfig;

	class XmlString
	{
	public:
		virtual const char* classname() const { return "XmlString"; }
		XmlString();
		~XmlString();


		static string TOKEN_START;
		static string TOKEN_STOP;

		void add( string k, string v ){
			DEBUG( classname(), "Adding [" << k << "] = " << v );
			kv[ k ] = v;
		}
		void add( string k, int v ){
			add( k, ts(v) );
		}
		void add( string k, double v ){
			add( k, dts(v) );
		}
		void add( string k, float v ){
			add( k, dts(v) );
		}

		string first_token_at( string &s, int &index, int &len, int pos = 0 ) const {
			string rs = s;
				
			string &_tkstart = XmlString::TOKEN_START;
			string &_tkstop = XmlString::TOKEN_STOP;

			index = s.find( _tkstart, pos );

			int stop = s.find( _tkstop, index >= pos ? index : pos);
			len = stop - index + 1;
			
			if ( -1 == index || -1 == stop || len < 0 ) return rs;
			
			rs = s.substr( index+1, len-2 ); // just the part inside of the {}
			return rs;
		}

		void replace_token( const XmlConfig &cfg, string &_s, string &_key, int index, int len );

		string format( const XmlConfig &_cfg, string _s ) {
			int index = -1;
			int len = -1;
			int pos = 0;
			string key = first_token_at( _s, index, len, pos );
			while ( index >= 0 ){

				replace_token( _cfg, _s, key, index, len );
				DEBUG( classname(), "new : " << _s );

				pos = index;
				DEBUG( classname(), "pos = " << pos );
				DEBUG( classname(), key );
				key = first_token_at( _s, index, len, pos );
			}
			return _s;
		}


	protected:
		map<string, string> kv;

		
	};
}


#endif