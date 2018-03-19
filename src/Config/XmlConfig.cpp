#include "XmlConfig.h"
#include <sys/stat.h>


#include "XmlString.h"
#include "Utils.h"

#ifdef __CINT__
ClassImp( jdb::XmlConfig );
#endif

namespace jdb{

	const string XmlConfig::declarationV1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

	XmlConfig::XmlConfig( string filename){
		DEBUG( classname(), "Loading Xml Configuration from : " << filename );

		setDefaults();
		loadFile( filename );
		
		// typedef map<string, string>::iterator map_it_type;
	}

	XmlConfig::XmlConfig(){
		DEBUG( classname(), "Empty XmlConfig");

		setDefaults();
	}

	XmlConfig::~XmlConfig(){
	}

	XmlConfig::XmlConfig( const XmlConfig &rhs){
		DEBUG( classname(), "copy constructor!" );
		setDefaults();

		DEBUG( classname(), "Copying filename" );
		this->filename = rhs.filename;

		DEBUG(classname(), "Copying NodeExist map" );
		this->nodeExists = rhs.nodeExists;
		DEBUG(classname(), "Copying Data map" );
		this->data         = rhs.data;
		this->orderedPaths = rhs.orderedPaths;
		DEBUG(classname(), "Copying Attribute map" );
		this->isAttribute = rhs.isAttribute;
	}


	void XmlConfig::loadXmlString( string xml ){
		map<string, string> empty_overrides;
		loadXmlString( xml, empty_overrides );
	}
	void XmlConfig::loadXmlString( string xml, map<string, string> overrides ){
		DEBUGC( "Loading XML data from string\n" << xml );
		this->filename = "";

		RapidXmlWrapper rxw;
		rxw.parseXmlString( xml );
		rxw.getMaps( &orderedPaths, &data, &isAttribute, &nodeExists );

		// Apply these overrides BEFORE parsing includes -> so that you can control what gets included dynamically
		applyOverrides( overrides );

		int nNotFound = parseIncludes();
		int nTries = 0;
		while ( nNotFound >= 1 && nTries < 1 ){
			nTries++;
			int nNotFoundBefore = nNotFound;
			nNotFound = parseIncludes();
			// check to see if we are making progress (for dependencies)
			// if we are then don't bail out
			if ( nNotFound < nNotFoundBefore ) nTries = 0;

		}

		// Apply again to really override any includes
		applyOverrides( overrides );

	}

	void XmlConfig::loadFile( string _filename ){
		map<string, string> empty_overrides;
		loadFile( _filename, empty_overrides );
	}
	void XmlConfig::loadFile( string _filename, map<string, string> overrides ){
		DEBUG( classname(), "Loading " << _filename );
		
		// check that the config file exists
		this->filename = _filename;
		struct stat buffer;   
		bool exists = (stat (_filename.c_str(), &buffer) == 0);
	
		if ( exists ){
#ifndef __CINT__
			RapidXmlWrapper rxw( _filename );
#endif
			rxw.getMaps( &orderedPaths, &data, &isAttribute, &nodeExists );

			// Apply these overrides BEFORE parsing includes -> so that you can control what gets included dynamically
			applyOverrides( overrides );

			int nNotFound = parseIncludes();
			int nTries = 0;
			while ( nNotFound >= 1 && nTries < 1 ){
				nTries++;
				int nNotFoundBefore = nNotFound;
				nNotFound = parseIncludes();
				// check to see if we are making progress (for dependencies)
				// if we are then don't bail out
				if ( nNotFound < nNotFoundBefore ) nTries = 0;

			}

			// Apply again to really override any includes
			applyOverrides( overrides );


		} else {
			ERROR( classname(), "Config File \"" << _filename << "\" DNE " ); 
		}

		INFO( classname(), "Loaded " << getFilename() );
	}

	/* Sets the default strings / delimeters
	 * 
	 */
	void XmlConfig::setDefaults(){
		DEBUG( classname(), "Setting Defaults" );
		// currently set may change
		pathDelim = '.';
		attrDelim = ':';
		indexOpenDelim = "[";
		indexCloseDelim = "]";
		equalDelim = '=';
		mapDelim = "::";

	}


	string XmlConfig::getRawString( string nodePath, string def ) const {
		string snp = sanitize( currentNode + nodePath );
		if ( nodeExists.count( snp ) >= 1 ){
			try{
				return data.at( snp );
			} catch (const std::out_of_range &oor ){
				return def;
			}
		}
		return def;
	}

	string XmlConfig::getString( string nodePath, string def ) const {
		return getXString(nodePath, def);
	}


	string XmlConfig::getString( string _prefix, vector<string> _paths, string _def ) const {
		for ( auto pp : _paths ){
			string p = _prefix + pp;
			if ( exists( p ) ){
				return getXString( p, _def );
			}
		}
		return _def;
	}

	string XmlConfig::getXString( string nodePath, string def ) const {
		string raw = getRawString( nodePath, def );
		if ( '@' == raw[0] ){
			raw = raw.substr( 1 );
			raw = XmlString().format( (*this), raw );
			return getXString( raw, def );
		}
		return XmlString().format( (*this), raw );
	}

