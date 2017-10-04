#include "TaskEngine.h"

TaskEngine::TaskEngine( int argc, char *argv[], string defaultType ){

	setDefaultTask( defaultType );

	try {

		getCmdLineConfigOverrides( argc, argv );
		// log everything for the moment
		Logger::setGlobalLogLevel( "All" );


		if ( argc >= 2 ){
			
			// sets the jobIndex either from positional arg
			// or if --jobIndex=N is given use that (dont the positional arg)
			setJobIndex( argc, argv );
			INFO( classname(), "Job Index = " << jobIndex );


			// create the config file for understanding jobs
			INFO( classname(), "Loading TaskEngine config : " << argv[1] );
			if ( !fileExists( argv[1] ) ){
				ERROR( classname(), "Cannot load " << argv[1] << ": File DNE" );
			} else if ( string(argv[1]).find(".xml") == string::npos ){
				ERROR( classname(), "Not an XML file" );
			} else {
				// passing as second arg here loads the overrides BEFORE evaluating include URLS
				config.loadFile( argv[1], cmdLineConfig );

				Logger::setGlobalLogLevel( config.getString( "Logger:globalLogLevel" ) );
				Logger::setGlobalColor( config.getBool( "Logger:color", false ) );

				runTasks();
			}
		} // if argv >= 2
	} catch ( exception &e ) {

	}

}

void TaskEngine::runTasks( ){

	vector<string> paths = config.childrenOf( "", "Task", 1 );
	INFO( classname(), "Found " << paths.size() << plural( paths.size(), " task", " tasks" ) );

	for ( string p : paths ){

		string _name       = config[ p + ":name" ];
		string _type       = config[ p + ":type" ];
		string _configFile = config[ p + ":config" ];
		string _nodePath   = config[ p + ":nodePath" ];
		vector<string> _extraArgs  = config.getStringVector( p + ":args" );


		XmlConfig _taskConfig;

		INFO( classname(), "Running " << _type << " " << _name );
		if ( "" == _configFile ){
			INFO( classname(), "Using main config" );
			_taskConfig = config;
		} else if ( fileExists( _configFile ) ){
			INFO( classname(), "Using " << _configFile << " config for task" );
			// passing as second arg here loads the overrides BEFORE evaluating include URLS
			_taskConfig.loadFile( _configFile, cmdLineConfig );
			// _taskConfig.applyOverrides( cmdLineConfig );
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

		INFO( classname(), "Task :" << _type << " " << _name << ANSIColors::color( " Complete", "green" ) );

	}// loop on task paths

	//  Check for default Task to run
	if ( paths.size() <= 0 && "" != _defaultType ){
		TaskRunner * taskRunner = TaskFactory::createTaskRunner( _defaultType );
		if ( nullptr == taskRunner ){
			ERROR( classname(), "Cannot create default Task of " << _defaultType << ". Maybe you need to register it?" );
			ERROR( classname(), "TaskFactory::registerTaskRunner<" << _defaultType << ">( \""<< _defaultType <<"\" );" );
			return;
		}
		config.applyOverrides( cmdLineConfig );
		taskRunner->init( config, "" );
		taskRunner->run();

		delete taskRunner;
	}


}// runTasks

void TaskEngine::getOverrideFromString( string s, string &path, string &value, int iarg ){
	if ( 0 == s.compare( 0, 2, "--" ) ){
		vector<string> parts = split( s, '=' );
		if ( parts.size() <= 1 ) return;

		path = parts[0].substr( 2 );
		value = parts[1];
	}
	
	return;
}

void TaskEngine::getCmdLineConfigOverrides( int argc, char * argv[] ){
	
	for ( int i = 0; i < argc; i++ ){
		string v = argv[ i ];
		string path = "";
		string value = "";
		
		getOverrideFromString( v, path, value, i );
		if ( path != "" )
			cmdLineConfig[ path ] = value;
		
		cmdLineConfig[ "argc" ] = ts(argc);
		cmdLineConfig[ "arg[" + ts(i) + "]" ] = argv[i];


		// if ( 0 == v.compare( 0, 2, "--" ) ){
			
		// 	vector<string> parts = split( v, '=' );
		// 	if ( parts.size() <= 1 ) continue;

		// 	string path = parts[0].substr( 2 );
		// 	string value = parts[1];

		// 	cmdLineConfig[ path ] = value;
		// } // check that it starts with "--"
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