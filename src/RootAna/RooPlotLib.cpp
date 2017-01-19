#include "RooPlotLib.h"
#include "HistoBook.h"

//#include "TROOT.h" 
#include "TStyle.h"

#ifdef __CINT__
ClassImp( jdb::RooPlotLib );
#endif
/*
 * Default Ctor
 * 
 */
jdb::RooPlotLib::RooPlotLib()  {
    DEBUG(classname(), "");
	// create a default canvas for generic plotting
	//canvas[ "default" ] = shared_ptr<XmlCanvas>( new XmlCanvas( nullptr, "" ) );


}

/*
 * Dtor
 */
jdb::RooPlotLib::~RooPlotLib(){
    DEBUG(classname(), "")
}

bool jdb::RooPlotLib::validObject( TObject * obj ){
    DEBUG(classname(), "( " << obj << " )")
	if ( nullptr == obj  )
		return false;

	if ( obj->InheritsFrom( "TH1" ) || obj->InheritsFrom( "TGraph" ) )
		return true;
	return false;
}

void jdb::RooPlotLib::link( shared_ptr<HistoBook> book ){
	this->linkedBook = book;
}

RooPlotLib & jdb::RooPlotLib::style( TObject * obj) {
    DEBUG(classname(), "( " << obj << " )")
    styling = obj;

    if ( cfgForDefaultStyle != nullptr ){ 
    	DEBUG(classname(),  "Setting Default Style at : " << defaultStylePath )
    	set( cfgForDefaultStyle, defaultStylePath );
    }
    

	return *this;
}
RooPlotLib & jdb::RooPlotLib::style( string name ) {
	if ( nullptr != this->linkedBook ){
		styling = this->linkedBook->get( name );
	} else {
		ERROR( classname(), "No HistoBook available for lookup" );
	}
	return *this;
}


RooPlotLib &jdb::RooPlotLib::set( string option, initializer_list<string> l ){
    DEBUG(classname(),  "( " << option <<", initializer_list " << " )" );
	vector<string> params( l.begin(), l.end() );
	return set( option, params );
}

/* sets visual styles 
 * @option 	the option to set
 * @l 		an initializer_list of parameters
 *
 * implements all of the options that expect a number type so that it can handle
 * styles loaded from configs etc.
 */
