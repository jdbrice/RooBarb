#include "SharedTreeAnalyzer.h"

#include "TaskFactory.h"


namespace jdb{

		SharedTreeAnalyzer::SharedTreeAnalyzer(){

		}

		SharedTreeAnalyzer::~SharedTreeAnalyzer(){
			
		}

		void SharedTreeAnalyzer::init( XmlConfig &_config, string _nodePath ){
			DEBUG( classname(), "( " << _config.getFilename() << ", " << nodePath << " )" );
			TaskRunner::init( _config, _nodePath );


			this->jobIndex = config.getInt( "jobIndex", -1 );

	 		initLogger();
			initDataSource( this->jobIndex );
			
			DEBUG( classname(), "Common Initialization" );
			initialize();
		}


		// void SharedTreeAnalyzer::init( XmlConfig &_config, string _nodePath, int _jobIndex ){
		// 	DEBUG( classname(), "( " << _config.getFilename() << ", " << nodePath << ", " << _jobIndex << " )" );
		// 	TaskRunner::init( _config, _nodePath, _jobIndex );

		// 	this->jobIndex = _jobIndex;

	 // 		initLogger();
		// 	initDataSource( _jobIndex );
			
		// 	DEBUG( classname(), "Common Initialization" );
		// 	initialize();
		// }

		// void SharedTreeAnalyzer::init( XmlConfig &_config, string _nodePath, string _fileList, string _jobPostfix ){
		// 	DEBUG( classname(), "( " << _config.getFilename() << ", " << nodePath << ", \"" << _fileList << "\", \"" << _jobPostfix << "\" )" );

		// 	TaskRunner::init( _config, _nodePath, _fileList, _jobPostfix );

		// 	initLogger();
		// 	initDataSource( _fileList );

		// 	DEBUG( classname(), "Common Initialization" );
		// 	initialize();
		// }



	void SharedTreeAnalyzer::initDataSource( string _fileList ){
		INFO( classname(), "( fileList=" << _fileList << ")" );
		
		chain = new TChain( this->config.getString( nodePath + ".input.dst:treeName" ).c_str() );

		if ( config.exists( config.join( nodePath, "input", "dst" ) ) ) {
	    	// single job
		    if ( "" == _fileList ){
		    	INFO( classname(), " Loading data from " << config.getString( nodePath + ".input.dst:url" ) );

		    	ChainLoader::load( 	chain, 
		    						this->config.getString( nodePath + ".input.dst:url" ), 
		    						this->config.getInt( nodePath + ".input.dst:maxFiles", -1 ) );
		    } else { // or parallel
			    	// star-submit style condor files
		    	INFO( classname(), " Parallel Job From " << _fileList );

				ChainLoader::loadList( 	chain, 
										_fileList, 
										this->config.getInt( nodePath + ".input.dst:maxFiles", -1 ) );	
		    }
	    } else {
	    	chain = nullptr;
	    	ERROR( classname(), "No Chain was created" );
	    	return;
	    }
	}

	void SharedTreeAnalyzer::initDataSource( int _jobIndex ){
	    INFO( classname(), "( jobIndex=" << _jobIndex << ")" );


	    treeName = this->config.getString( nodePath + ".input.dst:treeName" );
	    chain = new TChain( this->config.getString( nodePath + ".input.dst:treeName" ).c_str() );

	    if ( config.exists( config.join( nodePath, "input", "dst" ) ) ) {
	    	// create the Chain object
		    string url = this->config.getString( nodePath + ".input.dst:url" );

		    // load from a file list!
		    if ( url.find( ".lis" ) != std::string::npos ){
		    	
		    	INFO( classname(), " Loading data from listfile : " << url );
		    	// parallel job - load from list range
		    	if ( _jobIndex >= 0 ){

		    		int splitBy = config.getInt( nodePath + ".input.dst:splitBy", 50 );
		    		INFO( classname(), " Parallel Job index=" << _jobIndex << " splitBy=" << splitBy );
		    		ChainLoader::loadListRange( chain, url, _jobIndex, splitBy );


		    	} else { // load the entire list (up to maxFiles )

		    		int maxFiles = this->config.getInt( nodePath + ".input.dst:maxFiles", -1 );
		    		ChainLoader::loadList( 	chain, url, maxFiles );

		    	}

		    } else { // NOT a list file - a directory or file
				INFO( classname(), " Loading data from file or dir: " << url );
				int maxFiles = this->config.getInt( nodePath + ".input.dst:maxFiles", -1 );
				ChainLoader::load( 	chain, url, maxFiles ); 
		    }	
	    } else {
	    	chain = nullptr;
	    	ERROR( classname(), "No Chain was created" );
	    	return;
	    }

		INFO( classname(), "Data source initialized" );
	}


	void SharedTreeAnalyzer::initLogger(){
		Logger::setGlobalLogLevel( config.getString( nodePath + ".Logger:globalLogLevel" ) );
	}


