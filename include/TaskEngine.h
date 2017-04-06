#ifndef TASK_ENGINE_H
#define TASK_ENGINE_H

// RooBarb
#include "Logger.h"
#include "XmlConfig.h"
#include "Utils.h"

#include "TaskFactory.h"
#include "TaskRunner.h"
	// Interfaces
	#include "IConfig.h"
	#include "IObject.h"

// STL
#include <exception>
#include <sys/stat.h>


namespace jdb{

	class TaskEngine : public IObject, public IConfig
	{
	public:
		virtual const char * classname() const { return "TaskEngine"; }
		
		TaskEngine( int argc, char *argv[], string defaultType = "" );
		~TaskEngine() {}

		inline bool fileExists (const std::string& name) {
			struct stat buffer;   
			return (stat (name.c_str(), &buffer) == 0); 
		}

		void setDefaultTask( string defaultType ){
			_defaultType = defaultType;
		}
		void runTasks(  );

	protected:
		int jobIndex;
		string _defaultType;

		map<string, string> cmdLineConfig;
		void getCmdLineConfigOverrides( int argc, char * argv[] );
		void getOverrideFromString( string s, string &path, string &value );

		// only needed so thatI cna be backwards compatibile with my old way
		// should be called after getCmdLineConfigOverrides
		void setJobIndex(int argc, char * argv[]);


		std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
			std::stringstream ss(s);
			std::string item;
			while (std::getline(ss, item, delim)) {
				elems.push_back(item);
			}
			return elems;
		}


		std::vector<std::string> split(const std::string &s, char delim) {
			std::vector<std::string> elems;
			split(s, delim, elems);
			return elems;
		}
		


#ifdef __CINT__
		ClassDef( jdb::TaskEngine, 1)
#endif
		
	};


}


#endif