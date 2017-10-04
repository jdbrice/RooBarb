#ifndef JDB_XML_HISTOGRAM_H
#define JDB_XML_HISTOGRAM_H

// ROOBARB
#include "XmlConfig.h"
#include "Utils.h"
#include "IObject.h"
#include "HistoBins.h"
#include "HistoBook.h"

// STL
#include <memory>

// ROOT
#include "TH1.h"
#include "TFile.h"
#include "TAxis.h"


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
				// ERROR( classname(), "Could not load \"" << name << "\" from file:" << url );
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

		virtual string toString(){
			if ( h1 ){
				return "XmlHistogram<name=" + quote(string(h1->GetName())) + ">";
			}
			return "XmlHistogram<nullptr>";
		}

		static string toXml( TAxis * _a, string axis = "X" ){
			string xml = "<BinEdges" + axis + ">";
			string delim = "";
			for ( int i = 1; i <= _a->GetNbins(); i++ ){
				xml += delim + dts( _a->GetBinLowEdge(i) );
				delim = ", ";
			}
			xml += delim + dts( _a->GetBinUpEdge( _a->GetNbins() ) );
			xml += "</BinEdges" + axis + ">";
			return xml;
		}
		static string toXml( TH1 * _h ){
			if ( nullptr == _h )
				return "";
			int nDims = 1;
			if ( _h->GetNbinsY() > 1 ) nDims = 2;
			if ( _h->GetNbinsZ() > 1 ) nDims = 3;

			string xml = "<HistogramData name=" + quote( _h->GetName() ) + " title=" +quote( _h->GetTitle() ) + " dims=" + quote(nDims) + ">";
			if ( nDims >= 1 )
				xml += "\n\t" + XmlHistogram::toXml( _h->GetXaxis(), "X" );
			if ( nDims >= 2 )
				xml += "\n\t" + XmlHistogram::toXml( _h->GetYaxis(), "Y" );
			if ( nDims >= 3 )
				xml += "\n\t" + XmlHistogram::toXml( _h->GetZaxis(), "Z" );

			xml += "\n\t<Values>";
			string delim = "";
			for ( int ix = 1; ix <= _h->GetNbinsX(); ix++ ){
				for ( int iy = 1; iy <= _h->GetNbinsY(); iy++ ){
					for ( int iz = 1; iz <= _h->GetNbinsZ(); iz++ ){
						int globalBin = _h->GetBin( ix, iy, iz );
						xml += delim + dts( _h->GetBinContent( globalBin ) );
						delim = ", ";
					}
				}
			}
			xml += "</Values>";
			
			xml += "\n\t<Errors>";
			delim = "";
			for ( int ix = 1; ix <= _h->GetNbinsX(); ix++ ){
				for ( int iy = 1; iy <= _h->GetNbinsY(); iy++ ){
					for ( int iz = 1; iz <= _h->GetNbinsZ(); iz++ ){
						int globalBin = _h->GetBin( ix, iy, iz );
						xml += delim + dts( _h->GetBinError( globalBin ) );
						delim = ", ";
					}
				}
			}
			xml += "</Errors>";
				

			xml += "\n</HistogramData>";
			return xml;
		}

		static TH1 * fromXml( XmlConfig &config, string path ){
			if ( false == config.exists( path + ":name" ) ||
				 false == config.exists( path + ".BinEdgesX" ) ||
				 false == config.exists( path + ".Values" ) ) 
				return nullptr;
			string name = config.getString( path + ":name" );
			string title = config.getString( path + ":title" );
			int nDims = config.getInt( path + ":dims", 1 );
			HistoBins bx( config, path + ".BinEdgesX" );
			HistoBins by( config, path + ".BinEdgesY" );
			HistoBins bz( config, path + ".BinEdgesZ" );

			cout << "Making " << nDims << "D name=" << name << " title=" << title << endl;
			TH1 * _h = HistoBook::make( "D", name, title, bx, by, bz );

			vector<float> values = config.getFloatVector( path + ".Values" );
			vector<float> errors = config.getFloatVector( path + ".Errors" );
			for ( int ix = 1; ix <= _h->GetNbinsX(); ix++ ){
				for ( int iy = 1; iy <= _h->GetNbinsY(); iy++ ){
					for ( int iz = 1; iz <= _h->GetNbinsZ(); iz++ ){
						int i = _h->GetBin( ix, iy, iz );
						if (  i-1 >= values.size() ) continue;
						_h->SetBinContent( i, values[i-1] );
						if (  i-1 >= errors.size() ) continue;
						_h->SetBinError( i, errors[i-1] );
					}
				}
			}

			return _h;
		}

		
#ifdef __CINT__
		ClassDef( jdb::XmlHistogram, 1 )
#endif
	};

}


#endif