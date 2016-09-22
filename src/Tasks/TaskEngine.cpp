#include "TaskEngine.h"

TaskEngine::TaskEngine( int argc, char *argv[] ){

	try {

		getCmdLineConfigOverrides( argc, argv );
		// log everything for the moment
		Logger::setGlobalLogLevel( "all" );


		if ( argc >= 2 ){
			
			// sets the jobIndex either from positional arg
			// or if --jobIndex=N is given use that (dont the positional arg)
			setJobIndex( argc, argv );
			INFO( classname(), "Job Index = " << jobIndex );


			// create the config file for understanding jobs
			INFO( classname(), "Loading TaskEngine config : " << argv[1] );
			if ( !fileExists( argv[1] ) ){
				ERROR( classname(), "Cannot load " << argv[1] << ": File DNE" );
			} else {
				config.loadFile( argv[1] );
				config.applyOverrides( cmdLineConfig );

				Logger::setGlobalLogLevel( config.getString( "Logger:globalLogLevel" ) );

				runTasks();
			}
		} // if argv >= 2
	} catch ( exception &e ) {

	}

}

void TaskEngine::runTasks(){

	vector<string> paths = config.childrenOf( "", "Task", 1 );
	INFO( classname(), "Found " << paths.size() << plural( paths.size(), " task", " tasks" ) );

	for ( string p : paths ){

		string _name = config[ p + ":name" ];
		string _type = config[ p + ":type" ];
		string _configFile = config[ p + ":config" ];
		string _nodePath = config[ p + ":nodePath" ];

		XmlConfig _taskConfig;

		INFO( classname(), "Running " << _type << " " << _name );
		if ( "" == _configFile ){
			INFO( classname(), "Using main config" );
			_taskConfig = config;
		} else if ( fileExists( _configFile ) ){
			INFO( classname(), "Using " << _configFile << " config for task" );
			_taskConfig.loadFile( _configFile );
			_taskConfig.applyOverrides( cmdLineConfig );
		} else {
			ERROR( classname(), "Cannot load config " << _configFile );
			continue;
		}

		// we have a config file now;
		INFO( classname(), "Using " << _taskConfig.getFilename() << " config for task" );
		
		TaskRunner * taskRunner = TaskFactory::createTaskRunner( _type );
		if ( nullptr == taskRunner ){
			ERROR( classname(), "Cannot create " << _type << ". Maybe you need to register it?" );
			ERROR( classname(), "TaskFactory::registerTaskRunner<" << _type << ">( \""<< _type <<"\" );" );
			continue;
		}

		taskRunner->init( _taskConfig, _nodePath );
		taskRunner->run();

		delete taskRunner;

	}// loop on task paths
}// runTasks

void TaskEngine::getCmdLineConfigOverrides( int argc, char * argv[] ){
	
	for ( int i = 0; i < argc; i++ ){
		string v = argv[ i ];
		if ( 0 == v.compare( 0, 2, "--" ) ){
			
			vector<string> parts = split( v, '=' );
			if ( parts.size() <= 1 ) continue;

			string path = parts[0].substr( 2 );
			string value = parts[1];

			cmdLineConfig[ path ] = value;
		} // check that it starts with "--"
	} // loop over argc
} // getCmdLineConfigOverrides

void TaskEngine::setJobIndex(int argc, char * argv[]){

	if ( cmdLineConfig.count( "jobIndex" ) >= 1 ){
		jobIndex = atoi( cmdLineConfig[ "jobIndex" ].c_str() );
	} else if ( argc >= 3 && cmdLineConfig.size() == 0 ){
		jobIndex = atoi( argv[2] );
	} else {
		jobIndex = -1; // not parallel job
	}

	// amke sure that the jobIndex is set in the in-memory configs
	cmdLineConfig[ "jobIndex" ] = ts( jobIndex );
} // setJobIndex