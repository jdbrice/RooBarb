

#include "ChainLoader.h"
#include <fstream>

using namespace std;

namespace jdb{

	void ChainLoader::load( 
							TChain * chain, 	// the chain object to fill
							string ntdir, 	// the directory in which to look for ntuples
							int maxFiles 
							) {
		INFO( "ChainLoader", "( chain=" << chain << ", dir=" << ntdir << ", maxFiles=" << maxFiles << ")" );

		
		if ( chain == nullptr ){
			ERROR( "ChainLoader", "badd chain" );
			return;
		}

		if ( ntdir.find( ".root" ) != std::string::npos ){

			chain->AddFile( ntdir.c_str() );


		} else {
			DEBUG( "ChainLoader", "Searching " << ntdir << " for ntuples" );

			if ( ntdir[ ntdir.length()-1] != '/' ){
				DEBUG( "ChainLoader", "Appending / to path" );
				ntdir += "/";
			}

			if (maxFiles <= 0)
				maxFiles = 10000000;

			int nFiles = 0;
			DIR *dir;
			struct dirent *ent;
			bool go = true;
			if ( (dir = opendir ( ntdir.c_str() ) ) != NULL) {
				
				while ( go && (ent = readdir ( dir) ) != NULL) {

			    	if ( strstr( ent->d_name, "root") ){
			    		
			    		if ( nFiles >= maxFiles ) {
			    			go = false;
			    			break;
			    		}

			    		char fn[ 1024 ];
			    		sprintf( fn, "%s%s", ntdir.c_str(), ent->d_name );
			    		chain->Add( fn );

			    		DEBUG( "ChainLoader", "Adding file " << fn << " to chain" );
			    		nFiles++;

			    	}
			  	}
			  	
			  	DEBUG( "ChainLoader", nFiles << " files loaded into chain" );

			  	closedir (dir);
			}
		} // if a directory was given

	}

	void ChainLoader::loadRange( 
							TChain * chain, 	// the chain object to fill
							string ntdir, 		// the directory in which to look for ntuples
							int firstFile,
							int nFiles
							) {
		INFO( "ChainLoader", "( chain=" << chain << ", dir=" << ntdir << ", firstFile=" << firstFile << ", nFiles" << nFiles << ")" );

		Logger * logger = new Logger( Logger::llGlobal, "ChainLoader" );

		logger->info(__FUNCTION__) << "Searching " << ntdir << " for ntuples" << endl;

		if ( ntdir[ ntdir.length()-1] != '/' ){
			logger->info(__FUNCTION__) << "Appending / to path" << endl;
			ntdir += "/";
		}
        int iFile = 0;
		DIR *dir;
		struct dirent *ent;
		bool go = true;
		if ( (dir = opendir ( ntdir.c_str() ) ) != NULL) {
			
			while ( go && (ent = readdir ( dir) ) != NULL) {

		    	if ( strstr( ent->d_name, "root") ){
		    		
		    		if ( iFile >= firstFile && iFile - firstFile < nFiles ) {
		    			string fullName = ntdir + ent->d_name;	
		    			chain->Add( fullName.c_str() );
		    		}
		    		iFile++;
		    	} // is file a .root file

		  	} // while files in directory
		  	
		  	logger->info( __FUNCTION__ ) << iFile << " files loaded into chain" << endl;
		  	delete logger;

		  	closedir (dir);
		}

	}	



	void ChainLoader::loadList(  TChain * _chain, string _listFile, int _maxFiles ){
		
		string classname = "ChainLoader";
		INFO( classname, "( chain" << _chain << ", listFile=" << _listFile << ", maxFiles=" << _maxFiles << " )" );

		int nFiles = 0;

		string line;
		ifstream fListFile( _listFile.c_str());
		if ( fListFile.is_open() ){

			while ( getline( fListFile, line ) ){
				_chain->Add( line.c_str() );
				DEBUG( classname, "Adding File : " << line );
				nFiles++;

				if ( _maxFiles >= 1 && nFiles >= _maxFiles ){
					break;
				}

			}
			fListFile.close();
				

		} else {
			ERROR( classname, "Could not open " << _listFile );
		}

		INFO( classname, nFiles << " files loaded into chain" );
	} // loadList



	void ChainLoader::loadListRange(  TChain * _chain, string _listFile, int _jobIndex, int _splitBy ){
		
		string classname = "ChainLoader";
		INFO( classname, "( chain" << _chain << ", listFile=" << _listFile << ", index=" << _jobIndex << ", splitBy=" << _splitBy << ")" )
		
		unsigned long int min = _jobIndex * _splitBy;
		unsigned long int max = (_jobIndex + 1) * _splitBy - 1;
		DEBUG( classname, "range = (" << min << ", " << max << ") " );
		
		if ( 0 > _splitBy || 0 > _jobIndex ){
			DEBUG( classname, "splitBy or jobIndex are negative" );
			DEBUG( classname, "Accepting all files, not a range" );
			ChainLoader::loadList( _chain, _listFile, -1 );
			return;
		}

		unsigned long int fileIndex = 0;

		string line;
		ifstream fListFile( _listFile.c_str());
		if ( fListFile.is_open() ){

			while ( getline( fListFile, line ) ){
				
				if ( fileIndex < min || fileIndex > max ){
					// do not add
				} else {
					_chain->Add( line.c_str() );
					DEBUG( classname, "Adding File[" << fileIndex << "] : " << line );
				}
				fileIndex ++;
			}

			fListFile.close();
		} else {
			ERROR( classname, "Could not open " << _listFile );
		}
	} // loadListRange


}