RooPlotLib &jdb::RooPlotLib::set( string option, vector<string> params ){
	DEBUG(classname(), "( "<< option << " )")
	if ( nullptr == styling ){
		ERROR( classname(), "Invalid object" );
		return *this;
	}

	// force the param name to lowercase
	transform(option.begin(), option.end(), option.begin(), ::tolower);

	// commonly used so go ahead and get them
	// int ip0 = atoi( params[0].c_str() );
	// int ip1 = atoi( params[1].c_str() );

	// if param gives a valid color from a string then keep it for later
	int colorFromString = color( params[ 0 ] );

	TH1 * h 	= dynamic_cast<TH1*>(styling);
	TGraph * g 	= dynamic_cast<TGraph*>(styling);
	TF1 * fn 	= dynamic_cast<TF1*>(styling);


	// get the X axis
	TAxis * ax = nullptr;
	if ( nullptr != h ) ax = h->GetXaxis();
	if ( nullptr != g ) ax = g->GetXaxis();
	if ( nullptr != fn ) ax = fn->GetXaxis();
	// get the y axis
	TAxis * ay = nullptr;
	if ( nullptr != h ) ay = h->GetYaxis();
	if ( nullptr != g ) ay = g->GetYaxis();
	if ( nullptr != fn ) ax = fn->GetYaxis();

	// get the z axis (TH3 only)
	TAxis * az = nullptr;
	if ( nullptr != h ) az = h->GetZaxis();

	// set some general properties
	if ( "title" == option || "t" == option ){
		if ( nullptr != h ) h->SetTitle( params[0].c_str() );
		if ( nullptr != g ) g->SetTitle( params[0].c_str() );
		if ( nullptr != fn ) fn->SetTitle( params[0].c_str() );
	}
	// TODO: not really working
	if ( "titlesize" == option || "ts" == option ){
		INFO( classname(), option << " " << params[0] );
		gStyle->SetTitleFontSize( atof(params[0].c_str()) );
		// if ( nullptr != h ) h->SetTitleSize( atof(params[0].c_str()) );
		//if ( nullptr != g ) g->SetTitleSize( params[0].c_str() ); These don't exist?
		//if ( nullptr != fn ) fn->SetTitleSize( params[0].c_str() );
	}
	if ( "draw" == option ){
		drawOption = params[ 0 ];
	}

	// Axis Stuff
	// X-Axis
	if ( nullptr != ax ){
		if ( ("x" == option || "xtitle" == option) ){
			ax->SetTitle( params[ 0 ].c_str() );
		}
		else if ( ("xto" == option || "xtitleoffset" == option ) ){
			ax->SetTitleOffset( atof(params[ 0 ].c_str()) );
		}
		else if ( ("xts" == option || "xtitlesize" == option ) ){
			ax->SetTitleSize( atof(params[ 0 ].c_str()) );
		}
		// Label
		else if ( ("xlo" == option || "xlabeloffset" == option )  ){
			ax->SetLabelOffset( atof(params[ 0 ].c_str()) );
		}
		else if ( ("xls" == option || "xlabelsize" == option ) ){
			ax->SetLabelSize( atof(params[ 0 ].c_str()) );
		}
	

		// Range
		if ( ("xrange" == option || "xr" == option )  ){
			ax->SetRangeUser( atof(params[ 0 ].c_str()), atof(params[ 1 ].c_str()) );
		}
		if ( ("xbinrange" == option || "xbr" == option )  ){
			ax->SetRange( atof(params[ 0 ].c_str()), atof(params[ 1 ].c_str()) );
		}

		// n ticks
		else if (  "xticks" == option || "xtick" == option ){
			int n1 = atoi(params[ 0 ].c_str());
			int n2 = 12;
			int n3 = 0;
			bool opt = true;
			if ( params.size() >= 2 )
				n2 = atoi(params[ 1 ].c_str());
			if ( params.size() >= 3 )
				n3 = atoi(params[ 2 ].c_str());
			if ( params.size() >= 3 )
				opt = (bool) atoi(params[ 3 ].c_str());
			
			ax->SetNdivisions( n1, n2, n3, opt );
		}
	}
	// Y-Axis
	if ( nullptr != ay ){
		// Titl1
		if ( ("y" == option || "ytitle" == option )  ){
			ay->SetTitle( params[ 0 ].c_str() );
		} else if ( ("yto" == option || "ytitleoffset" == option )  ){
			ay->SetTitleOffset( atof(params[ 0 ].c_str()) );
		}
		else if ( ("yts" == option || "ytitlesize" == option ) ){
			ay->SetTitleSize( atof(params[ 0 ].c_str()) );
		}
		// Label
		else if ( ("ylo" == option || "ylabeloffset" == option )  ){
			ay->SetLabelOffset( atof(params[ 0 ].c_str()) );
		}
		else if ( ("yls" == option || "ylabelsize" == option ) ){
			ay->SetLabelSize( atof(params[ 0 ].c_str()) );
		}	

		// Range
		else if ( ("yrange" == option || "yr" == option )  ){
			ay->SetRangeUser( atof(params[ 0 ].c_str()), atof(params[ 1 ].c_str()) );
		}
		else if ( ("ybinrange" == option || "ybr" == option )  ){
			ay->SetRange( atof(params[ 0 ].c_str()), atof(params[ 1 ].c_str()) );
		}

		// n ticks
		else if (  "yticks" == option || "ytick" == option ){
			int n1 = atoi(params[ 0 ].c_str());
			int n2 = 12;
			int n3 = 0;
			bool opt = true;
			if ( params.size() >= 2 )
				n2 = atoi(params[ 1 ].c_str());
			if ( params.size() >= 3 )
				n3 = atoi(params[ 2 ].c_str());
			if ( params.size() >= 3 )
				opt = (bool) atoi(params[ 3 ].c_str());
			
			ay->SetNdivisions( n1, n2, n3, opt );
		}
	}

	// Z-Axis
	if ( nullptr != az ){
		// Titl1
		if ( ("z" == option || "ztitle" == option )  ){
			az->SetTitle( params[ 0 ].c_str() );
		} else if ( ("zto" == option || "ztitleoffset" == option )  ){
			az->SetTitleOffset( atof(params[ 0 ].c_str()) );
		}
		else if ( ("zts" == option || "ztitlesize" == option ) ){
			az->SetTitleSize( atof(params[ 0 ].c_str()) );
		}
		// Label
		else if ( ("zlo" == option || "zlabeloffset" == option )  ){
			az->SetLabelOffset( atof(params[ 0 ].c_str()) );
		}
		else if ( ("zls" == option || "zlabelsize" == option ) ){
			az->SetLabelSize( atof(params[ 0 ].c_str()) );
		}	

		// Range
		else if ( ("zrange" == option || "zr" == option )  ){
			az->SetRangeUser( atof(params[ 0 ].c_str()), atof(params[ 1 ].c_str()) );
		}
		else if ( ("zbinrange" == option || "zbr" == option )  ){
			az->SetRange( atof(params[ 0 ].c_str()), atof(params[ 1 ].c_str()) );
		}

		// n ticks
		else if (  "zticks" == option || "ztick" == option ){
			int n1 = atoi(params[ 0 ].c_str());
			int n2 = 12;
			int n3 = 0;
			bool opt = true;
			if ( params.size() >= 2 )
				n2 = atoi(params[ 1 ].c_str());
			if ( params.size() >= 3 )
				n3 = atoi(params[ 2 ].c_str());
			if ( params.size() >= 3 )
				opt = (bool) atoi(params[ 3 ].c_str());
			
			az->SetNdivisions( n1, n2, n3, opt );
		}
	}
	
	
	// SEMI - GLOBAL
	// gPad Options
	if ( "logx" == option && nullptr != gPad )
		gPad->SetLogx( atoi( params[ 0 ].c_str() ) );
	if ( "logy" == option && nullptr != gPad )
		gPad->SetLogy( atoi( params[ 0 ].c_str() ) );
	if ( "logz" == option && nullptr != gPad )
		gPad->SetLogz( atoi( params[ 0 ].c_str() ) );
	if ( "gridx" == option && nullptr != gPad )
		gPad->SetGridx( atoi( params[ 0 ].c_str() ) );
	if ( "gridy" == option && nullptr != gPad )
		gPad->SetGridy( atoi( params[ 0 ].c_str() ) );

	// GLOBAL
	// gStyle Options
	if ( "stats" == option || "stat" == option || "optstat" == option){
		gStyle->SetOptStat( atoi( params[ 0 ].c_str() ) );
	}
	if ( "fitbox" == option || "fit" == option || "optfit" == option){
		gStyle->SetOptFit( atoi( params[ 0 ].c_str() ) );
	}


	// Line attributes
	TAttLine * line = dynamic_cast<TAttLine*>( styling );
	if ( line ){
		if ( "linecolor" == option || "lc" == option 
			|| "color" == option || "c" == option  ){
			int c = atoi( params[ 0 ].c_str() );
			if ( colorFromString >= 0 )
				c = colorFromString;
			DEBUG( classname(), "LC COLOR=" << c );
			line->SetLineColor( c );

		}
		if ( "linewidth" == option || "lw" == option){
			line->SetLineWidth( atof( params[ 0 ].c_str() ) );
		}
		if ("linestyle" == option || "lst" == option ){
			line->SetLineStyle( atoi( params[ 0 ].c_str() ) );
		}
	}

	// Fill attributes
	TAttFill * fill = dynamic_cast<TAttFill*>( styling );
	if ( fill ){
		if ( "fillcolor" == option || "fc" == option 
			|| "color" == option || "c" == option ){
			int c = atoi( params[ 0 ].c_str() );
			if ( colorFromString >= 0 )
				c = colorFromString;
			DEBUG( classname(), "FC COLOR=" << c );
			fill->SetFillColor( c );
		}
		if ( "fillcoloralpha" == option || "fca" == option ){
			int c = atoi( params[ 0 ].c_str() );
			if ( colorFromString >= 0 )
				c = colorFromString;
				DEBUG( classname(), "FCA COLOR=" << c );
#if ROOT6 > 0
			if ( "" != params[ 1 ] ){
				if ( nullptr != h )
					h->SetFillColorAlpha( c, atof( params[ 1 ].c_str() ) );
				if ( nullptr != g )
					g->SetFillColorAlpha( c, atof( params[ 1 ].c_str() ) );
			}
			else { 
				if ( nullptr != h )
					h->SetFillColor( c );
				if ( nullptr != g )
					g->SetFillColor( c );
			}
#else
			if ( nullptr != h )
					h->SetFillColor( c );
				if ( nullptr != g )
					g->SetFillColor( c );
#endif



		}
		if ( "fillstyle" == option || "fst" == option){
			fill->SetFillStyle( atoi( params[ 0 ].c_str() ) );
		}
	}

	// Marker attributes
	TAttMarker * marker = dynamic_cast<TAttMarker*>( styling );
	if ( marker ){
		
		if ( "markercolor" == option || "mc" == option 
			|| "color" == option || "c" == option ){
			int c = atoi( params[ 0 ].c_str() );
			if ( colorFromString >= 0 )
				c = colorFromString;
			DEBUG( classname(), "MC COLOR=" << c );
			marker->SetMarkerColor( c );
		}
		if ( "markersize" == option || "ms" == option ){
			marker->SetMarkerSize( atoi( params[ 0 ].c_str() ) );
		}
		if ( "markerstyle" == option || "mst" == option){
			marker->SetMarkerStyle( atoi( params[ 0 ].c_str() ) );
		}
	}


	return *this;

}

