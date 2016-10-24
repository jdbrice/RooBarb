#include "DataSource.h"
#include "ChainLoader.h"
#include "Utils.h"
#include <stdio.h>


#include "TLeafElement.h"

namespace jdb{
	
	DataSource::DataSource( XmlConfig _config, string _nodePath, string _treeName, TChain * _chain ){
		// DEBUG( classname(), fmt::format( "confif={0}, nodePath={1}, treeName={2}", _config.getFilename(), _nodePath, _treeName ) );

		config = _config;
		nodePath = config.basePath( _nodePath );
		cache = NULL;
		chain = _chain;
		treeName = _treeName;
		
		if ( chain && chain->GetNtrees() >= 1 ){
			INFO( classname(), "time for init" );
			init();
		} else {
			ERROR( classname(), "Chain is empty, cannot build a DataSource Object" );
		}
	}


	DataSource::DataSource( string _treeName, TChain * _chain ){
		// DEBUG( classname(), fmt::format( "treename={0}, chain=0x0", _treeName ) );

		cache = NULL;
		chain = _chain;
		treeName = _treeName;
		
		if ( chain && chain->GetNtrees() >= 1 ){
			INFO( classname(), "time for init" );
			init();
		} else {
			ERROR( classname(), "Chain is empty, cannot build a DataSource Object" );
		}
	}

	void DataSource::init(){

		
		/**
		 * Now create the data access structure
		 */
		if ( cacheExists() ){
			INFO( classname(), "Loading cache from " << (".DataSource_" + treeName + ".xml") );
			cache = new XmlConfig( ".DataSource_" + treeName + ".xml" );
		} else{
			INFO( classname(), "No cache found for " << treeName  );
		}

		mapTree();
		addressBranches();
		addressLeaves();

		initializeBranchStatus();
		makeAliases();
		makeEvaluatedLeaves();
		cacheTreeInfo();

	}

	DataSource::~DataSource(){

		INFO( classname(), "Cleaning up" );

		if (cache)
			delete cache;

		typedef map<string, void*>::iterator svp_it_type;
		for(svp_it_type iterator = data.begin(); iterator != data.end(); iterator++) {

			if ( iterator->second ){
				DEBUG( classname(), "Releasing " << iterator->first );
				free( data[ iterator->first ] );
			}

		}

		typedef map<string, EvaluatedLeaf*>::iterator sel_it_type;
		for(sel_it_type it = evalLeaf.begin(); it != evalLeaf.end(); it++) {

			if ( it->second ){
				DEBUG( classname(), "Releasing " << it->first );
				delete evalLeaf[ it->first ];
			}

		}

		DEBUG( classname(), "Memory Released" );

	}


	void DataSource::mapTree(){

		DEBUG( classname(), "" );

		assert( chain );

		// Set the Tree into decomposed object mode
		chain->SetMakeClass(1);

		nEntries = chain->GetEntriesFast();
		nTrees = chain->GetNtrees();
		INFO( classname(), "Chain contains " << nTrees << " TTrees" );


		/**
		 * Investigate the Branches
		 */
		TObjArray * brs = chain->GetListOfBranches();
		int l = brs->GetEntries();

		for ( int i = 0; i < l; i++ ){
			string name = ((TBranchElement*)brs->At( i ))->GetName();
			DEBUG( classname(), "Branch " << name );
			branchName.push_back( name );
			
			// sort of a hack
			leafType[ name + "_" ] = "Int_t";
			leafLength[ name + "_" ] = 1;
			leafType[ name ] = "Int_t";
			leafLength[ name ] = 1;
			
			mapBranch( ((TBranchElement*) brs->At( i ) ), name );
		}

		/**
		 * Now get the lengths of every leaf for the whole chain. 
		 * This must be done in a sep loop bc the leaf pointers are 
		 * not consistent accross TTrees
		 */
		if ( cache && cache->getInt( "TreeInfo:nTrees" ) >= nTrees ){
			loadLengthsFromCache();
		} else {
			TaskTimer tt;
			tt.start();
			for ( unsigned int i = 0; i < leafName.size(); i++ ){
				string name = leafName[ i ];
				leafLength[ name ] = chainLeafLength( name );
			}
			INFO( classname(), "Finding Lengths : Elapsed " << tt.elapsedTime() );
		}
		INFO( classname(), "Complete" );
		
	}