	vector<string> XmlConfig::getStringVector( string nodePath ) const {
		if ( !exists( nodePath ) ){
			vector<string> d;
			return d;
		}
		string value = getXString( nodePath );
		return vectorFromString( value );
	}
	
	vector<string> XmlConfig::getStringVector( string nodePath, string defaultVal, int defaultLength ) const {
		
		if ( !exists( nodePath ) ){
			vector<string> d;
			for ( int i = 0; i < defaultLength; i++ )
				d.push_back( defaultVal );
			return d;
		}
		string value = getXString( nodePath );
		return vectorFromString( value );
	}
	vector<string> XmlConfig::getStringVector( string nodePath, vector<string> defaultVals ) const {
		if ( !exists( nodePath ) ){
			return defaultVals;
		}
		string value = getString( nodePath );
		return vectorFromString( value );
	}

	map<string, string> XmlConfig::getStringMap( string nodePath ) const{

		// first get a vector of comma delimeted pairs
		string value = getString( nodePath );
		vector<string> pairVec =  vectorFromString( value );
		
		map<string, string> rmap;
		// now we need to split each pair
		for ( unsigned int i = 0; i < pairVec.size(); i++ ){
			pair<string, string> parts = stringToPair( pairVec[ i ], mapDelim );
			rmap[ parts.first ] = parts.second;
		}
		return rmap;
	}

	map<int, int> XmlConfig::getIntMap( string nodePath ) const{

		// first get a vector of comma delimeted pairs
		string value = getXString( nodePath );
		vector<string> pairVec =  vectorFromString( value );
		
		map<int, int> rmap;
		// now we need to split each pair
		for ( unsigned int i = 0; i < pairVec.size(); i++ ){
			pair<string, string> parts = stringToPair( pairVec[ i ], mapDelim );
			rmap[ atoi( parts.first.c_str() ) ] = atoi( parts.second.c_str() );
		}
		return rmap;
	}

	map<float, float> XmlConfig::getFloatMap( string nodePath ) const{

		// first get a vector of comma delimeted pairs
		string value = getXString( nodePath );
		vector<string> pairVec =  vectorFromString( value );
		
		map<float, float> rmap;
		// now we need to split each pair
		for ( unsigned int i = 0; i < pairVec.size(); i++ ){
			pair<string, string> parts = stringToPair( pairVec[ i ], mapDelim );
			rmap[ atof( parts.first.c_str() ) ] = atof( parts.second.c_str() );
		}
		return rmap;
	}

	int XmlConfig::getInt( string nodePath, int def  ) const{
		string str = getXString( nodePath, "" );
		if ( "" != str && str.length() >= 1 )
			return atoi( str.c_str() );
		return def;
	}
	vector<int> XmlConfig::getIntVector( string nodePath, int defaultVal, int defaultLength ) const{
		
		vector<int> d;
		// default if node does not exist
		if ( !exists( nodePath ) ){
			for ( int i = 0; i < defaultLength; i++ )
				d.push_back( defaultVal);
			return d;
		}
		

		vector<string> vec = getStringVector( nodePath );
		
		for ( unsigned int i = 0; i < vec.size(); i++  ){
			d.push_back( atoi( vec[ i ].c_str() ) );
		}
		return d;
	}

	double XmlConfig::getDouble( string nodePath, double def  ) const {
		string str = getXString( nodePath, "" );
		if ( "" != str && str.length() >= 1 )
			return atof( str.c_str() );
		return def;
	}
	vector<double> XmlConfig::getDoubleVector( string nodePath, double defaultVal, int defaultLength ) const{
		
		vector<double> d;
		// default if node does not exist
		if ( !exists( nodePath ) ){
			for ( int i = 0; i < defaultLength; i++ )
				d.push_back( defaultVal );
			return d;
		}

		vector<string> vec = getStringVector( nodePath );
		for ( unsigned int i = 0; i < vec.size(); i++  ){
			d.push_back( atof( vec[ i ].c_str() ) );
		}
		return d;
	}

	vector<float> XmlConfig::getFloatVector( string nodePath, float defaultVal, int defaultLength ) const{
		
		vector<float> d;
		// default if node does not exist
		if ( !exists( nodePath ) ){
			for ( int i = 0; i < defaultLength; i++ )
				d.push_back( defaultVal );
			return d;
		}

		vector<string> vec = getStringVector( nodePath );
		for ( unsigned int i = 0; i < vec.size(); i++  ){
			d.push_back( atof( vec[ i ].c_str() ) );
		}
		return d;
	}

	float XmlConfig::getFloat( string nodePath, float def  ) const{
		return (float) getDouble( nodePath, (double)def );
	}


	bool XmlConfig::getBool( string nodePath, bool def  ) const{

		string str = getXString( nodePath );

		// first check for string literal "true" or "false"
		// push to lower case
		//std::transform( str.begin(), str.end(), str.begin(), std::tolower );
		str = manualToLower( str );
		if ( str == "false" )
			return false;
		if ( str == "true" )
			return true;

		// if not look for an integer
		// 0 or negative = false
		// any positive = true
		if ( str.length() >= 1 ){
			return ( atoi( str.c_str() ) >= 1 );
		}
		return def;
	}

	bool XmlConfig::exists( string nodePath ) const{
		string snp = sanitize( currentNode + nodePath );
		try{
			if( true == nodeExists.at( snp ) )
				return true;
		} catch ( std::out_of_range &oor ){
			return false;
		}
		return false;
	}

