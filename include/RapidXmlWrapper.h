#ifndef RAPID_XML_WRAPPER_H
#define RAPID_XML_WRAPPER_H

#include "rapidXML.h"
#include <string>
#include <sstream>
#include <exception>
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>
#include <map>
using namespace std;
using namespace rapidxml;

#include "Logger.h"
using namespace jdb;


template <typename key_type, typename mapped_type>
class NodeMap {
public:
	typedef std::pair<const key_type, mapped_type> value_type;

	NodeMap(){

	}
	~NodeMap(){

	}

	vector<key_type> keys;
	vector<mapped_type> vals;

	bool has_key(const key_type key) {
		if ( std::find( keys.begin(), keys.end(), key ) != keys.end() )
			return true;
		return false;
	}

	size_t index_of( const key_type key ){
		size_t pos = std::find(keys.begin(), keys.end(), key) - keys.begin();
		return pos;
	}
	
	mapped_type &operator[](const key_type key){
		if ( has_key( key ) ){
			return vals[ index_of( key ) ];
		}

		mapped_type t;
		return t;
	}

};


class RapidXmlWrapper
{

protected:
	string configFile;
	string fname;
	char* doc_string = nullptr;

	bool good = false;

	xml_document<> doc;
public:
	RapidXmlWrapper(){
		DEBUG(  "RapidXmlWrapper", "()" );
		pathDelim = '.';
		attrDelim = ':';
		indexOpenDelim = "[";
		indexCloseDelim = "]";
		equalDelim = '=';
	}
	RapidXmlWrapper( string filename ){

        DEBUG( "RapidXmlWrapper", "( filename=" << filename << " )" )
		fname = filename;
		configFile = getFileContents( filename.c_str() );
		parseXmlString( configFile );
		// char* cstr = new char[configFile.size() + 1];  	// Create char buffer to store string copy
	 //  	strcpy (cstr, configFile.c_str());             		// Copy string into char buffer

	 //  	try {
	 //  		doc.parse<0>(cstr);
	 //  	} catch ( exception &e ){
	 //  		cout << "Could not parse " << filename << " : " << e.what() << endl;
	 //  	}

	  	pathDelim = '.';
		attrDelim = ':';
		indexOpenDelim = "[";
		indexCloseDelim = "]";
		equalDelim = '=';

	}
	~RapidXmlWrapper(){
		if ( doc_string != nullptr ){
			delete[] doc_string;
			doc_string = nullptr;
		}
	}

	void parseXmlString( string xml ){
		if ( doc_string != nullptr ){
			delete[] doc_string;
			doc_string = nullptr;
		}
		doc_string = new char[xml.size() + 1];  	// Create char buffer to store string copy
		strcpy (doc_string, xml.c_str());             		// Copy string into char buffer

		try {
			doc.parse<0>(doc_string);
			good = true;
		} catch ( exception &e ){
			cout << "Could not parse xml string : " << e.what() << endl;
		}

	}

	string getFileContents(const char *filename){
		ifstream in(filename, ios::in | ios::binary);

		if (in.good()){
			std::string contents;

			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());

			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());

			in.close();

