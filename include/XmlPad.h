//
// Created by James Brandenburg on 6/4/15.
//

#ifndef ROOBARB_XMLPAD_H
#define ROOBARB_XMLPAD_H

#include "XmlConfig.h"
using namespace jdb;

#include "TPad.h"


namespace jdb{
	class XmlPad : public IObject{

	protected:

		string name, title;
		float x1, x2, y1, y2;
		float lm, rm, tm, bm;
		TPad * rootPad;


	public:
		virtual const char* classname() const { return "XmlPad"; }
		XmlPad( XmlConfig &_cfg, string _nodePath, int _nGridX = 6, int _nGridY = 6 ){
			
			if ( _cfg.exists( _nodePath ) ) {

				string preNode = "";
				if ( "" != _nodePath )
					preNode = _cfg.cn(_nodePath);

				int nGridX = _nGridX;
				int nGridY = _nGridY;
				x1 = _cfg.getFloat( ":x1", ndcFromGrid( _cfg.getFloat( ":gx1", 0 ), nGridX ) );
				x2 = _cfg.getFloat( ":x2", ndcFromGrid( _cfg.getFloat( ":gx2", 0 ), nGridX ) );

				y1 = _cfg.getFloat( ":y1", ndcFromGrid( _cfg.getFloat( ":gy1", 0 ), nGridY ) );
				y2 = _cfg.getFloat( ":y2", ndcFromGrid( _cfg.getFloat( ":gy2", 0 ), nGridY ) );

				if ( _cfg.exists( ":gx" ) && _cfg.getIntVector( ":gx" ).size() >=2 ){
					int gx1 = _cfg.getIntVector( ":gx" )[0];
					int gx2 = _cfg.getIntVector( ":gx" )[1];

					x1 = ndcFromGrid( gx1, nGridX );
					x2 = ndcFromGrid( gx2, nGridX );
				}

				if ( _cfg.exists( ":gy" ) && _cfg.getIntVector( ":gy" ).size() >=2 ){
					int gy1 = _cfg.getIntVector( ":gy" )[0];
					int gy2 = _cfg.getIntVector( ":gy" )[1];

					y1 = ndcFromGrid( gy1, nGridY );
					y2 = ndcFromGrid( gy2, nGridY );
				}

				lm =  0.1 / (x2 - x1);
				rm =  0.1 / (x2 - x1);
				tm =  0.1 / (y2 - y1);
				bm =  0.1 / (y2 - y1);

				// top right bottom left
				vector<double> ms = _cfg.getDoubleVector( ":m" );
				if ( _cfg.exists( ":m" ) && ms.size() >= 4 ){
					tm = ms[0] / ( y2 - y1 );
					rm = ms[1] / ( x2 - x1 );
					bm = ms[2] / ( y2 - y1 );
					lm = ms[3] / ( x2 - x1 );
				} else if ( _cfg.exists( ":m" ) && ms.size() == 1 ){
					tm = ms[0] / ( y2 - y1 );
					rm = ms[0] / ( x2 - x1 );
					bm = ms[0] / ( y2 - y1 );
					lm = ms[0] / ( x2 - x1 );
				}

				lm = _cfg.getFloat( ":lm", lm );
				rm = _cfg.getFloat( ":rm", rm );
				tm = _cfg.getFloat( ":tm", tm );
				bm = _cfg.getFloat( ":bm", bm );

				name = _cfg.getString( ":name", _cfg.getString( ":n", "c" ) );
				title = _cfg.getString( ":title", _cfg.getString( ":t", "c" ) );

				DEBUGC( "name=" << name << ", title=" << title <<", x1=" << x1 << ", x2=" << x2 << ", y1=" << y1 << ", y2=" << y2 );


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
				   _cfg.cn(preNode);
			}
		}

		void cd(){
			if ( !rootPad ){
				return;
			}
			rootPad->cd();
		}

		TPad * getRootPad(){
			return rootPad;
		}

		XmlPad * getPad(){
			return this;
		}

		void reposition(){
			if ( nullptr ==rootPad ) return;
			rootPad->SetPad( x1, y1, x2, y2 );
		}

		void moveToOrigin(  ){
			if ( nullptr ==rootPad ) return;
			rootPad->SetPad( 0, 0, x2-x1, y2-y1 );
		}

		
	protected:
		float ndcFromGrid( float gPoint, int nGridSpaces ){
			return ( gPoint / (float)nGridSpaces );
		}

	};
}// namespace

#endif //ROOBARB_XMLPAD_H
