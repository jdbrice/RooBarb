#ifndef I_OUTPUT_MAKER_H
#define I_OUTPUT_MAKER_H

#include "IObject.h"
#include "Logger.h"
#include "XmlConfig.h"
#include "XmlHistoBook.h"


#include "TFile.h"

#include <memory>

namespace jdb{
	class IHistoBookMaker : virtual public IObject
	{
	public:
		virtual const char* classname() const { return "IHistoBookMaker"; }
		IHistoBookMaker() {}
		~IHistoBookMaker() {}

		//Store project histograms and data
		shared_ptr<HistoBook> 	book = nullptr;
		string outputPath = "";

		virtual void initializeHistoBook( XmlConfig &config, string nodePath, string _jobPostfix ){
			// look for histobooks
			// Take the first one
			vector<string> searchPaths = {
				".output.HistoBook",
				".output.TFile"
			};

			for ( string p : searchPaths ){
				if ( !config.exists( nodePath + p ) ) continue;
				book = XmlHistoBook( config, nodePath + p ).getBook();
				if ( nullptr != book )
					break;
			}
			
			// None of the new forms work
			// probably the old output.data using ouput:path etc.
			// DEPRECIATE v4
			if ( nullptr == book ){
				// old school - no string interp
				string jobPrefix = "";  // will we ever use this?
				outputPath = config[ nodePath + ".output:path" ];
				string name = config[ config.join( nodePath, "output", "data" ) ];

				// add in the inline output node
				if ( config.exists( nodePath + ".output:name" ) )
					name = config[ nodePath + ".output:name" ];

				// remove .root from the name if it is in there
				// the jobPostfix will add it back
				// Warning - this assumes that the '.root' is at the end of the string
				string ext = ".root";
				size_t extPos = name.find_last_of( ext );
				DEBUG( classname(), "name = \"" << name << "\"");
				if ( extPos != std::string::npos )
					name = name.substr( 0, extPos - (ext.length() - 1) );
				DEBUG( classname(), "name = \"" << name << "\"");
				string full = outputPath + jobPrefix + name + _jobPostfix;
				// // create the book
				DEBUG( classname(), " Creating book : " << full );
				book = shared_ptr<HistoBook>(new HistoBook( full, config ) );
			}
		}
		
	};
}


#endif