	string XmlConfig::oneOf( vector<string> _paths ){
		for ( string p : _paths ){
			if ( "" == p ) continue;
			if ( exists( p ) ) return p;
		}
		return "";
	}
	string XmlConfig::oneOf( string _p1, string _p2, string _p3, string _p4 ){
		return oneOf( {_p1, _p2, _p3, _p4} );
	}

	string XmlConfig::sanitize( string nodePath ) const{

		/*************************************************/
		// TODO
		// make the cn option applied here so everything uses it.
		// currently childrenOf does not work with it

		/*************************************************/
		// Remove internal whitespaces
		string ret = "";
		for ( unsigned int i = 0; i < nodePath.length(); i++ ){
			if ( nodePath[i] != ' ' )
				ret += (char)nodePath[ i ];
		}

		/*************************************************/
		// Add [0] to unindexed paths
		string attr = attributeName( ret );
		vector<string> parts = split( stripAttribute( ret ), pathDelim );

		int i = 0;
		ret = "";
		for ( string p : parts ){
			string np = p;
			if ( -1 == pathIndex( p ) ){
				np = addIndex( p );
			}
			ret += np;
			i++;
			if ( i < parts.size() )
				ret += pathDelim;
		}

		/*************************************************/
		// Add back the attribute if it was there
		if ( attr.length() >= 1 )
			ret += attrDelim + attr;

		return ret;
	}

	string XmlConfig::operator[]( string nodePath ) const {
		return getXString( nodePath);
	}

	vector<string> & XmlConfig::split(const string &s, char delim, vector<string> &elems) const {
		stringstream ss(s);
		string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}

	vector<string> & XmlConfig::split( string &s, string delim, vector<string> &elems) const {
		size_t pos = 0;
		std::string token;
		while ((pos = s.find(delim)) != std::string::npos) {
			token = s.substr(0, pos);
			elems.push_back( token );
			s.erase(0, pos + delim.length() );
		}
		elems.push_back( s );
		return elems;
	}

	vector<string> XmlConfig::vectorFromString( string data ) const {
				
		vector<string> d = split( data, ',' );
		
		for ( unsigned int i = 0; i < d.size(); i++ ){
			d[ i ] =  trim( d[ i ] );
		}
		return d;

	}

