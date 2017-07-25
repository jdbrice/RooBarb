
#include "Reporter.h"
#include "Utils.h"
#include "LoggerConfig.h"

namespace jdb{

	int Reporter::instances = 0;

	Reporter::Reporter( string filename, int w, int h ){
		DEBUG( classname(), "(" <<  filename <<  ", width=" <<  w <<  ", height=" <<  h << ")" )
	
		this->filename = filename;

		canvas = new TCanvas( ("Reporter"+ts(instances)).c_str() , "canvas", w, h);
		canvas->Print( ( filename + "[" ).c_str() );
		INFO( classname(), " Opening report " << filename );
		instances++;

		isOpen = true;

		DEBUG( classname(), " Instance #" << instances );
	}

	Reporter::Reporter( string filename, TCanvas * _canvas ){
		DEBUG( classname(), "(" <<  filename <<  ", canvas=" << _canvas << ")" )
	
		this->filename = filename;

		canvas = _canvas;
		if ( nullptr == canvas ){
			ERRORC( "Canvas was NULL, fallback to new canvas" );
			canvas = new TCanvas( ("Reporter"+ts(instances)).c_str() , "canvas", 800, 800);
			instances++;
		}

		canvas->Print( ( filename + "[" ).c_str() );
		INFO( classname(), " Opening report " << filename );
	
		isOpen = true;

		DEBUG( classname(), " Instance #" << instances );
	}

	Reporter::Reporter( XmlConfig &config, string np, string prefix ){
		DEBUG( classname(), "( config, np=" << np << ", prefix=" << prefix << ")" )

		this->config = config;
		this->nodePath = config.basePath( np );

		this->filename = prefix + config.getXString( nodePath + ".output:url" );

		int w = config.getInt( nodePath + ".output:width", 400 );
		int h = config.getInt( nodePath + ".output:height", 400 );

		canvas = new TCanvas( ("Reporter" + ts( instances ) ).c_str() , "canvas", w, h);
		canvas->Print( ( filename + "[" ).c_str() );
		
		DEBUG( classname(), " Opening " << filename ) 
		
		instances++;

		isOpen = true;

		DEBUG( classname(), " Instance #" << instances )
	}

	Reporter::~Reporter() {
		// properly close the report file
		DEBUG( classname(), "" )
		
		if ( isOpen )
			canvas->Print( ( filename + "]" ).c_str() );
		delete canvas;

		DEBUG( classname(), filename << " Closed " );
	}

	void Reporter::close(){
		canvas->Print( ( filename + "]" ).c_str() );
		isOpen = false;
	}

	void Reporter::newPage( int dx, int dy, float marginX, float marginY ){
		DEBUG( classname(), " New Page ( " << dx << ", " << dy << ", " << marginX << ", " << marginY << " ) " );

		this->dx = dx;
		this->dy = dy;

		canvas->Clear();
		canvas->Divide( dx, dy, marginX, marginY );
		cd( 1 );
	}

	void Reporter::cd( int pad ){
		DEBUG( classname(), " Current Pad " << pad  );
		canvas->cd( pad );
		currentPad = pad;
	}

	void Reporter::cd( int x, int y){
		cd( ( (y - 1 ) * dx) + x );
	}

	void Reporter::next(){
		DEBUG( classname(), " Pushing to next pad " );
		currentPad++;
		if ( currentPad > dx*dy){
			savePage();
			newPage( dx, dy);
		} else {
			cd( currentPad );
		}
	}

	void Reporter::savePage( string name ) {
		DEBUG( classname(), "Save : " << name  );
		if ( "" == name ){
			canvas->Print( ( filename ).c_str() );
			DEBUG( classname(), " Saving Page to " << filename );
		}
		else {
			canvas->Print( name.c_str() );
			DEBUG( classname(), " Saving Page to " << name );
		}
	}

	void Reporter::savePage( TCanvas *_canvas ){
		if ( nullptr == _canvas ) return;
		_canvas->Print( ( filename ).c_str() );
	}

	void Reporter::saveImage( string name ){
		DEBUG( classname(), " Saving Image to " << name );
		canvas->SaveAs( name.c_str() );
	}
	
}
