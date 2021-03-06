#ifndef COMMAND_RUNNER_H
#define COMMAND_RUNNER_H

#include <glob.h>
#include <vector>
#include <string>
#include <cstdlib>
using namespace std;

// RooBarb
#include "TaskRunner.h"

namespace jdb {
	class CommandRunner : public TaskRunner {
	public:
		virtual const char* classname() const { return "CommandRunner"; }
		CommandRunner() {}
		~CommandRunner(){}

		virtual void init(XmlConfig &_config, string _nodePath ) {
			TaskRunner::init( _config, _nodePath );
			initialize();
		}

		virtual void initialize(){
			// globStr = config.getXString( nodePath + ":glob" );
		}
	protected:

		virtual void make(){
			DEBUG( classname(), "" );

			INFO( classname(), "COMMAND : " << config.getXString( nodePath + ".command" ) );
			INFO( classname(), "executing" );

			vector<string> cmds = config.childrenOf( nodePath, "command" );
			INFOC( "Found " << cmds.size() << plural( cmds.size(), " command", " commands" ) );

			for ( string pcmd : cmds ){
				INFOC( "Executing: " << quote( config.getXString( pcmd ) ) );
				// do it here
				system( config.getXString( pcmd ).c_str() );
			}
			

			INFO( classname(), "Completed" );
		}

	};
}



#endif