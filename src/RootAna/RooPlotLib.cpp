#include "RooPlotLib.h"
#include "HistoBook.h"

//#include "TROOT.h" 
#include "TStyle.h"
#include "TColor.h"
#include "TGaxis.h"

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
    DEBUG(classname(), "( " << obj << ", name=" << obj->GetName() << " )")
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

	string p0 = "", p1 = "", p2 = "", p3 = "", p4 = "";
	if ( params.size() >= 1 ) p0 = params[0];
	if ( params.size() >= 2 ) p1 = params[1];
	if ( params.size() >= 3 ) p2 = params[2];
	if ( params.size() >= 4 ) p3 = params[3];
	if ( params.size() >= 5 ) p4 = params[4];
	float fp0 = atof( p0.c_str());
	float fp1 = atof( p1.c_str());
	float fp2 = atof( p2.c_str());
	float fp3 = atof( p3.c_str());
	float fp4 = atof( p4.c_str());

	int ip0 = atoi( p0.c_str());
	int ip1 = atoi( p1.c_str());
	int ip2 = atoi( p2.c_str());
	int ip3 = atoi( p3.c_str());
	int ip4 = atoi( p4.c_str());

	// force the param name to lowercase
	transform(option.begin(), option.end(), option.begin(), ::tolower);

	// commonly used so go ahead and get them
	// int ip0 = atoi( params[0].c_str() );
	// int ip1 = atoi( params[1].c_str() );

	// if param gives a valid color from a string then keep it for later
	int colorFromString = color( p0 );

	TH1 * h 	= dynamic_cast<TH1*>(styling);
	TGraph * g 	= dynamic_cast<TGraph*>(styling);
	TF1 * fn 	= dynamic_cast<TF1*>(styling);

	if ( "min" == option ){
		cout << "MIN: " << fp0 << endl;
		if ( nullptr != h ) h->SetMinimum( fp0 );
	}

	if ( "max" == option ){
		cout << "MAX: " << fp0 << endl;
		if ( nullptr != h ) h->SetMaximum( fp0 );
	}


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
		if ( nullptr != h ) h->SetTitle( p0.c_str() );
		if ( nullptr != g ) g->SetTitle( p0.c_str() );
		if ( nullptr != fn ) fn->SetTitle( p0.c_str() );
	}
	// TODO: not really working
	if ( "titlesize" == option || "ts" == option ){
		INFO( classname(), option << " " << p0 );
		gStyle->SetTitleFontSize( fp0 );
	}
	if ("titlex" == option ){
		gStyle->SetTitleX( fp0 );
	}
	if ("titley" == option ){
		gStyle->SetTitleY( fp0 );
	}
	if ("titlexy" == option ){
		gStyle->SetTitleX( fp0 );
		gStyle->SetTitleY( fp1 );
	}
	if ("titlealign" == option ){
		gStyle->SetTitleX( ip0 );
	}
	if ( "draw" == option ){
		drawOption = p0;
	}
	if ( "norm" == option && (p0 == "true" || ip0 > 0 ) ){
		drawNorm = true;
	} else {
		drawNorm = false;
	}

	if ( "clone" == option ){
		drawClone = true;
	} else {
		drawClone = false;
	}

	if ( "contours" == option ){
		if ( nullptr != h ) h->SetContour( ip0 );
	}




	if ( "axisdigits" == option ){
		TGaxis::SetMaxDigits( ip0 );
	}

	// Axis Stuff
	// X-Axis
	if ( nullptr != ax ){
		if ( ("x" == option || "xtitle" == option) ){
			ax->SetTitle( p0.c_str() );
		}
		else if ( ("xto" == option || "xtitleoffset" == option ) ){
			ax->SetTitleOffset( fp0 );
		}
		else if ( ("xts" == option || "xtitlesize" == option ) ){
			ax->SetTitleSize( fp0 );
		}
		// Label
		else if ( ("xlo" == option || "xlabeloffset" == option )  ){
			ax->SetLabelOffset( fp0 );
		}
		else if ( ("xls" == option || "xlabelsize" == option ) ){
			ax->SetLabelSize( fp0 );
		}
	

		// Range
		if ( ("xrange" == option || "xr" == option )  ){
			ax->SetRangeUser( fp0, fp1 );
		}
		if ( ("xbinrange" == option || "xbr" == option )  ){
			ax->SetRange( fp0, fp1 );
		}

		// n ticks
		else if (  "xticks" == option || "xtick" == option ){
			int n1 = ip0;
			int n2 = 12;
			int n3 = 0;
			bool opt = true;
			if ( params.size() >= 2 )
				n2 = ip1;
			if ( params.size() >= 3 )
				n3 = ip2;
			if ( params.size() >= 3 )
				opt = (bool) ip3;
			
			ax->SetNdivisions( n1, n2, n3, opt );
		}
	}
	// Y-Axis
	if ( nullptr != ay ){
		// Titl1
		if ( ("y" == option || "ytitle" == option )  ){
			ay->SetTitle( p0.c_str() );
		} else if ( ("yto" == option || "ytitleoffset" == option )  ){
			ay->SetTitleOffset( fp0 );
		}
		else if ( ("yts" == option || "ytitlesize" == option ) ){
			ay->SetTitleSize( fp0 );
		}
		// Label
		else if ( ("ylo" == option || "ylabeloffset" == option )  ){
			ay->SetLabelOffset( fp0 );
		}
		else if ( ("yls" == option || "ylabelsize" == option ) ){
			ay->SetLabelSize( fp0 );
		}	

		// Range
		else if ( ("yrange" == option || "yr" == option )  ){
			ay->SetRangeUser( fp0, fp1 );
		}
		else if ( ("ybinrange" == option || "ybr" == option )  ){
			ay->SetRange( fp0, fp1 );
		}

		// n ticks
		else if (  "yticks" == option || "ytick" == option ){
			int n1 = ip0;
			int n2 = 12;
			int n3 = 0;
			bool opt = true;
			if ( params.size() >= 2 )
				n2 = ip1;
			if ( params.size() >= 3 )
				n3 = ip2;
			if ( params.size() >= 3 )
				opt = (bool) ip3;
			
			ay->SetNdivisions( n1, n2, n3, opt );
		}
	}

	// Z-Axis
	if ( nullptr != az ){
		// Titl1
		if ( ("z" == option || "ztitle" == option )  ){
			az->SetTitle( p0.c_str() );
		} else if ( ("zto" == option || "ztitleoffset" == option )  ){
			az->SetTitleOffset( fp0 );
		}
		else if ( ("zts" == option || "ztitlesize" == option ) ){
			az->SetTitleSize( fp0 );
		}
		// Label
		else if ( ("zlo" == option || "zlabeloffset" == option )  ){
			az->SetLabelOffset( fp0 );
		}
		else if ( ("zls" == option || "zlabelsize" == option ) ){
			az->SetLabelSize( fp0 );
		}	

		// Range
		else if ( ("zrange" == option || "zr" == option )  ){
			az->SetRangeUser( fp0, fp1 );
		}
		else if ( ("zbinrange" == option || "zbr" == option )  ){
			az->SetRange( fp0, fp1 );
		}

		// n ticks
		else if (  "zticks" == option || "ztick" == option ){
			int n1 = ip0;
			int n2 = 12;
			int n3 = 0;
			bool opt = true;
			if ( params.size() >= 2 )
				n2 = ip1;
			if ( params.size() >= 3 )
				n3 = ip2;
			if ( params.size() >= 3 )
				opt = (bool) ip3;
			
			az->SetNdivisions( n1, n2, n3, opt );
		}
	}
	
	
	// SEMI - GLOBAL
	// gPad Options
	if ( "logx" == option && nullptr != gPad )
		gPad->SetLogx( ip0 );
	if ( "logy" == option && nullptr != gPad )
		gPad->SetLogy( ip0 );
	if ( "logz" == option && nullptr != gPad )
		gPad->SetLogz( ip0 );
	if ( "gridx" == option && nullptr != gPad )
		gPad->SetGridx( ip0 );
	if ( "gridy" == option && nullptr != gPad )
		gPad->SetGridy( ip0 );

	// GLOBAL
	// gStyle Options
	if ( "stats" == option || "stat" == option || "optstat" == option){
		gStyle->SetOptStat( ip0 );
	}
	if ( "fitbox" == option || "fit" == option || "optfit" == option){
		gStyle->SetOptFit( ip0 );
	}


	// Line attributes
	TAttLine * line = dynamic_cast<TAttLine*>( styling );
	if ( line ){
		if ( "linecolor" == option || "lc" == option 
			|| "color" == option || "c" == option  ){
			int c = ip0;
			if ( colorFromString >= 0 )
				c = colorFromString;
			DEBUG( classname(), "LC COLOR=" << c );
			line->SetLineColor( c );

		}
		if ( "linewidth" == option || "lw" == option){
			line->SetLineWidth( atof( p0.c_str() ) );
		}
		if ("linestyle" == option || "lst" == option ){
			line->SetLineStyle( ip0 );
		}
	}

	// Fill attributes
	TAttFill * fill = dynamic_cast<TAttFill*>( styling );
	if ( fill ){
		if ( "fillcolor" == option || "fc" == option 
			|| "color" == option || "c" == option ){
			int c = ip0;
			if ( colorFromString >= 0 )
				c = colorFromString;
			DEBUG( classname(), "FC COLOR=" << c );
			fill->SetFillColor( c );
		}
		if ( "fillcoloralpha" == option || "fca" == option ){
			int c = ip0;
			if ( colorFromString >= 0 )
				c = colorFromString;
				DEBUG( classname(), "FCA COLOR=" << c );
	#if ROOT6 > 0
			if ( "" != p1 ){
				if ( nullptr != h )
					h->SetFillColorAlpha( c, fp1 );
				if ( nullptr != g )
					g->SetFillColorAlpha( c, fp1 );
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
			fill->SetFillStyle( ip0 );
		}
	}

	// Marker attributes
	TAttMarker * marker = dynamic_cast<TAttMarker*>( styling );
	if ( marker ){
		
		if ( "markercolor" == option || "mc" == option 
			|| "color" == option || "c" == option ){
			int c = ip0;
			if ( colorFromString >= 0 )
				c = colorFromString;
			DEBUG( classname(), "MC COLOR=" << c );
			marker->SetMarkerColor( c );
		}
		if ( "markersize" == option || "ms" == option ){
			marker->SetMarkerSize( fp0 );
		}
		if ( "markerstyle" == option || "mst" == option){
			marker->SetMarkerStyle( ip0 );
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
	if ( cfg.exists( cfg.getString( nodePath ) ) ){
		set( &cfg, cfg.getString( nodePath ) );
	}
	set( &cfg, nodePath );

	return *this;
}


RooPlotLib &jdb::RooPlotLib::draw(){
    DEBUG(classname(), "");
	if ( nullptr == styling ){
		ERROR( classname(), "Invalid object" );
		return *this;
	}

	if ( true == drawNorm && static_cast<TH1*>( styling ) )
		static_cast<TH1*>( styling )->DrawNormalized( drawOption.c_str() );
	else if ( true == drawClone )
		styling->DrawClone( drawOption.c_str() );
	else 
		styling->Draw( drawOption.c_str() );

	return *this;
}

RooPlotLib &jdb::RooPlotLib::draw( string appendDrawOpt ){
	drawOption += " same";
	return draw();
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

	if ( color[0] == '#' && color.size() == 7 ){
		return TColor::GetColor( color.c_str() );
	}
	return -1;
}














