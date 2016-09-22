#ifndef SHARED_TREE_ANALYZER_H
#define SHARED_TREE_ANALYZER_H


//STL
#include <memory>
#include <exception>
#include <sys/stat.h>


//ROOBARB
#include "Utils.h"
#include "HistoBook.h"
#include "Reporter.h"
#include "DataSource.h"
#include "TaskRunner.h"
#include "ChainLoader.h"
#include "Utils.h"
#include "TreeAnalyzer.h"


//ROOT
#include "TChain.h"
#include "TError.h"


namespace jdb {
	class SharedTreeAnalyzer : public TaskRunner
	{
	protected:
		//The chain object which gets automatically set up
		TChain 		*chain = nullptr;
		string 		treeName;
		
		// the maximum number of events to process. If not set all events will be processed 
		int nEventsToProcess;

		// show progres via TaskProgress
		bool showProgress = false;

		vector<shared_ptr<TreeAnalyzer> > treeAnalyzers;

		int jobIndex;

	public:
		/* @inherit */
		virtual const char* classname() const { return "SharedTreeAnalyzer";}

		SharedTreeAnalyzer();
		~SharedTreeAnalyzer();

		virtual void init( XmlConfig &_config, string _nodePath );
		// virtual void init( XmlConfig &_config, string _nodePath="", int _jobIndex = -1);
		// virtual void init( XmlConfig &_config, string _nodePath="", string _fileList = "", string _jobPostfix = "" );
		virtual void initialize() { DEBUG( classname(), "TreeAnalyzer" ); };


	protected:
		void initDataSource( int _jobIndex );
		void initDataSource( string _fileList );
		void initLogger( );


		void makeTreeAnalyzers();


		/*The maker function for publicly starting the job
		 *
		 */
		virtual void make();

		/* Called before event loop
		 *	Used for setup + initialization before 
		 *	processing the entire loop
		 * 
		 */
		virtual void preEventLoop();

		/* The logic for an event loop
		 * Used for overriding just the tree reading and event loop logic
		 */
		virtual void eventLoop( );

		/*
		 * Called after the event loop for cleanup, 
		 * report generation, etc."
		 */
		virtual void postEventLoop();

		/* Used to analyze events
		 * Analyzes a single event in the chain
		 */
		virtual void analyzeEvent();
		virtual void analyzeEventBeforeCuts();
		virtual void analyzeRejectedEvent();
		virtual bool keepEvent();
		

		inline bool fileExists (const std::string& name) {
			struct stat buffer;   
			return (stat (name.c_str(), &buffer) == 0); 
		}
	};	
}



#endif