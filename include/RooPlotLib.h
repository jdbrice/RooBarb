//
// Created by James Brandenburg on 6/4/15.
//

#ifndef ROOBARB_ROOPLOTLIB_H
#define ROOBARB_ROOPLOTLIB_H

// ROOBARB
#include "Utils.h"
#include "XmlConfig.h"
using namespace jdb;

// STL
#include <string>
#include <initializer_list>
using namespace std;


// ROOT
#include "TObject.h"
#include "TH1.h"
#include "TGraph.h"
#include "TF1.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TAttFill.h"

// TODO: Add @UnitTest for RooPlotLib (1)
// TODO: Add @documentation for RooPlotLib (0)

#include "IObject.h"

namespace jdb{

	class HistoBook;

	class RooPlotLib : public IObject {

	protected:

		//string type = "";
		TObject * styling = nullptr;
		string drawOption = "";
		bool drawNorm     = false;
		bool drawClone    = false;

		bool validObject( TObject * obj );

		XmlConfig * cfgForDefaultStyle = nullptr;
		string defaultStylePath ="";

		shared_ptr<HistoBook> linkedBook;

		const float fontScale = 360.0f;

		void style_axes( string opt, vector<string> params );


	public:

		virtual const char* classname() const { return "RooPlotLib"; }
		RooPlotLib ();
		~RooPlotLib();

		RooPlotLib &defaultStyle( XmlConfig * _cfg, string stylePath ){
			cfgForDefaultStyle = _cfg;
			defaultStylePath = stylePath;
			return *this;
		}


		RooPlotLib &style( TObject * );
		RooPlotLib &style( string name );

		void link( shared_ptr<HistoBook> book );

		
		RooPlotLib &set( string opt, float p0 = -999, float p1 = -999, float p2 = -999, float p3 = -999 ){
			return set( opt, { dts(p0), dts(p1), dts(p2), dts(p3) } );
		}
		RooPlotLib &set( string, initializer_list<string> l );
		RooPlotLib &set( string, vector<string> l );
		RooPlotLib &set( string opt, string p0 ="", string p1 ="", string p2 ="", string p3 =""){
			return set( opt, { p0, p1, p2, p3 } );
		}
		RooPlotLib &set( XmlConfig * cfg, string np );
		RooPlotLib &set( XmlConfig &cfg, string np );

		/**
		 * Draws the object being styled
		 * @return 		reference to RooPlotLib object for method chaining
		 */
		RooPlotLib &draw();
		RooPlotLib &draw( string appendDrawOpt );

		static string normalizeAttribute( string str );
		string normalize( string sin );

		int color( string color );
#ifdef __CINT__
		ClassDef( jdb::RooPlotLib, 1 )
#endif
	};
}

#endif //ROOBARB_ROOPLOTLIB_H
