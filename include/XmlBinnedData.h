#ifndef JDB_XML_BINNED_DATA_H
#define JDB_XML_BINNED_DATA_H

// ROOBARB
#include "XmlConfig.h"
#include "Utils.h"
#include "IObject.h"
#include "IConfig.h"
#include "HistoBins.h"

// STL
#include <memory>
#include <limits>

// ROOT
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"


namespace jdb {

	/* TGraph as xml
	 * Stores and loads a TGraph as xml structure
	 */
	class XmlBinnedData : public IObject, public IConfig {

	protected:
		vector<double> bins, y, eyl, eyh;

	public:
		virtual const char* classname() const { return "XmlBinnedData"; }
		XmlBinnedData(){

		}

		
		XmlBinnedData( TH1 * h ){
			
			bins.clear();
			y.clear();
			eyl.clear();
			eyh.clear();

			if ( h ){
				for ( int i = 0; i < h->GetNbinsX(); i++ ){
					bins.push_back( h->GetBinLowEdge( i+1 ) );
					y.push_back( h->GetBinContent( i+1 ) );
					eyl.push_back( h->GetBinError( i+1 ) );
					eyh.push_back( h->GetBinError( i+1 ) );
				}
				int i = h->GetNbinsX();
				bins.push_back( h->GetBinLowEdge( i+1 ) + h->GetBinWidth( i+1 ) );
			}
		}

		// bins from Histogram, values from graph
		XmlBinnedData( TH1 * h, TGraph* g ){
			
			bins.clear();
			y.clear();
			eyl.clear();
			eyh.clear();

			if ( h ){
				for ( int i = 0; i < h->GetNbinsX(); i++ ){
					bins.push_back( h->GetBinLowEdge( i+1 ) );
				}
				int i = h->GetNbinsX();
				bins.push_back( h->GetBinLowEdge( i+1 ) + h->GetBinWidth( i+1 ) );
			}

			if ( g ){
				double * yv = g->GetY();
				double * eylv = g->GetEYlow();
				double * eyhv = g->GetEYhigh();
				for ( int i = 0; i < g->GetN(); i++ ){
					y.push_back( yv[i] );
					eyl.push_back( eylv[i] );
					eyh.push_back( eyhv[i] );	
				}		
			}
		}

		XmlBinnedData( 	vector<double> _bins, 
						vector<double> _y, 
						vector<double> _ey ){
			bins = _bins;
			y = _y;
			eyl = _ey;
			eyh = _ey;
		}

		XmlBinnedData( XmlConfig &_config, string _nodePath ){
			loadConfig( _config, _nodePath );
		}

		XmlBinnedData( 	vector<double> _bins, 
						vector<double> _y, 
						vector<double> _eyl, 
						vector<double> _eyh ){
			bins = _bins;
			y = _y;
			eyl = _eyl;
			eyh = _eyh;
		}

		void loadConfig( XmlConfig &_config, string _nodePath ){
			config = _config;
			nodePath = _nodePath;

			bins = config.getDoubleVector( nodePath + ".bins" );
			y = config.getDoubleVector( nodePath + ".y" );
			eyl = config.getDoubleVector( nodePath + ".eyl", 0, y.size() );
			eyh = config.getDoubleVector( nodePath + ".eyh", 0, y.size() );

			if ( config.exists( nodePath + ".ey" ) ) { //symm errors
				eyl = config.getDoubleVector( nodePath + ".ey" );
				eyh = config.getDoubleVector( nodePath + ".ey" );				
			}
		}

		/* Linear interpolate between points
		 *
		 * @return interpolated value
		 */
		double linterp( double x, double x0, double y0, double x1, double y1 ){
			return y0 + ( y1 - y0 ) * ( (x - x0) / (x1 - x0) );
		}

		double eval( double x_val, double nSigmaError = 0.0 ) {

			int binIndex  = HistoBins::findBin( bins, x_val );

			if ( binIndex == (int)BinEdge::underflow )
				binIndex = 0;
			if ( binIndex == (int)BinEdge::overflow )
				binIndex = bins.size() - 1;


			if ( binIndex >= y.size() ){
				ERROR( classname(), "bin index [" << binIndex << "] is out of bounds" );
				return numeric_limits<double>::quiet_NaN();
			}

			if ( 0 == nSigmaError  )
				return y[ binIndex ];
			else if ( nSigmaError > 0 )  {
				return y[ binIndex ] + fabs( eyh[ binIndex ] * nSigmaError );
			} else if ( nSigmaError < 0 )  {
				return y[ binIndex ] - fabs( eyl[ binIndex ] * nSigmaError );
			}

			return numeric_limits<double>::quiet_NaN();
		}


		string toXml( string lp = "" ){
			string lx = "<bins>" + vts( bins ) + "</bins>";
			string ly = "<y>" + vts( y ) + "</y>";
			string leyl = "<eyl>" + vts( eyl ) + "</eyl>";
			string leyh = "<eyh>" + vts( eyh ) + "</eyh>";


			return lp + lx + "\n" + lp + ly + "\n" + lp + leyl + "\n" + lp + leyh + "\n";
		}


	};
}


#endif