	void DataSource::initializeBranchStatus(){

		vector<string> children = config.childrenOf( nodePath );

		for ( unsigned int i = 0; i < children.size(); i++ ){

			// we are only interested in the branch status nodes
			if ( "BranchStatus" == config.tagName( children[ i ] )){
				string path = children[ i ];
				int status = config.getInt( path + ":status", 0 );
				string active = "Active";
				if ( !status )
					active = "Inactive";

				vector<string> bNames = config.getStringVector( path );

				for ( unsigned int ib = 0; ib < bNames.size(); ib++ ){
					INFO( classname(), "Setting " << bNames[ ib ] << " to " << active );
					chain->SetBranchStatus( bNames[ ib ].c_str(), status );
				} // loop on branch names
			} // require node to be a BranchStatus node
		} // loop on children of nodepPath
	}
	void DataSource::makeAliases(){
		vector<string> children = config.childrenOf( nodePath );

		for ( unsigned int i = 0; i < children.size(); i++ ){

			// we are only interested in the branch status nodes
			if ( "Alias" == config.tagName( children[ i ] )){
				string path = children[ i ];
				
				string aName = config.getString( path + ":name" );
				string pointsTo = config.getString( path + ":pointsTo" );

				addAlias(aName, pointsTo );

			} // require node to be a Alias node
		} // loop on children of nodepPath
	}

	void DataSource::makeEvaluatedLeaves(){
		vector<string> children = config.childrenOf( nodePath );

		for ( unsigned int i = 0; i < children.size(); i++ ){
			string path = children[ i ];
			// we are only interested in the branch status nodes
			if ( "EvaluatedLeaf" == config.tagName( path ) && config.exists( path+":name" ) && config.exists( path+":value" ) ){
				string name = config.getString( path+":name" );
				EvaluatedLeaf * evl = new EvaluatedLeaf( &config, path );
				evalLeaf[ name ] = evl;

			} // require node to be a Alias node
		} // loop on children of nodepPath


	}

	int DataSource::singleTreeLeafLength( string name ){


		int dim = 0;

		TLeaf * l = chain->GetLeaf( name.c_str() );
		if ( !l ){
			DEBUG( classname(), name << " is not a leaf" );
			DEBUG( classname(), "Trying to get Branch->Leaf" )
			TBranch * b = chain->GetBranch( name.c_str() );
			if ( b ){

				size_t dotp = name.find( '.' );
				if ( dotp != std::string::npos ){
					string lname = name.substr( dotp + 1);
					DEBUG( classname(), "Now looking for '" << lname << "' in the branch " << name  )

					if ( b->GetLeaf( lname.c_str() ) ){
						l = b->GetLeaf( lname.c_str() );
						DEBUG( classname(), "Got usable branch" ); 
					} else 
						return dim;
				} else 
					return dim;

			} else {
				DEBUG( classname(), "Unable to find length for " << name )
				return dim;
			}
		}

		int nElem = 0;
		TLeaf* cl = l->GetLeafCounter( nElem );
		Int_t length = l->GetLen();
		if ( 0 >= length)
			length = 1;

		DEBUG( classname(), "\t" << l->GetTitle() );
		DEBUG( classname(), "\tLength = " << length );
		DEBUG( classname(), "\tMaximum = " << l->GetMaximum() );
		DEBUG( classname(), "\tnElem = " << nElem );
		if ( cl )
			DEBUG( classname(), "\tCount Leaf Maximum = " << cl->GetMaximum() );

		if ( 1 == nElem && cl )	{ // Leaf like pt[ nTracks ] where nTracks is another leaf
			dim = cl->GetMaximum();
			if ( 0 == dim )
				dim = 1;
			DEBUG( classname(), "\t\tCase 1" );
		} else if ( 1 <= nElem && !cl ){ // Leaf like channelHits[ 19 ] ( ie fixed array size )
			// could be single or multi-dim array based on the title
			// eg 	var[10][20] => nElem = 200
			// 		var[200] => nElem = 200
			// 		same memory footprint anyways so no worries really
			dim = nElem;
			DEBUG( classname(), "\t\tCase 2" );
		} else {
			DEBUG( classname(), "\tCould not determine dimensions of " << name );
			return -1;
		}

		TRACE( classname(), "\t" << name << "@size ==> " << dim * length );
		return dim * length;
	}


