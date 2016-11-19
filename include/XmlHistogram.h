#ifndef JDB_XML_HISTOGRAM_H
#define JDB_XML_HISTOGRAM_H

// ROOBARB
#include "XmlConfig.h"
#include "Utils.h"
#include "IObject.h"

// STL
#include <memory>

// ROOT
#include "TH1.h"
#include "TFile.h"


namespace jdb{

	/* Loads a TF1 or subclass function from an Xml Config
	 *
	 * Loads a function from an XmlConfig.
	 */
	class XmlHistogram : public IObject { 
	protected:
		// The ROOT TH1 object 
		shared_ptr<TH1> h1 = nullptr;
	public:
		virtual const char * classname() const { return "XmlHistogram"; }

		/* Creates a function from xml config
		 *
		 * @cfg 		XmlConfig object
		 * @nodePath 	path to node
		 * @pMod 		Default : ':p' - the modifier to get the parameter from node path
		 * @eMod 		Default : ':e' - the modifier to get the parameter error from node path
		 * @formMod 	Default : ':formula' - the modifier to get the formula from node path
		 */
		XmlHistogram( XmlConfig  &_cfg, string _nodePath, string uMod=":url", string nMod = ":name"){
			load( _cfg, _nodePath, uMod, nMod );
		}
		XmlHistogram() {}

		void load( XmlConfig &_cfg, string _nodePath, string uMod=":url", string nMod = ":name" ){

			string url = _cfg.getXString( _nodePath + uMod );
			string name = _cfg.getXString( _nodePath + nMod );


			// open the TFile
			TFile * f = new TFile( url.c_str(), "READ" );
			
			h1 = shared_ptr<TH1>( (TH1*)f->Get( name.c_str() ) );
			if ( nullptr == h1 ){
				ERROR( classname(), "Could not load \"" << name << "\" from file:" << url );
				return;
			}

			// detach the histogram from the directory
			h1->SetDirectory(0);

			f->Close();
		}
		

		/* Destructor
		 *
		 */
		~XmlHistogram() {}

		shared_ptr<TH1> getTH1() { return h1; }

		
#ifdef __CINT__
		ClassDef( jdb::XmlHistogram, 1 )
#endif
	};

}


#endif