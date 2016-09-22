#include "HistoAnalyzer.h"

namespace jdb{

	HistoAnalyzer::HistoAnalyzer(){	} 

	void HistoAnalyzer::init( XmlConfig &_config, string _nodePath ){
		TaskRunner::init( _config, _nodePath );

		int _jobIndex = config.getInt( "jobIndex", -1 );

		jobIndex = _jobIndex;
		string jobPostfix = "_" + ts( _jobIndex ) + ".root";
		
		if ( -1 >=_jobIndex )
			jobPostfix = ".root";

		this->jobModifier = "job_" + ts( _jobIndex ) +"_";

		if ( -1 == _jobIndex ){
 			jobPostfix = ".root";
 			this->jobModifier = "";
 		}
 		
		initHistoBook( jobPostfix );
		initReporter( jobPostfix );
		initRootFiles();

		initialize();
	}

	// void HistoAnalyzer::init( XmlConfig &_config, string _nodePath, string _fileList, string _jobPostfix ){
	// 	TaskRunner::init( _config, _nodePath, _fileList, _jobPostfix );

	// 	this->jobModifier = _jobPostfix;
	// 	initHistoBook( _jobPostfix );
	// 	initReporter( _jobPostfix );
	// 	initRootFiles();

	// 	initialize();
	// }

	void HistoAnalyzer::initHistoBook( string _jobPostfix ){
		initializeHistoBook( config, nodePath, _jobPostfix );
	}

	void HistoAnalyzer::initReporter( string _jobPostfix ){

		INFO( classname(), "Creating Reporter" );
		string pRepOut = config.join( nodePath, "Reporter", "output:url" );
		string outputURL = config[ pRepOut ];

	   	// Default reporter
	   	// dont make for parallel!
	    if ( ".root" == _jobPostfix && config.exists( pRepOut ) ) {
		    reporter = shared_ptr<Reporter>(
		    	new Reporter( 	config, 
		    					config.join( nodePath, "Reporter" ), 
		    					"" 
		    				) 
		    	); // TODO: is reporter's path handeling broken?

		    INFO( classname(), "Creating report @" << outputURL );
	    } else {
	    	INFO( classname(), "No Reporter created, jobPostfix == " << _jobPostfix );
			reporter = nullptr;
	    }
	}


	void HistoAnalyzer::initRootFiles(){
		vector<string> filenames;
		string inf = "";
		/***************************************/
		// nodePath.input.TFile:url

		vector<string> pathTFile = config.childrenOf( nodePath + ".input", "TFile" );
		INFO( classname(), "Found " << pathTFile.size() << plural( pathTFile.size(), " TFile", " TFiles" ) );

		if ( pathTFile.size() >= 1 ){
			for ( string tfPath : pathTFile ){
				string fn = config.getXString( tfPath + ":url" );
				string name = config.getString( tfPath + ":name", config.getString( tfPath + ":as", fn ) );
				INFO( classname(), "Loading " << fn << " with name = " << name );
				addRootFile( fn, name );
				setCurrentFile( name );
			}
		} else if ( config.exists( nodePath + ".input.data:url" ) ){
			// TODO : Add multiple file support!
			inf = config.getString( nodePath + ".input.data:url" );
		} else {
			WARN( classname(), "No input file given!" );
			WARN( classname(), "Search paths:" );
			WARN( classname(), nodePath  << ".input.TFile:url" );
			WARN( classname(), nodePath  << ".input.data:url" );
		}

		if ( "" != inf ){
			DEBUG( classname(), " Loading data from " << inf )
			inFile = new TFile( inf.c_str(), "READ" );
		}

		if ( !inFile->IsOpen() ){
			ERROR( classname(), "Data File Could not be opened from : " + inf );
		}
	}

	HistoAnalyzer::~HistoAnalyzer(){
		
		// book->save();
	}

}
