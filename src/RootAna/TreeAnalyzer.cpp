#include "TreeAnalyzer.h"
#include "ChainLoader.h"

namespace jdb{

	TreeAnalyzer::TreeAnalyzer(){
		DEBUG( classname(), "" );
	}

	TreeAnalyzer::~TreeAnalyzer(){
		DEBUG( classname(), "" );
		// if ( book )
		// 	delete book;
		// DEBUG( classname(), "Deleted book" );
		if ( reporter )
			delete reporter;
		DEBUG( classname(), "Deleted Reporter" );
	}


	void TreeAnalyzer::init( XmlConfig &_config, string _nodePath ){
		DEBUG( classname(), "( " << _config.getFilename() << ", " << _nodePath << " )" );

		TaskRunner::init( _config, _nodePath );

		int _jobIndex = config.getInt( "jobIndex", -1 );
		DEBUG( classname(), "jobIndex = " << _jobIndex );
		string jobPostfix = "_" + ts( _jobIndex ) + ".root";
		if ( -1 >= jobIndex )
			jobPostfix = ".root";

		this->jobModifier = "job_" + ts( _jobIndex ) +"_";
		if ( -1 == _jobIndex ){
			jobPostfix = ".root";
			this->jobModifier = "";
		}

		initDataSource( );
		if ( chain ){
			initHistoBook( jobPostfix );
			initReporter( jobPostfix );
			initLogger();	
		}
		nEventLoops = config.getInt( nodePath + ":nEventLoops", 1 );
		DEBUG( classname(), "Common Initialization" );
		initialize();
	}

	void TreeAnalyzer::sharedInit( XmlConfig &_config, string _nodePath, TChain * _chain ){
		DEBUG( classname(), "( " << _config.getFilename() << ", " << _nodePath << ", chain=" << _chain << " )" );

		TaskRunner::init( _config, _nodePath );

		int _jobIndex = config.getInt( "jobIndex", -1 );
		string jobPostfix = "_" + ts( _jobIndex ) + ".root";
		if ( -1 >= _jobIndex )
			jobPostfix = ".root";

		this->jobModifier = "job_" + ts( _jobIndex ) +"_";
		if ( -1 == _jobIndex ){
			jobPostfix = ".root";
			this->jobModifier = "";
		}

	
		this->chain = _chain;
		sharedTree = true;
		if ( chain ){
			initHistoBook( jobPostfix );
			initReporter( jobPostfix );
			initLogger();	
		}
		nEventLoops = config.getInt( nodePath + ":nEventLoops", 1 );
		DEBUG( classname(), "Common Initialization" );
		initialize();
	}

	void TreeAnalyzer::initHistoBook( string _jobPostfix ) {
		initializeHistoBook( config, nodePath, _jobPostfix );
	}

	void TreeAnalyzer::initReporter( string _jobPostfix ){

		INFO( classname(), "Creating Reporter" );
		string pRepOut = config.join( nodePath, "Reporter", "output:url" );
		string outputURL = config[ pRepOut ];

		// Default reporter
		if ( ".root" == _jobPostfix && config.exists( pRepOut ) ) {
			reporter = new Reporter( config, config.join( nodePath, "Reporter" ), "" ); // TODO: is reporter's path handeling broken?
			INFO( classname(), "Creating report @" << outputURL );
		} else{
			INFO( classname(), "No Reporter created, jobPostfix == " << _jobPostfix );
			reporter = nullptr;
		}
	}

	void TreeAnalyzer::initDataSource( ){
		
		int _jobIndex = config.getInt( "jobIndex", -1 );

		if ( !config.exists( config.join( nodePath, "input", "dst" ) ) ){
			treeName = "";
			chain = nullptr;
			ERROR( classname(), "No Chain was created because the <" + nodePath + ".input.dst> could not be found" );
			return;
		}

		// create the Chain object
		treeName   = this->config.getString( nodePath + ".input.dst:treeName" );
		chain      = new TChain( treeName.c_str() );
		string url = this->config.getXString( nodePath + ".input.dst:url" );

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
			if ( _jobIndex != -1 ){
				WARN( classname(), "Range loading not allowed for directory listings" );
			}
			ChainLoader::load( chain, url, maxFiles ); 
		}	


		// make a DataSource if you want it
		if ( config.exists( nodePath + ".DataSource" ) && chain  ){
			ds = new DataSource( config, config.join(nodePath, ".DataSource"), treeName, chain );
			DEBUG( classname(), "DataSource for chain : " << chain );
		}


		INFO( classname(), "Data source initialized" );
	}


	void TreeAnalyzer::initLogger(){
		Logger::setGlobalLogLevel( config.getString( nodePath + ".Logger:globalLogLevel" ) );
	}


	void TreeAnalyzer::make(){
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

		

		/**
		 * Run the pre event loop
		 */
		if ( !skipMake ){
			book->cd( );
			
			

			
			int totalNumberOfEvents = -1;
			showProgress = config.getBool( nodePath + ".EventLoop:progress", false );

			if ( showProgress ){
				totalNumberOfEvents = chain->GetEntries();
			}


			nEventsToProcess = config.getInt( nodePath + ".input.dst:nEvents", totalNumberOfEvents );

			// if neg then process all
			if ( nEventsToProcess < 0 || nEventsToProcess > totalNumberOfEvents )
				nEventsToProcess = totalNumberOfEvents;

			// check for datastore
			if ( ds )
				nEventsToProcess = config.getInt( nodePath + ".DataSource:maxEvents", nEventsToProcess );
			
			if ( showProgress ){
				INFO( classname(), "Loaded: " << nEventsToProcess << " events " );
			}
			
			INFO( classname(), "Starting Event Loops" );
			for ( iEventLoop = 0; iEventLoop < nEventLoops; iEventLoop++ ){
				INFO( classname(), "Starting Event Loop " << iEventLoop+1 << " of " << nEventLoops );
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
			} // end loop on iEventLoop
		} // skipMake
		else {
			postEventLoop();
		}
		INFO( classname(), "Make completed in " << t.elapsed() );
	} // eventLoop


	void TreeAnalyzer::eventLoop( ){
		DEBUG( classname(), "EventLoop" );

		TaskTimer t;
		t.start();

		TaskProgress tp( "Event Loop", nEventsToProcess );
		
		// loop over all events 
		iEvent = 0;
		
		while (true){ // what could go wrong 
			Long64_t read = chain->GetEntry(iEvent);
			
			if ( read <= 0 || (nEventsToProcess >= 0 && iEvent >= nEventsToProcess) ){ // break if we read past end or hit limit
				INFO( classname(), "Ending event loop on iEvent = " << iEvent );
				break;
			}
			if ( finishCurrentEventLoopEarly ){
				finishCurrentEventLoopEarly = false;
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


	void TreeAnalyzer::preEventLoop(){

		// make some histos here
		if ( config.exists( nodePath+".histograms" ) )
			book->makeAll( nodePath+".histograms" );
	}

	/**
	 * Reads data from the chain - cuts events
	 * @return true or false
	 */
	bool TreeAnalyzer::keepEvent() {


		return true;
	}
}