	std::string XmlConfig::trim(const std::string& str, const std::string& whitespace ) const {
		std::size_t strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
			return ""; // no content

		std::size_t strEnd = str.find_last_not_of(whitespace);
		std::size_t strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

	vector<string> XmlConfig::split(const string &s, char delim) const {
		vector<string> elems;
		split(s, delim, elems);
		return elems;
	}

	vector<string> XmlConfig::split(string &s, string delim) const {
		vector<string> elems;
		split(s, delim, elems);
		return elems;
	}

	string XmlConfig::manualToLower( string str ) const {
		string str2 = str;
		for ( unsigned int i = 0; i < str.length(); i++ ){
			str2[ i ] = std::tolower( str[ i ] );
		}
		return str2;
	}
	string XmlConfig::pathToParent( string nodePath ) const {
		vector<string> ntf = split( nodePath, pathDelim );
		vector<string> attr = split( nodePath, attrDelim );
		if ( attr.size() >= 2 ){
			ntf[ ntf.size() - 1 ] = ntf[ ntf.size() - 1 ].substr( 0, ntf[ ntf.size() - 1 ].length() - (attr[ 1].length() + 1) );
		}
		if ( ntf.size() >= 2 ){
			string fullPath ="";
			for ( unsigned int i = 0; i < ntf.size() - 1; i++ ){
				fullPath += (ntf[ i ] + pathDelim );
			}

			// remove the final pathDelim
			fullPath = fullPath.substr( 0, fullPath.length() - 1 );
			return fullPath;
		}
		return "";
	}

	string XmlConfig::pathToDepth( string nodePath, int depth ) const {
		vector<string> ntf = split( nodePath, pathDelim );
		if ( ntf.size() >= 1 ){
			string fullPath ="";
			for ( unsigned int i = 0; i < ntf.size(); i++ ){
				if ( i > depth ) break;
				fullPath += (ntf[ i ] + pathDelim );
			}
			
			return basePath( fullPath );
		}
		return "";
	}

	string XmlConfig::tagName( string nodePath, int atDepth ) const{
		
		if ( atDepth < 0 ){
			vector<string> ntf = split( nodePath, pathDelim );
			vector<string> attr = split( nodePath, attrDelim );
			if ( attr.size() >= 2 ){
				ntf[ ntf.size() - 1 ] = ntf[ ntf.size() - 1 ].substr( 0, ntf[ ntf.size() - 1 ].length() - (attr[ 1].length() + 1) );
			}
			if ( ntf.size() >= 1 ){
				vector<string> byIndex = split( ntf[ ntf.size() - 1 ], indexOpenDelim[0] );
				if ( byIndex.size() >= 2 ){
					return byIndex[ 0 ];
				}

				return ntf[ ntf.size() - 1 ];
			}
			return "";
		} else {

			vector<string> ntf = split( nodePath, pathDelim );
			if (atDepth >= ntf.size() )
				atDepth = ntf.size()-1;
			return stripIndex( stripAttribute( ntf[atDepth] ) );
		}
	}

	vector<string> XmlConfig::childrenOf( string nodePath, int relDepth, bool attrs ) const {

		nodePath = sanitize( nodePath );
	
		int npDepth = depthOf( nodePath );
		

		vector<string> paths;
		// for ( const_map_it_type it = data.begin(); it != data.end(); it++ ){
			// const string &key = it->first; 
		// cout << "orderedPaths.size()=" << orderedPaths.size() << endl; 
		for ( const string key : orderedPaths ) {
			// cout << "key : "  << key << endl;
			// reject self
			if ( key == nodePath )
				continue;

			string::size_type found = key.find( attrDelim );
			if ( found != string::npos && false == attrs )
				continue;
			
			string parent = (key).substr( 0, nodePath.length() );
			if ( nodePath == parent ){
				
				if ( -1 == relDepth ) 
					paths.push_back( key );
				else {
					int dp = depthOf( key );

					if ( dp - npDepth > 0 && dp - npDepth <= relDepth ) 
						paths.push_back( key );
				}
			}
		}
		return paths;

	}

	vector<string> XmlConfig::childrenOf( string nodePath, string tag, int depth) const {

		nodePath = sanitize( nodePath );

		int npDepth = depthOf( nodePath );

		if ( 	nodePath[ nodePath.length() - 1] != pathDelim && 
				nodePath[ nodePath.length() - 1] != attrDelim && "" != nodePath)
			nodePath += pathDelim;
	
		vector<string> paths;
		// for ( const_map_it_type it = data.begin(); it != data.end(); it++ ){
		for ( const string key : orderedPaths ) {

			size_t found = key.find( attrDelim );
			if ( found != string::npos )
				continue;
			
			// reject self
			if ( key == nodePath )
					continue;
			string parent = (key).substr( 0, nodePath.length() );
			
			if ( nodePath == parent && (tag == tagName( key )) ){
				
				if ( -1 == depth ) 
					paths.push_back( key );
				else {
					int dp = depthOf( key );

					if ( dp - npDepth > 0 && dp - npDepth <= depth ) 
						paths.push_back( key );
				}

			} else if ( nodePath != parent ){
				// DEBUG( "Rejected because parent does not match" )
				// DEBUG( "parent=" << parent << ", shouldBe=" << nodePath )
			} else if ( tag != tagName( key ) ){
				// DEBUG( "Rejected because tag does not match" )
				// DEBUG( "tag=" << tagName( it->first ) << ", shouldBe=" << tag )
			}
		}
		return paths;

	}

	string XmlConfig::basePath( string nodePath, bool keepAttribute ) const {
    	DEBUG( classname(), "(nodePath=\"" << nodePath << "\", keepAttrs=" << bts( keepAttribute ) << ")" );
    	string np = sanitize( nodePath );

    	// first split off any attributes
		vector<string> attr = split( np, attrDelim );
		if ( attr.size() >= 1 )
			np = attr[ 0 ];

		// now split by path
		vector<string> ntf = split( np, pathDelim );

		vector<string> goodPaths;
		for ( string p : ntf ){
			if ( "" == p ) continue;
			goodPaths.push_back( p );
		}
		// rebuild as a fully sanitized and normalized path
		string ret ="";
		for ( unsigned long int i = 0; i < goodPaths.size(); i++ ){
			
			ret += goodPaths[i];

			if ( i < goodPaths.size() - 1 )
				ret += pathDelim;
		}

		if ( keepAttribute ){
			ret += attr[ attr.size() - 1 ];
		}
		
		return ret;
    }

    string XmlConfig::join( std::vector<string> paths ) const {
    	if ( paths.size() == 1 ){
    		WARN( classname(), "Only one path given, returning unaltered" );
    		for ( string p : paths ){
    			return p;
    		}
    		return "";
    	} else if ( paths.size() < 1 ){
    		ERROR( classname(), "No paths given" );
    		return "";
    	} else {

    		string full = "";
    		unsigned long int count = 0;
    		for ( string p : paths ){
    			
    			// keep the attribute only on the last one
    			bool keepAttribute = false;
    			if ( count >= paths.size() - 1 )
    				keepAttribute = true; 

    			// get the base path
    			string tmp = basePath( sanitize( p ), keepAttribute );
    			if ( "" == tmp ) continue;

    			if ( count > 0 )
    				full += ( pathDelim + tmp );
    			else 
    				full += tmp;	// only first time

    			count ++;
    		}
    		return full;
    	}
    	return "";
    }

	vector< string > XmlConfig::attributesOf( string nodePath ) const{
		nodePath = sanitize( nodePath ) + attrDelim;

		vector<string> paths;
		for ( const_map_it_type it = data.begin(); it != data.end(); it++ ){
			const string &key = it->first; 
			// reject self
			if ( key == nodePath )
				continue;

			// look for attribute delimeter
			string::size_type found = key.find( attrDelim );
			if ( found == string::npos )
				continue;
			
			// push back the paths
			string parent = (key).substr( 0, nodePath.length() );
			if ( nodePath == parent ){
				paths.push_back( key );
			}
		}
		return paths;
	}

	map<string, string> XmlConfig::attributesMap( string nodePath ) const{
		DEBUG( classname(), "(" << nodePath << ")" )
		vector<string> pathToAttrs = attributesOf( nodePath );
		
		map<string, string> rmap;
		for ( string p : pathToAttrs ){
			rmap[ attributeName( p ) ] = getString( p );
		}
		return rmap;
	}


	vector<string> XmlConfig::query( string _qs ) const {
		// Query String _qs :
		// 1) nodePath* - all nodepaths matching base, star optional -> like child selector
		// 
		// 2) query test on attribute, something like:
		// query string = "FDS[0].DeltaTOF[0].XmlHistogram{name==signal}"
		// will search all XmlHistogram nodes under FDS[0].DeltaTOF[0].* for the first one 
		// with attribute "name" == "signal"
		
		XmlString xstr;
		vector<string> conds = xstr.tokens( _qs );
		string npc = trim( xstr.clean( _qs ), " \t\n*" ); // remove whitespace and '*'maybe we can improve star support later

		string np = "";
		string attr = "";

		vector<string> attrs = split( npc, attrDelim );

		if ( attrs.size() >= 1 )
			np = stripIndex( sanitize( attrs[0] ) );
		if ( attrs.size() >= 2)
			attr = attrDelim + attrs[1];

		DEBUG( classname(), "_qs == \"" << _qs << "\"" );
		DEBUG( classname(), "conditionals " << vts( conds ) );
		DEBUG( classname(), "wo-cond np " << quote( np ) );
		DEBUG( classname(), "attr" << quote( attr ) );

		int npl = np.length();

		vector<string> paths;

		for ( const_map_it_type it = data.begin(); it != data.end(); it++ ){
			string p = it->first;

			// Skip atribute paths
			size_t found = p.find( attrDelim );
			if ( found != string::npos )
				continue;
			
			string parent = p.substr( 0, npl );
			DEBUGC( "checking " << quote(np) << " against " << quote( parent ) << " from path " << quote( p ) );
			if ( np == parent ){

				if ( conds.size() < 1 )
					paths.push_back( p + attr );
				else if( passConditional( conds[0], p ) ){
					paths.push_back( p + attr );
				}
			}
		}

		// pre-conditional part
		
		
		return paths;
	}

	bool XmlConfig::passConditional( string cond, string nodePath ) const {

		// split by logical operators first
		if ( cond.find( "&&" ) != std::string::npos ){
			// TODO: add recursive splitting to allow logical operators && ||
		}
		// at this point we think that we have no logical operators only comparisons ==, >, <
		vector<string> parts = split( cond, "==" );
		string trimCs = " \t\n";
		trimCs += attrDelim;

		if ( parts.size() >= 2 ){
			DEBUG( classname(), "left: " << parts[0] );
			DEBUG( classname(), "right: " << parts[1] );

			DEBUG( classname(), "eval " << getXString( nodePath + ":" + parts[0] ) );

			return getXString( nodePath + attrDelim + trim(parts[0], trimCs ) ) == trim(parts[1]);
		} else {
			return exists( nodePath + attrDelim + trim(cond, trimCs ) );
		}

		return false;
	}

	vector<string> XmlConfig::getNodes( string nodePath ) const{

		nodePath = sanitize( nodePath );
		// for instance
		// case 1) test.sub should return test.sub[0...N]
		// case 2) test.sub:name should return all test.sub[0...N] with a name attribute
		// case 3) test.sub:name=dan should return test.sub[]:name == dan -> true
		
		vector<string> nodes = split( nodePath, pathDelim );
		vector<string> attrs = split( nodePath, attrDelim );
	
		// case 1) no attr given, just find siblings like given
		if ( attrs.size() <= 1  ){
			
			vector<string> paths;
			for ( const_map_it_type it = data.begin(); it != data.end(); it++ ){
				
				try {
					if ( isAttribute.at( it->first ) )
						continue;				
				} catch ( std::out_of_range &oor ){
					// TODO: nothing?
				}

				
				string parent = (it->first).substr( 0, nodePath.length() );
				if ( nodePath == parent ){
					paths.push_back( it->first );
				}
			}
			return paths;

		} else if ( attrs.size() == 2 ) {
			
			// get everything up to attr delim
			string baseNodePath = attrs[ 0 ];

			// split off the conditional
			vector<string> conds = split( attrs[ 1 ], equalDelim );
			
			// case 2) if no conditional is given just check for existance of attr
			string attrName = attrs[ 1 ];
			bool attrEquals = false;
			string attrIs = "";

			// case 3) if an equals is given
			if ( conds.size() == 2 ){
				attrIs = conds[ 1 ];
				attrName = conds[ 0 ];
				attrEquals = true;
			} else {
				
			}

			vector<string> paths;
			for ( const_map_it_type it = data.begin(); it != data.end(); it++ ){

				try {
					if ( isAttribute.at( it->first ) )
						continue;				
				} catch ( std::out_of_range &oor ){
					// TODO: nothing?
				}

				string wA = it->first + attrDelim + attrName;
				string parent = (it->first).substr( 0, baseNodePath.length() );

				if ( baseNodePath == parent ){
					bool aExists = exists( wA );
					
					// test for attribute existing
					if ( !aExists ){
						continue;
					}
					// if given, test for equality to value
					if ( aExists && attrEquals && attrIs != getString( wA ) ){
						continue;
					}
					
					paths.push_back( it->first );
				}
			}
			return paths;
		}

		vector<string> ret;
		return ret;
	}

	pair<string, string> XmlConfig::stringToPair( string &s, string delim  ) const{

		std::size_t delimPos = s.find( delim );

		if ( std::string::npos != delimPos ){

			// get the string before the delim
			string pA = trim( s.substr( 0, delimPos ) );
			// get the part after
			string pB = trim( s.substr( delimPos + delim.length() ) );
			return make_pair( pA, pB );
		}
		return make_pair( "", "");
	}

	int XmlConfig::depthOf( string nodePath ) const {
		nodePath = sanitize( nodePath );
		if ( "" == nodePath ) return -1;
		int depth = 0;
		for ( unsigned int i = 0; i < nodePath.length(); i++ ){
			if ( pathDelim == nodePath[ i ] )
				depth++;
		}
		return depth;
	}

	string XmlConfig::toXml( string nodePath, int tabCount, string tab, string nl ) const {

		string xml = "";

		// learn about this node
		string ind = indentation( tabCount, tab );
		nodePath   = basePath( nodePath );
		string tn  = tagName( nodePath );

		// cout << ind << "basePath = " << nodePath << endl;
		// cout << ind << "tagName = " << tn << endl;

		// scrub out include tags?
		// if ( "Include" == tn )
		// 	return ""; 
		// handle root node export:
		if ( "" == tn || "" == nodePath ) 
			tn = "config";

		string content = getString( nodePath );
		vector<string> children = childrenOf( nodePath, 1 );
		DEBUG( classname(), tn << " has " << children.size() << " children" );
		string childrens = "\n";
		for ( string c : children ){
			childrens += ind + c + "\n";
		}
		DEBUG( classname(), "children: " << childrens );

		// write the encoding if we are exporting from root node
		if ( "config" == tn && "" == nodePath )
			xml += ( nl + XmlConfig::declarationV1 + nl );

		// write the header
		xml += nl + ind + "<" + tn;

		map<string, string> attrs = attributesMap( nodePath );
		// add attributes
		for (auto a : attrs){
			xml += ( " " + a.first + "=\"" + a.second + "\"" );
		}			

		// Close tag inline or allow contents
		if ( 0 >= children.size() && "" == content )
			xml += "/>"; // inline close
		else
			xml += ">";	// just close the open tag

		// write contents if they exist
		if ( "" != content )
			xml += ( nl + ind + tab + content );

		// handle children
		if ( 0 < children.size() ){
			// recurse on children nodes
			for ( string cp : children ){
				DEBUG( classname(), "XML for " << cp );
				string cXml = toXml( cp, tabCount + 1, tab, nl );
				xml += cXml;	
			}
		}

		// write the closing tag if we didnt close inline
		if ( 0 < children.size() || "" != content )
			xml += nl + ind + "</" + tn + ">";
		
		return xml;
	}

	void XmlConfig::toXmlFile( string filename ) const {
		ofstream out;
		out.open( filename.c_str(), ios::out );

		if ( out.is_open() ){
			out << toXml(  );
			out.close();
		} else {
			ERROR( classname(), "Cannot open " << filename );
		}
	}


	string XmlConfig::dump() const {
		string msg = "";
		for ( auto kv : data ){
			msg += quote(kv.first) + " : " + quote(kv.second) + "\n";
		}
		return msg;
	}
	void XmlConfig::dumpToFile( string filename) const {
		ofstream out;
		out.open( filename.c_str(), ios::out );

		if ( out.is_open() ){
			out << dump(  );
			out.close();
		} else {
			ERROR( classname(), "Cannot open " << filename );
		}
	}

	void XmlConfig::add( string nodePath, string value ){
		DEBUG( classname(), "(" << nodePath << " = " << value << ", currentNode=" << currentNode << " )" );
		nodePath = sanitize( currentNode + nodePath );
		bool isAttr = (nodePath.find( attrDelim )!=std::string::npos);

		if( isAttr )
			addAttribute( nodePath, value );
		else
			addNode( nodePath, value );
	}


	int XmlConfig::unprocessedIncludes( string nodePath ){
		vector<string> allPaths = childrenOf( nodePath, "Include" );
		int nFound = 0;
		for ( string path : allPaths ){
			if ( getBool( path + ":processed" ) ) continue;
			nFound++;
		}
		return nFound;
	}

	void XmlConfig::include( XmlConfig otherCfg, string path, bool overwrite ){

		auto d = otherCfg.getDataMap();
		auto e = otherCfg.getNodeExistMap();
		auto a = otherCfg.getIsAttributeMap();

		vector<string> op2;
		map<string, string> d2;
		map<string, bool> e2;
		map<string, bool> a2;
		// probably a better way!

		string pp = sanitize( path );
		// prepend the path to the maps to make them scoped
		for ( auto kv : d ){
			// INFOC( "d[" << kv.first << "] -> d[ " << pp << pathDelim << kv.first << " ] = " << kv.second );
			d2[ pp + pathDelim + kv.first ] = kv.second;
		}
		for ( auto kv : e ){
			e2[ pp + pathDelim + kv.first ] = kv.second;
		}
		for ( auto kv : a ){
			a2[ pp + pathDelim + kv.first ] = kv.second;
		}

		// i chose to name variable overwrite to make it clear what the user is requesting when it is used
		merge( &op2, &d2, &e2, &a2, !overwrite );
		// this makes sure we dont have orphaned nodes
		ensureLineage( path );

	}

	void XmlConfig::ensureLineage( string path ){
		string p = sanitize( path );
		int depth = depthOf( p );

		for ( int i = 0; i <= depth; i++  ){
			string dp = pathToDepth( p, i );
			INFOC( "path[depth=" << i << "] = " << dp );
			if ( false == exists( dp ) )
				addNode( dp );
		}
	}

	int XmlConfig::parseIncludes( string searchPath) {
		DEBUG( classname(), " Looking for Include tags under : " << quote( searchPath ) );

		int nNotFound = 0;
		vector<string> allPaths = childrenOf( searchPath, "Include" );

		DEBUG( classname(), "Found " << allPaths.size() << " Include Tag(s)" );

		for ( string path : allPaths ){
			DEBUG( classname(), path );
			DEBUG( classname(), "parent path: " << pathToParent( path ) );

			if ( getBool( path + ":processed" ) ) continue;

			string ifn = getXString( path + ":url" );
			struct stat buffer;
			bool exists = (stat (ifn.c_str(), &buffer) == 0);
			DEBUG( classname(), "file " << ifn << " exists " << exists )

			// if we can't find it from the path directly then try relative to base config path
			if ( !exists ) { // try relative to this config file
				string basePath = pathFromFilename( filename );
				ifn = basePath + ifn;

				exists = (stat (ifn.c_str(), &buffer) == 0);
				DEBUG( classname(), "file " << ifn << " exists " << exists )
			}

			if ( exists ){
				#ifndef __CINT__
				RapidXmlWrapper rxw(  ifn  );
				#endif

				map<string, string> tmpData;
				vector<string> tmpOrderedPaths;
				map<string, bool> tmpIsAttribute;
				map<string, bool> tmpNodeExists;
				
				rxw.includeMaps( pathToParent( path ), &tmpOrderedPaths, &tmpData,  &tmpIsAttribute, &tmpNodeExists );
				merge( &tmpOrderedPaths, &tmpData,  &tmpIsAttribute, &tmpNodeExists );


				applyOverrides( makeOverrideMap( path ) );

				addAttribute( path + ":processed", "true" );

				// now look for new includes and parse those
				if ( unprocessedIncludes( pathToParent(path) ) > 0 ){
					parseIncludes( pathToParent(path) );
				}

			} else {
				WARN( classname(), "Include not found: " << ifn );
				nNotFound++;
			}

		}


		return nNotFound;
	   //DEBUG( report() );
	}

	bool XmlConfig::conflictExists( map<string, string> *_data, string &shortestConflict ){
		
		bool conflicts = false;
		for ( auto kv : *_data ){
			if ( this->data.count( kv.first ) >= 1 ){
				if ( false == conflicts && pathIndex( kv.first ) >= 0){
					
					shortestConflict = kv.first;
				}
				else if ( kv.first.length() < shortestConflict.length() && pathIndex( kv.first ) >= 0 ){
					
					shortestConflict = kv.first;
				}
				conflicts = true;
			}
		} // loop on _data
		shortestConflict = stripIndex( shortestConflict );
		return conflicts;
	}

	void XmlConfig::merge( vector<string> *_orderedPaths, map<string, string> *_data, map<string, bool> *_isAttribute, map<string, bool> *_exists, bool resolveConflicts ){


		////////!!!!!!!!!!!!!!! TODO
		/// ADD support for new ordered Paths!!!!!!
		/// !!!!!
		/// !!!!!
		/// !!!!!
		/// 


		string shortestConflict = "";
		bool conflicts = conflictExists( _data, shortestConflict );
		INFOC( "Attempting to merge config, conflicts = " << bts( conflicts ) );
		while( conflicts && resolveConflicts ){
			map<string, string> _d;
			map<string, bool> _ia;
			map<string, bool> _e;
			vector<string> _op;

			int cDepth = depthOf( shortestConflict );
			int incBy = numberOf( shortestConflict );
			if ( incBy <= 0 ) incBy = 1;
			// cout << "Depth of conflict (" << quote(shortestConflict) <<") = " << depthOf( shortestConflict) << endl;
			// cout << "increment by " << incBy << endl;

			for ( auto kv : *_data ){
				string str = kv.first;
				
				// no conflict here
				if ( str.find( shortestConflict ) == string::npos ) {
					// cout << "[NO CONFLICT] " << str << " with " << shortestConflict << endl;
					_d[ str ] = (*_data)[ kv.first ];
					_ia[ str ] = (*_isAttribute)[ kv.first ];
					_e[ str ] = (*_exists)[ kv.first ];
					continue;
				}

				string ptr = pathToDepth( str, cDepth );
				string npp = incrementPath( pathToDepth( str, cDepth ), incBy );
				
				string::size_type index = 0;
				index = str.find( ptr, index);

				if ( index != string::npos ){
					/* Make the replacement. */
					str.replace(index, ptr.length(), npp );
					// cout << "[REPLACE] " << ptr << ", with " << npp << " = " << str << endl;

					_d[ str ] = (*_data)[ kv.first ];
					_ia[ str ] = (*_isAttribute)[ kv.first ];
					_e[ str ] = (*_exists)[ kv.first ];
				}
			}

			// replace existing
			(*_data) = _d;
			(*_isAttribute) = _ia;
			(*_exists) = _e;

			conflicts = conflictExists( _data, shortestConflict );
		} // while

		INFOC( "Conflicts resolved" );
		for ( auto kv : *_data ){
			this->data[ kv.first ] = kv.second;
		}
		for ( auto kv : *_isAttribute ){
			this->isAttribute[ kv.first ] = kv.second;
		}
		for ( auto kv : *_exists ){
			this->nodeExists[ kv.first ] = kv.second;
		}

	}

	int XmlConfig::numberOf( string _path ){
		int n = 0;
		string sstr = stripIndex( _path );
		//assuming they are in order
		while ( true ){
			if ( !exists( addIndex( sstr, n ) ) ) break;
			n++;
		}
		return n;
	}
	string XmlConfig::incrementPath( string _in, int _n ){
		string base = basePath( _in );
		vector<string> byIndex = split( _in, indexOpenDelim[0] );
		if ( byIndex.size() >= 2 ){
			base = byIndex[0];
		}

		return base + indexOpenDelim + ts( pathIndex( _in )+_n ) + indexCloseDelim;
	}

	int XmlConfig::pathIndex( string _in ) const{
		string::size_type first = _in.find(indexOpenDelim[0]);
		string::size_type last  = _in.find(indexCloseDelim[0]);

		if ( string::npos == first || string::npos == last ) return -1;

		string between = _in.substr(first+1,last-first-1);
		return atoi( between.c_str() );
	}

	void XmlConfig::applyOverrides( map< string, string > over ) {
		DEBUG( classname(), "Applying Overrides" );
		for ( auto k : over ){
			DEBUG( classname(), "Override [" << k.first << " ] = " << k.second );
			set( k.first, k.second );
		}
	}

	void XmlConfig::set( string nodePath, string value ) {
		DEBUG( classname(), "nodePath = " << quote(nodePath) << ", value=" << quote(value) << ", currentNode=" << currentNode << " )" );
		string fqn = currentNode + nodePath;
		// already exists? just override
		if ( data.count( fqn ) ){
			data[ fqn ] = value;
			DEBUG( classname(), quote(fqn) << " set to " << value );
			DEBUG( classname(), "now = " << data[ fqn ] );
		} else {
			add( nodePath, value );
			DEBUG( classname(), "add" );
		}
	}

	string XmlConfig::report( string nodePath ) const {

		vector<string> allPaths = childrenOf( nodePath, -1, true );

		stringstream sstr;
		for ( string path : allPaths ){
			string val = getString( path, "" );
			if ( "" != val )
				sstr << path << " === " << val << endl;
		}
		return sstr.str();

	}

	void XmlConfig::addNode( string nodePath, string value ) {
		DEBUG( classname(), "(" << nodePath << " = " << value << ")" );
		if ( exists( nodePath ) ){
			WARN( classname(), "Overwriting nodePath " << nodePath );
		}

		data[ nodePath ] = value;
		nodeExists[ nodePath ] = true;
	}

	void XmlConfig::addAttribute( string nodePath, string value ){
		DEBUG( classname(), "(" << nodePath << " = " << value << ")" );
		if ( exists( nodePath ) ){
			WARN( classname(), "Overwriting nodePath " << nodePath );
		}

		// TODO: test nodePath for attribute char, set isAttribute map and exists map
		// 

		
		string base = basePath( nodePath );
		if ( !exists( base  ) )
			addNode( base, "" );
		
		// add myself
		data[ nodePath ] = value;
		isAttribute[ nodePath ] = true;
		nodeExists[ nodePath ] = true;

	}

	void XmlConfig::deleteNode( string nodePath ){
		if ( !exists( nodePath ) ) return;

		vector<string> paths = childrenOf( nodePath, -1, true );

		// first delete children
		for ( string path:paths ){
			data.erase( nodePath );
			isAttribute.erase( nodePath );
			nodeExists.erase( nodePath );			
		}

		// now delete self
		data.erase( nodePath );
		isAttribute.erase( nodePath );
		nodeExists.erase( nodePath );
	}

	// just an alias
	void XmlConfig::deleteAttribute( string path ){
		deleteNode( path );
	}

	map<string, string> XmlConfig::makeOverrideMap( string _nodePath ){

		// all depths below and include attributes
		vector<string> keys = childrenOf( _nodePath + ".", -1, true );
		map<string, string> over;

		string ppath = pathToParent( _nodePath );

		DEBUGC( "key = " << vts( keys ) );
		DEBUGC( "ppath = " << quote( ppath ) );
		DEBUGC( "# of children " << keys.size() );

		string sMap = "";
		for ( string key : keys ){
			
			string nKey = key;
			nKey.replace( nKey.begin(), nKey.begin() + _nodePath.length()+1, "" );
			if ( "url" == nKey || "processed" == nKey ) continue;	// skip the url, processed since it is part of Include tag (doesnt hurt to include)
			
			nKey = join( ppath, nKey );
			over[ nKey ] = getString( key );
			sMap += "\n\t" + key + " => [ " + nKey + " ] = " + over[nKey];
		}
		DEBUGC( sMap );
		return over;
	}




}