			return(contents);
		}
		//throw(errno);
		return "";
	}

	xml_node<> * getNode( string path ){
		//if ( !nodeExists( path ) )
		//	return NULL;

		stringstream sstr; 

	  	vector<string> ntf = split( path, '.' );
	  	vector<string> attr = split( path, ':' );

	  	if ( attr.size() >= 2 ){
			ntf[ ntf.size() - 1 ] = ntf[ ntf.size() - 1 ].substr( 0, ntf[ ntf.size() - 1 ].length() - (attr[ 1].length() + 1) );
		}

		xml_node<> *node = doc.first_node();
		for ( uint i = 0; i < ntf.size(); i++ ){
			if ( node ){
				node = node->first_node( ntf[ i ].c_str() );
				if ( node && ntf.size() - 1 == i ){
					
					return node;
				}
			} else {
				return NULL;	
			}
		}

		return NULL;
	}

	vector<string> &split(const string &s, char delim, vector<string> &elems) {
	    stringstream ss(s);
	    string item;
	    while (std::getline(ss, item, delim)) {
	        elems.push_back(item);
	    }
	    return elems;
	}
	vector<string> split(const string &s, char delim) {
	    vector<string> elems;
	    split(s, delim, elems);
	    return elems;
	}

	std::string trim(const std::string& str, const std::string& whitespace = " \t\n") {
	    const std::size_t strBegin = str.find_first_not_of(whitespace);
	    if (strBegin == std::string::npos)
	        return ""; // no content

	    const std::size_t strEnd = str.find_last_not_of(whitespace);
	    const std::size_t strRange = strEnd - strBegin + 1;

	    return str.substr(strBegin, strRange);
	}



	void getMaps( vector<string> *keys, map<string, string> *data, map<string, bool> * isAttribute, map<string, bool> *exists ){

		xml_node<> *node = doc.first_node();
		makeMap( node, "", keys, data, isAttribute, exists );

	}

	void includeMaps( string context, vector<string> *keys, map<string, string> *data, map<string, bool> * isAttribute, map<string, bool> *exists ){

		xml_node<> *node = doc.first_node();
		makeMap( node, context, keys, data, isAttribute, exists );

	}

	void makeMap( xml_node<> * node, string cp, vector<string> *keys, map<string, string> *data, map<string, bool> * isAttribute, map<string, bool> *exists ){
		// DEBUG( "RapidXmlWrapper", "child_name" )
		if ( false == good ) {
			cerr << "RapidXmlWrapper :: ERROR -> doc root invalid" << endl;
			return;
		}
		map< string, int > index;
		for (xml_node<> *child = node->first_node(); child; child = child->next_sibling() ){
		    string nodeName = child->name();
		    if ( "" == nodeName )
		    	continue;

		    // cout << "nodeName = " << child->name() << endl;

		    // if ( !index[ nodeName ] )
		    if ( index.count( nodeName ) <= 0 )
				index[ nodeName ] = 0;

			stringstream sstr;
			sstr << index[ nodeName ];
			string path =  cp ;

			if ( "" != cp )
				path += pathDelim + nodeName;
			else 
				path += nodeName;
			
			if ( index[ nodeName ] >= 0 )
				path += indexOpenDelim + sstr.str() + indexCloseDelim;

			index[ nodeName ]++;

			(*isAttribute)[ path ] = false;
			(*exists)[ path ] = true;
			string nValue = "";
			if ( child->value() ){
				nValue = trim( std::string(child->value()) );
			} 
			// else 
				// (*data)[path] = "";
			// DEBUG( "RapidXmlWrapper", "["<< path << "]=<" << nValue << ">" );
			(*data)[path] = nValue;
			(*keys).push_back( path );

			/**
			 * Get attributes
			 */
			for( const xml_attribute<>* a = child->first_attribute(); a ; a = a->next_attribute() ) {
				string aName = a->name();
				string aVal = a->value();
				if ( "" == aName )
					continue;

				string aPath = path + attrDelim + aName;
				(*data)[ aPath ] = aVal;
				(*keys).push_back( aPath );
				(*isAttribute)[ aPath ] = true;
				(*exists)[ aPath ] = true;
			}

			makeMap( child, path, keys, data, isAttribute, exists );

		}

	}



	//pjdco{ "name" : "char attrDelim", "desc" : "The delimiter used for attributes - Default is \":\""}
	char attrDelim;

	//pjdco{ "name" : "char pathDelim", "desc" : "The delimiter used for paths - Default is \".\""}
	char pathDelim;

	//pjdco{ "name" : "char equalDelim", "desc" : ""}
	char equalDelim;

	//pjdco{ "name" : "indexOpenDelim", "desc" : "The delimeter for index open - Default is \"[\""}
	string indexOpenDelim;
	
	//pjdco{ "name" : "indexCloseDelim", "desc" : "The delimeter for index open - Default is \"]\""}
	string indexCloseDelim;
	
};



#endif