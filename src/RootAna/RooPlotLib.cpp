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

void jdb::RooPlotLib::link( XmlConfig* xfg ){
	this->linkedConfig = xfg;
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

void jdb::RooPlotLib::style_axes( string opt, vector<string> params ){
	// opt should already be normalized

	TAxis * axis = nullptr;

	TH1 * h 	= dynamic_cast<TH1*>(styling);
	TGraph * g 	= dynamic_cast<TGraph*>(styling);
	TF1 * fn 	= dynamic_cast<TF1*>(styling);

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

	if ( 'x' == opt[0] )
		axis = ax;
	else if( 'y' == opt[0] )
		axis = ay;
	else if ( 'z' == opt[0] )
		axis = az;

	if ( nullptr == axis )
		return;

	string p0 = "", p1 = "", p2 = "", p3 = "", p4 = "";
	if ( params.size() >= 1 ) p0 = params[0];
	if ( params.size() >= 2 ) p1 = params[1];
	if ( params.size() >= 3 ) p2 = params[2];
	if ( params.size() >= 4 ) p3 = params[3];
	if ( params.size() >= 5 ) p4 = params[4];
	float fp0 = atof( p0.c_str());
	float fp1 = atof( p1.c_str());

	int ip0 = atoi( p0.c_str());
	int ip1 = atoi( p1.c_str());
	int ip2 = atoi( p2.c_str());
	int ip3 = atoi( p3.c_str());


	// should strip the x,y,z part
	string axis_opt = opt.substr( 1 );


	if ( ("title" == axis_opt) ){
		axis->SetTitle( p0.c_str() );
	}
	else if ( ("to" == axis_opt || "titleoffset" == axis_opt ) ){
		axis->SetTitleOffset( fp0 );
	}
	else if ( ("ts" == axis_opt || "titlesize" == axis_opt ) ){
		axis->SetTitleSize( fp0 );
	}
	else if ( ("tp" == axis_opt || "titlepoint" == axis_opt ) ){
		axis->SetTitleSize( fp0 / fontScale );
	} else if (  "tc" == axis_opt || "titlecenter" == axis_opt  ){
		axis->CenterTitle();
	}
	else if ( ("lo" == axis_opt || "labeloffset" == axis_opt )  ){
		axis->SetLabelOffset( fp0 );
	}
	else if ( ("ls" == axis_opt || "labelsize" == axis_opt ) ){
		axis->SetLabelSize( fp0 );
	}
	else if ( ("lp" == axis_opt || "labelpoint" == axis_opt ) ){
		axis->SetLabelSize( fp0 / fontScale );
	} 
	else if ( "ta" == axis_opt || "titlealign" == axis_opt ){
		if ("center" == p0) {
			axis->CenterTitle( true );
		}
	}
	else if ( ("range" == axis_opt || "r" == axis_opt )  ){
		axis->SetRangeUser( fp0, fp1 );
	}
	else if ( ("binrange" == axis_opt || "br" == axis_opt )  ){
		axis->SetRange( fp0, fp1 );
	}
	else if (  "ticks" == axis_opt || "tick" == axis_opt ){
		int n1 = ip0;
		int n2 = 12;
		int n3 = 0;
		bool opt = true;
		if ( params.size() >= 2 )
			n2 = ip1;
		if ( params.size() >= 3 )
			n3 = ip2;
		if ( params.size() >= 4 )
			opt = (bool) ip3;
		
		axis->SetNdivisions( n1, n2, n3, opt );
	} else if ( "moreloglabels" == axis_opt || "mll" == axis_opt ){
		axis->SetMoreLogLabels( ip0 );
		cout << "MORE LOG LABELS" << endl;
	} else if ( "noexp" == axis_opt ){
		axis->SetNoExponent( ip0 );
		cout << "NO EXPONENT" << endl;
	}
	cout << "STYLE AXIS : " << axis_opt << endl;


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

	option = normalize( option );

	string p0 = "", p1 = "", p2 = "", p3 = "", p4 = "";
	if ( params.size() >= 1 ) p0 = params[0];
	if ( params.size() >= 2 ) p1 = params[1];
	if ( params.size() >= 3 ) p2 = params[2];
	if ( params.size() >= 4 ) p3 = params[3];
	if ( params.size() >= 5 ) p4 = params[4];
	float fp0 = atof( p0.c_str());
	float fp1 = atof( p1.c_str());
	// float fp2 = atof( p2.c_str());
	// float fp3 = atof( p3.c_str());
	// float fp4 = atof( p4.c_str());

	int ip0 = atoi( p0.c_str());
	// int ip1 = atoi( p1.c_str());
	// int ip2 = atoi( p2.c_str());
	// int ip3 = atoi( p3.c_str());
	// int ip4 = atoi( p4.c_str());

	

	// if param gives a valid color from a string then keep it for later
	int colorFromString = color( p0 );

	TH1 * h 	= dynamic_cast<TH1*>(styling);
	TGraph * g 	= dynamic_cast<TGraph*>(styling);
	TF1 * fn 	= dynamic_cast<TF1*>(styling);


	/***************************************************************************
	 * Global Pad
	 ****************************************************************************/

	if ( nullptr == gPad ){
		if ( "logx" == option || "logy" == option || "logz" == option || "gridx" == option || "gridy" == option ){
			ERRORC( "gPad is null, cannot set property" );
		}
	}

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

	if ( "topxaxis" == option || "topx" == option ){
		if (nullptr != gPad)
			gPad->SetTickx( ip0 );
	}
	if ( "rightyaxis" == option || "righty" == option ){
		if (nullptr != gPad)
			gPad->SetTicky( ip0 );
	}
	if ( "theta" == option && nullptr != gPad){
		gPad->SetTheta( fp0 );
		gPad->Update();
	}
	if ( "phi" == option && nullptr != gPad){
		gPad->SetPhi( fp0 );
		gPad->Update();
	}

	/***************************************************************************
	 * Global Drawing options
	 ****************************************************************************/
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

	if ( "axisdigits" == option ){
		TGaxis::SetMaxDigits( ip0 );
	}

	/***************************************************************************
	 * Histogram / Graph / Function
	 ****************************************************************************/
	if ( "title" == option || "t" == option ){
		if ( nullptr != h ) h->SetTitle( p0.c_str() );
		if ( nullptr != g ) g->SetTitle( p0.c_str() );
		if ( nullptr != fn ) fn->SetTitle( p0.c_str() );
	}
	if ( "contours" == option ){
		if ( nullptr != h ) h->SetContour( ip0 );
	}
	if ( "min" == option ){
		if ( nullptr != h ) h->SetMinimum( fp0 );
	}
	if ( "max" == option ){
		if ( nullptr != h ) h->SetMaximum( fp0 );
	}


	style_axes( option, params );

	/***************************************************************************
	 * Global Style
	 ****************************************************************************/
	if ( "stats" == option || "stat" == option || "optstat" == option){
		gStyle->SetOptStat( ip0 );
		if ( nullptr != h )
			h->SetStats( ip0 );
	}
	if ( "fitbox" == option || "fit" == option || "optfit" == option){
		gStyle->SetOptFit( ip0 );
	}
	if ( "titlesize" == option || "ts" == option ){
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


	/***************************************************************************
	 * Line Attributes
	 ****************************************************************************/
	TAttLine * line = dynamic_cast<TAttLine*>( styling );
	if ( nullptr != line ){
		if ( "linecolor" == option || "lc" == option || "color" == option || "c" == option ){
			int c = ip0;
			if ( colorFromString >= 0 )
				c = colorFromString;
			line->SetLineColor( c );
		}
		if ( "linewidth" == option || "lw" == option){
			line->SetLineWidth( atof( p0.c_str() ) );
		}
		if ("linestyle" == option || "lst" == option ){
			line->SetLineStyle( ip0 );
		}
	}

	/***************************************************************************
	 * Fill Attributes
	 ****************************************************************************/
	TAttFill * fill = dynamic_cast<TAttFill*>( styling );
	if ( nullptr != fill ){
		if ( "fillcolor" == option || "fc" == option || "color" == option || "c" == option || "fillcoloralpha" == option || "fca" == option){
			int c = ip0;
			if ( colorFromString >= 0 )
				c = colorFromString;
			fill->SetFillColor( c );

			if ( 2 == params.size() ){
	#if ROOT6 > 0
				if ( nullptr != h )
					h->SetFillColorAlpha( c, fp1 );
				if ( nullptr != g )
					g->SetFillColorAlpha( c, fp1 );
	#endif
			}
		}
		if ( "fillstyle" == option || "fst" == option){
			fill->SetFillStyle( ip0 );
		}
	}

	/***************************************************************************
	 * Marker Attributes
	 ****************************************************************************/
	TAttMarker * marker = dynamic_cast<TAttMarker*>( styling );
	if ( nullptr != marker ){
		
		if ( "markercolor" == option || "mc" == option || "color" == option || "c" == option ){
			int c = ip0;
			if ( colorFromString >= 0 )
				c = colorFromString;
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

RooPlotLib &jdb::RooPlotLib::set(string nodePath ){
	DEBUG(classname(), "");

	if ( nullptr == this->linkedConfig ){
		ERRORC( "No config linked" );
		return *this;
	}

	return set( this->linkedConfig, nodePath );
}



string jdb::RooPlotLib::normalizeAttribute( string str ){
	// remove '-'
	std::string::iterator end_pos = std::remove(str.begin(), str.end(), '-');
	str.erase(end_pos, str.end());

	// remove '_'
	end_pos = std::remove(str.begin(), str.end(), '_');
	str.erase(end_pos, str.end());

	// lowercase
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}
string jdb::RooPlotLib::normalize( string str ){
	return RooPlotLib::normalizeAttribute(str);
}

RooPlotLib &jdb::RooPlotLib::draw(){
    DEBUG(classname(), "");
	if ( nullptr == styling ){
		ERROR( classname(), "Invalid object" );
		return *this;
	}

	if ( "none" == drawOption || "no" == drawOption )
		return *this;

	if ( true == drawNorm && static_cast<TH1*>( styling ) )
		static_cast<TH1*>( styling )->DrawNormalized( drawOption.c_str() );
	else if ( true == drawClone )
		styling->DrawClone( drawOption.c_str() );
	else 
		styling->Draw( drawOption.c_str() );

	return *this;
}

RooPlotLib &jdb::RooPlotLib::draw( string appendDrawOpt ){
	drawOption += " " + appendDrawOpt;
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
	} else if ( color[0] == '#' && color.size() == 4 ){
		string colstr = "#";
		colstr += color[1];
		colstr += color[1];
		colstr += color[2];
		colstr += color[2];
		colstr += color[3];
		colstr += color[3];
		return TColor::GetColor( colstr.c_str() );
	}
	return -1;
}














