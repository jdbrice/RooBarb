//
// Created by James Brandenburg on 6/4/15.
//

#ifndef ROOBARB_XMLPAD_H
#define ROOBARB_XMLPAD_H

#include "XmlConfig.h"
using namespace jdb;

#include "TPad.h"

class XmlCanvas;

class XmlPad {

protected:

	string name, title;
	float x1, x2, y1, y2;
	float lm, rm, tm, bm;
	TPad * rootPad;


public:
	XmlPad( XmlConfig * _cfg, string _nodePath ){
		TRACE("");
		if ( _cfg && _cfg->exists( _nodePath ) ) {

			string preNode = "";
			if ( "" != _nodePath )
				preNode = _cfg->cn(_nodePath);

			int nGridX = 6;
			int nGridY = 6;
			x1 = _cfg->getFloat( ":x1", ndcFromGrid( _cfg->getFloat( ":gx1", 0 ), nGridX ) );
			x2 = _cfg->getFloat( ":x2", ndcFromGrid( _cfg->getFloat( ":gx2", 0 ), nGridX ) );

			y1 = _cfg->getFloat( ":y1", ndcFromGrid( _cfg->getFloat( ":gy1", 0 ), nGridY ) );
			y2 = _cfg->getFloat( ":y2", ndcFromGrid( _cfg->getFloat( ":gy2", 0 ), nGridY ) );

			lm = _cfg->getFloat( ":lm", 0.1 / (x2 - x1) );
			rm = _cfg->getFloat( ":rm", 0.1 / (x2 - x1) );
			tm = _cfg->getFloat( ":tm", 0.1 / (y2 - y1) );
			bm = _cfg->getFloat( ":bm", 0.1 / (y2 - y1));

			name = _cfg->getString( ":name", _cfg->getString( ":n", "c" ) );
			title = _cfg->getString( ":title", _cfg->getString( ":t", "c" ) );

			DEBUG( "name=" << name << ", title=" << title <<", x1=" << x1 << ", x2=" << x2 << ", y1=" << y1 << ", y2=" << y2 );


			rootPad = new TPad( name.c_str(), title.c_str(), x1, y1, x2, y2 );
			rootPad->SetMargin( lm, rm, bm, tm );

			// fill style to transparent
			rootPad->SetFrameFillStyle(0); // this alone works for image formats like jpg, png, etc.
#if ROOT6 > 0
			rootPad->SetFillColorAlpha(0, 0);   // this is needed to also work for pdf and vector formats;
#else 
			rootPad->SetFillColor( 0 );   // this is needed to also work for pdf and vector formats;
#endif



			rootPad->Draw();
			rootPad->cd();

			if ( "" != _nodePath )
			   _cfg->cn(preNode);
		}
	}

	void cd(){
		if ( !rootPad ){
			ERROR( "ROOT PAD DNE" )
			return;
		}
		rootPad->cd();
	}

	
protected:
	float ndcFromGrid( float gPoint, int nGridSpaces ){
		return ( gPoint / (float)nGridSpaces );
	}

};

#endif //ROOBARB_XMLPAD_H