RooPlotLib &jdb::RooPlotLib::set( XmlConfig * cfg, string nodePath ){
	DEBUG(classname(), "");

	// get the list of attributes and set the style from that
	vector< string > list = cfg->attributesOf( nodePath );
	for ( unsigned int i = 0; i < list.size(); i++ ){
		vector<string> params = cfg->getStringVector( list[ i ] );
		DEBUG( classname(), "attr : " << cfg->attributeName( list[ i ] )  );

		if ( 0 == params.size()  )
			params.push_back( "" );
		set( cfg->attributeName( list[ i ] ), params );
	}

	return *this;
}

RooPlotLib &jdb::RooPlotLib::set( XmlConfig &cfg, string nodePath ){
	DEBUG(classname(), "");
	set( &cfg, nodePath );

	return *this;
}


RooPlotLib &jdb::RooPlotLib::draw(){
    DEBUG(classname(), "");
	if ( nullptr == styling ){
		ERROR( classname(), "Invalid object" );
		return *this;
	}

	styling->Draw( drawOption.c_str() );
	return *this;
}



// TODO: Add more colors
int jdb::RooPlotLib::color( string color ) {
    DEBUG(classname(), "( " + color + " )");
	transform(color.begin(), color.end(), color.begin(), ::tolower);
	if ( "red" == color )
		return kRed;
	if ( "green" == color )
		return kGreen;
	if ( "blue" == color )
		return kBlue;
	if ( "black" == color )
		return kBlack;
	return -1;
}














