#ifndef JDB_XML_GRAPH_H
#define JDB_XML_GRAPH_H

// ROOBARB
#include "XmlConfig.h"
#include "Utils.h"
#include "IObject.h"
#include "HistoBins.h"

// STL
#include <memory>

// ROOT
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"


namespace jdb {

	/* TGraph as xml
	 * Stores and loads a TGraph as xml structure
	 */
	class XmlGraph : public IObject
	{
	protected:

		vector<double> x, y, exh, exl, eyh, eyl;
	public:
		virtual const char* classname() const { return "XmlGraph"; }
		XmlGraph( XmlConfig * cfg, string nodePath ){

			x = cfg->getDoubleVector( nodePath + ".x" );
			y = cfg->getDoubleVector( nodePath + ".y" );
		}
		~XmlGraph(){
			
		}

		/* Evaluates the graph using the given method
		 *
		 * @x_val the value at which to evaluate the graph
		 * @interpolate the interpolation method one of :
		 *              "linear" - linear interpolate points,
		 *              "nearest" - nearest data point,
		 *              "" ie none - first point that is less than x_val (assuming monotonically inscreasing values )
		 *
		 * @return 	The value at the given x_val using the interpolation method
		 */
		double eval( double x_val, string interpolate = "linear" ) {

			int index = -1;
			// first find the two points we are between
			// we assume monotomically increasing values of x
			for ( unsigned int iX = 0; iX < x.size() - 1; iX++ ){
				if ( x_val < x[ iX + 1] ){
					index = iX;
					break;
				}
			}

			index = HistoBins::findBin( x, x_val );
			INFO( classname(), "bins = [" << vts( x ) << "]" );
			INFO( classname(), "index = " << index );

			if ( -1 == index && x_val < x[0] ){
				index = 0; // use the first two values
			} else if ( -1 == index ){
				// TODO: signal error here
				WARN( classname(), "stupid" );
				return 0.0;
			}

			if ( "closest" == interpolate || "nearest" == interpolate ){
				if ( abs( x_val - x[index] ) <= abs( x_val - x[index + 1] ) )
					return y[index];
				else
					return y[index+1];
			}

			if ( "linear" == interpolate ){
				return linterp( x_val, x[index], y[index], x[index+1], y[index+1] );
			}

			// default if no interp method 
			return y[index];

		}

		/* Linear interpolate between points
		 *
		 * @return interpolated value
		 */
		double linterp( double x, double x0, double y0, double x1, double y1 ){
			return y0 + ( y1 - y0 ) * ( (x - x0) / (x1 - x0) );
		}


		/* Converts a Root Graph (TGraph or subclass) to an xml data node
		 * @g 			Graph to export as XML attributes
		 * @linePrefix  prefix for lines
		 * 
		 * @return 		string representing the Graph as XML attributes
		 */
		static string toXml(  TGraph * g, string linePrefix =""  ){

			string lx = "<x>";
			string ly = "<y>";

			for ( int iP = 0; iP < g->GetN(); iP++ ){
				double x, y;
				g->GetPoint( iP, x, y );
				lx += dts( x );
				ly += dts( y );

				if ( iP < g->GetN() - 1 ){
					lx += ", ";
					ly += ", ";
				}
			}
			lx += "</x>";
			ly += "</y>";

			return linePrefix + lx + "\n" + linePrefix + ly;
		} // toXml(  TGraph * g, string linePrefix  )

		/* Converts a Root Graph with symmetric errors (TGraph or subclass) to an xml data node
		 * @g 			Graph to export as XML attributes
		 * @linePrefix  prefix for lines
		 * 
		 * @return 		string representing the Graph as XML attributes
		 */
		static string toXml( TGraphErrors * g, string linePrefix ="" ){
			string a = XmlGraph::toXml( (TGraph*) g, linePrefix );

			string lex = "<ex>";
			string ley = "<ey>";

			for ( int iP = 0; iP < g->GetN(); iP++ ){
				double ex, ey;
				ex = g->GetErrorX( iP );
				ey = g->GetErrorY( iP );
				lex += dts( ex );
				ley += dts( ey );

				if ( iP < g->GetN() - 1 ){
					lex += ", ";
					ley += ", ";
				}
			}

			return a + "\n" + lex + "\n" + ley;
		} // toXml( TGraphErrors * g )

		/* Converts a Root Graph with asymmetric errors (TGraphAsymmErrors or subclass) to an xml data node
		 * @g 			Graph to export as XML attributes
		 * @linePrefix  prefix for lines
		 * 
		 * @return 		string representing the Graph as XML attributes
		 */
		static string toXml( TGraphAsymmErrors * g, string linePrefix ="" ){
			string a = XmlGraph::toXml( (TGraph*) g, linePrefix );

			string lexh = "<exh>";
			string leyh = "<eyh>";
			string lexl = "<exl>";
			string leyl = "<eyl>";

			for ( int iP = 0; iP < g->GetN(); iP++ ){
				double exh, eyh;
				double exl, eyl;
				exh = g->GetErrorXhigh( iP );
				eyh = g->GetErrorYhigh( iP );
				lexh += dts( exh );
				leyh += dts( eyh );

				exl = g->GetErrorXlow( iP );
				eyl = g->GetErrorYlow( iP );
				lexl += dts( exl );
				leyl += dts( eyl );

				if ( iP < g->GetN() - 1 ){
					lexh += ", ";
					leyh += ", ";
					lexl += ", ";
					leyl += ", ";
				}
			}

			lexh += "</exh>";
			leyh += "</eyh>";
			lexl += "</exl>";
			leyl += "</eyl>";

			return a + "\n" + linePrefix + lexh + "\n" + linePrefix + leyh +"\n" + linePrefix + lexl + "\n" + linePrefix + leyl;
		} // toXml( TGraphAsymmErrors * g, string linePrefix )



#ifdef __CINT__
		ClassDef( jdb::XmlGraph, 1 )
#endif
		
	}; // XmlGraph
}// namespace jdb

#endif