	int DataSource::chainLeafLength( string name ) {

		//The way root does the reporting it is necessary to 
		//find the max on each tree individually. The max for 
		//the entire chain is then the maximum value of each tree max 
		int max = 0;
		int nt = chain->GetNtrees();
		DEBUG( classname(), "Looping over " << nt << " trees" );
		for ( int i = 0; i < nt; i ++ ){
			Long64_t first = chain->GetTreeOffset()[i];
			chain->LoadTree( first );
			DEBUG( classname(), "Getting leaf length for tree " << i );
			int stl = singleTreeLeafLength( name );
			if ( stl > max) 
				max = stl;
			
		}
		// reload the first tree to put us back at the beg.
		chain->LoadTree( 0 );
		TRACE( classname(), name << "@size = " << max );

		return max;
	}

	void DataSource::addressBranches(){

		for ( unsigned int i = 0; i < branchName.size(); i ++ ){

			// get the data type from the "leaf" stored as "BranchName_"
			string bName = branchName[ i ];
			string lName = bName + "_";

			DEBUG( classname(), "Addressing branch " << bName );

			int ms = memSize( lName );

			if ( 0 < ms ){
				data[bName] = malloc( ms );
				branches[ bName ] = 0;
				chain->SetBranchAddress( bName.c_str(), data[bName], &branches[ bName ] );
				// DEBUG( classname(), fmt::format("chain->SetBranchAddress( {0}, data[{0}], &branches[ {0} ] );", bName));
				DEBUG( classname(), leafType[ lName ] << " " << bName << "(" << lName << ") addressed to " << ms << " bytes" );
			} else {
				DEBUG( classname(), "Cannot address branch" << bName );	
			}
			

		}

	}

	void DataSource::addressLeaves() {

		for ( unsigned int i = 0; i < leafName.size(); i++ ){

			string name = leafName[ i ];
			DEBUG( classname(), "Leaf Name = " << name )
			//Skip "leaves" for top level branches since these are already taken care of
			TString addressName(name);
			if ( addressName.EndsWith( "_" ) )	
				continue;

			if ( "" == name )
				continue;

			int ms = memSize( name );
			DEBUG( classname(), "Allocating " << ms << " bytes for " << name );

			if ( 0 == ms )
				continue;

			// Allocate the memory
			data[ name ] = malloc( ms );

			//Change fixed length arrays from "array" to "array[length]" since that is how they are addressed
			TLeaf * l = chain->GetLeaf( name.c_str() );
			//lg.debug( __FUNCTION__ ) << name << " : " << leafLength[ name ] << ", " << l->GetLen() << endl;
			//EXPERIMENTAL
			if ( l ){
				Int_t len = l->GetLen();
				if ( len >= 2  && len  == leafLength[ name ] ){
					string nName = name + "[" + ts(len) + "]";
					DEBUG( classname(), l->GetTitle() << " : " << name << " --> " << nName );
					name = nName;
				}	

				TBranch * b = l->GetBranch();
				if ( b )
					DEBUG( classname(), "Leaf's Branch Address : " << b  )
			}
			


			// Address the leaves
			// branches[ name ] = 0;
			chain->SetBranchAddress( name.c_str(), data[ name ], &branches[ name ] );
			DEBUG( classname(), "Addressed " << name );
		}
	}

