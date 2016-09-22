#ifndef ITASK_RUNNER_H
#define ITASK_RUNNER_H


// #include "XmlConfig.h"

#include <string>
using namespace std;

namespace jdb {

	class XmlConfig;

	class ITaskRunner {
	public:
		virtual ~ITaskRunner() {}
		virtual void init( XmlConfig &_config, string _nodePath ) = 0;
		// virtual void init( XmlConfig &_config, string _nodePath="", int _jobIndex = -1 ) = 0;
		// virtual void init( XmlConfig &_config, string _nodePath="", string _fileList="", string _jobPostfix = "" ) = 0;
		virtual void run() = 0;

	protected:
		
		virtual void preMake() = 0;
		virtual void make() = 0;
		virtual void postMake() = 0;


	};
}
#endif