	void SharedTreeAnalyzer::makeTreeAnalyzers(){
		vector<string> paths = config.childrenOf( nodePath + ".Tasks", "Task" );
		INFO( classname(), "Found " << paths.size() << plural( paths.size(), " task", " tasks" ) );

		for ( string p : paths ){

			string _name = config[ p + ":name" ];
			string _type = config[ p + ":type" ];
			string _configFile = config[ p + ":config" ];
			string _nodePath = config[ p + ":nodePath" ];

			XmlConfig _taskConfig;

			INFO( classname(), "Running " << _type << " " << _name );
			if ( "" == _configFile ){
				_taskConfig = config;
			} else if ( fileExists( _configFile ) ){
				// INFO( classname(), "Using " << _configFile << " config for task" );
				_taskConfig.loadFile( _configFile );
				// _taskConfig.applyOverrides( cmdLineConfig );
			} else {
				ERROR( classname(), "Cannot load config " << _configFile );
				continue;
			}

			// we have a config file now;
			INFO( classname(), "Using " << _taskConfig.getFilename() << " config for task" );
			
			shared_ptr<TreeAnalyzer> treeAnalyzer = shared_ptr<TreeAnalyzer>( static_cast<TreeAnalyzer*>(TaskFactory::createTaskRunner( _type )) );
			if ( nullptr == treeAnalyzer ){
				ERROR( classname(), "Cannot create " << _type << ". Maybe you need to register it?" );
				ERROR( classname(), "TaskFactory::registerTaskRunner<" << _type << ">( \""<< _type <<"\" );" );
				continue;
			}

			treeAnalyzers.push_back( treeAnalyzer );

			treeAnalyzer->sharedInit( _taskConfig, _nodePath, chain );

		}// loop on task paths
	}


	void SharedTreeAnalyzer::make(){
		DEBUG( classname(), "" );
		TaskTimer t;
		t.start();

		if ( !chain ){
			ERROR( classname(), "Invalid chain object" );
			return;
		}
		if ( chain->GetNtrees() <= 0 ){
			INFO( classname(), "Empty Chain" );
			return;
		}

		makeTreeAnalyzers();
		
		int totalNumberOfEvents = -1;
		showProgress = config.getBool( nodePath + ".EventLoop:progress", false );

		if ( showProgress ){
			totalNumberOfEvents = chain->GetEntries();
		}

		nEventsToProcess = config.getInt( nodePath + ".input.dst:nEvents", totalNumberOfEvents );

		// if neg then process all
		if ( nEventsToProcess < 0 || nEventsToProcess > totalNumberOfEvents )
			nEventsToProcess = totalNumberOfEvents;

		if ( showProgress ){
			INFO( classname(), "Loaded: " << nEventsToProcess << " events " );
		}

		/**
		 * Run the pre event-loop hook
		 */
		preEventLoop();

		/**
		 * Run the event-loop
		 */
		eventLoop();

		/**
		 * Run the post event-loop hook
		 */
		postEventLoop();
		INFO( classname(), "Make completed in " << t.elapsed() );
	} // eventLoop


	void SharedTreeAnalyzer::eventLoop( ){
		DEBUG( classname(), "EventLoop" );

		TaskTimer t;
		t.start();

		TaskProgress tp( "Event Loop", nEventsToProcess );
		
		// loop over all events 
		Long64_t iEvent = 0;
		
		while (true){ // what could go wrong 
			Long64_t read = chain->GetEntry(iEvent);
			
			if ( read <= 0 || (nEventsToProcess >= 0 && iEvent >= nEventsToProcess) ){ // break if we read past end or hit limit
				break;
			}

			if ( showProgress )
				tp.showProgress( iEvent );

			analyzeEventBeforeCuts();

			if ( !keepEvent() ){
				analyzeRejectedEvent();
				iEvent++;
				continue;
			}

			analyzeEvent();
			iEvent++;
		} // Event Loop
		
		if ( false == showProgress ){
			INFO( classname(), "Event Loop Completed in " << t.elapsed() );
		}
	}

	bool SharedTreeAnalyzer::keepEvent(){
		bool passAny = false;
		for ( shared_ptr<TreeAnalyzer> ta : treeAnalyzers ){
			if (ta->keepEvent())
				passAny = true;
		}
		return passAny;
	}

	void SharedTreeAnalyzer::preEventLoop(){
		DEBUG( classname(), "" );
		for ( shared_ptr<TreeAnalyzer> ta : treeAnalyzers ){
			ta->preEventLoop();
		}
	}

	void SharedTreeAnalyzer::postEventLoop(){
		DEBUG( classname(), "" );
		for ( shared_ptr<TreeAnalyzer> ta : treeAnalyzers ){
			ta->postEventLoop();
		}
	}

	void SharedTreeAnalyzer::analyzeEvent(){
		DEBUG( classname(), "" );
		for ( shared_ptr<TreeAnalyzer> ta : treeAnalyzers ){
			ta->analyzeEvent();
		}
	}

	void SharedTreeAnalyzer::analyzeRejectedEvent(){
		DEBUG( classname(), "" );
		for ( shared_ptr<TreeAnalyzer> ta : treeAnalyzers ){
			ta->analyzeRejectedEvent();
		}
	}

	void SharedTreeAnalyzer::analyzeEventBeforeCuts(){
		DEBUG( classname(), "" );
		for ( shared_ptr<TreeAnalyzer> ta : treeAnalyzers ){
			ta->analyzeEventBeforeCuts();
		}
	}


}