	double DataSource::get( string name, int i ){

		// Aliased leaves
		// Allows possible recursive alias (good or bad? )
		if ( alias.find( name ) != alias.end() ){ // name is an alias
			name = alias[ name ];
			return get( name, i );
		}
		// Evaluated leaves
		if ( evalLeaf.find( name ) != evalLeaf.end() ){
			return evalLeaf[ name ]->eval( this, i );
		}

		if ( i >= leafLength[ name ] || i < 0 ){
			DEBUG( classname(), name << "[ " << i << " ] Out Of Bounds" );
			return numeric_limits<double>::quiet_NaN();
		}
		if ( !data[ name ]){
			DEBUG( classname(), name << "[ " << i << " ] Invalid data" );
			return numeric_limits<double>::quiet_NaN();	
		}

		// we need to lookup the data pointer, re-cast it and look it up	
		if ( "Int_t" == leafType[ name ] ){
			Int_t * tData = (Int_t*)data[ name ];
			return tData[ i ];
		}else if ( "UInt_t" == leafType[ name ] ){
			UInt_t * tData = (UInt_t*)data[ name ];
			return tData[ i ];
		}else if ( "Short_t" == leafType[ name ] ){
			Short_t * tData = (Short_t*)data[ name ];
			return tData[ i ];
		}else if ( "UShort_t" == leafType[ name ] ){
			UShort_t * tData = (UShort_t*)data[ name ];
			return tData[ i ];
		}else if ( "Char_t" == leafType[ name ] ){
			Char_t * tData = (Char_t*)data[ name ];
			return tData[ i ];
		}else if ( "UChar_t" == leafType[ name ] ){
			UChar_t * tData = (UChar_t*)data[ name ];
			return tData[ i ];
		}else if ( "Float_t" == leafType[ name ] ){
			Float_t * tData = (Float_t*)data[ name ];
			return tData[ i ];
		}else if ( "Double_t" == leafType[ name ] ){
			Double_t * tData = (Double_t*)data[ name ];
			return tData[ i ];
		}	

		return numeric_limits<double>::quiet_NaN();

	}

	void DataSource::cacheTreeInfo(){


		if ( cacheExists() && ( (cache) && cache->getInt( "TreeInfo:nTrees" ) >= nTrees) ){
			INFO( classname(), "No need to recache" );
			// no need to recache
			return;
		}
		
		/*if ( cacheExists() ){
			
			delete cache;
			cache = NULL;
			remove( (".DataSource_" + treeName + ".xml").c_str() );
		}*/

		ofstream cacheFile( ".DataSource_" + treeName + ".xml" );

		cacheFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		cacheFile << "<root>" << endl;
		cacheFile << "\t<TreeInfo nTrees=\"" << nTrees << "\">" << endl;
		typedef map<string, int>::iterator si_it_type;
		for(si_it_type iterator = leafLength.begin(); iterator != leafLength.end(); iterator++) {

			if ( iterator->second ){
				cacheFile << "\t\t<LeafInfo name=\"" << iterator->first << "\" size=\"" << iterator->second <<"\" /> " << endl; 
			}
		}
		cacheFile << "\t</TreeInfo>" << endl;
		cacheFile << "</root>" << endl;

		cacheFile.close();

	}

	void DataSource::loadLengthsFromCache(){

		DEBUG( classname(), "" );
		vector<string> cli = cache->childrenOf( "TreeInfo" );
		for ( unsigned int i = 0; i < cli.size(); i++ ){

			string ln = cache->getString( cli[ i ] + ":name" );
			int ls = cache->getInt( cli[ i ] +":size" );
			leafLength[ ln ] = ls;
		}
	}


	void DataSource::mapBranch( TBranchElement* br, string pname ){
		//DEBUG( "branch=" << br << " name=" << pname )
		TObjArray * brs = br->GetListOfBranches();
		int l = brs->GetEntries();

		for ( int i = 0; i < l; i++ ){
			string name = ((TBranchElement*)brs->At( i ))->GetName();
			//lg.debug() <<  name << endl;

			mapBranch( ((TBranchElement*)brs->At( i )), name );
		}

		TObjArray * lfs = br->GetListOfLeaves();
		l = lfs->GetEntries();

		for ( int i = 0; i < l; i++ ){
			string tName = ((TLeafElement*)lfs->At( i ))->GetTypeName();
			string name = ((TLeafElement*)lfs->At( i ))->GetName();
			

			if ( !((TLeafElement*)lfs->At( i ))->IsOnTerminalBranch() ){
				DEBUG( classname(), tName << " " <<  pname << "." << name );

				leafName.push_back( pname + "." + name );
				leafType[ pname + "." + name ] = tName; 
			}
			else {
				DEBUG( classname(), tName << " " <<  pname );
				leafName.push_back( pname );
				leafType[ pname ] = tName;	
			}
		}


	}


}