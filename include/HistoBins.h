#ifndef HISTOGRAM_BINS
#define HISTOGRAM_BINS

// ROOBARB
#include "XmlConfig.h"
#include "Utils.h"
#include "Logger.h"

	#include "IObject.h"


// STL
#include <memory>
#include <string> 
#include <algorithm>
#include <limits>
#include <math.h>
#include <unordered_map>
using namespace std;

namespace jdb{

	enum class BinEdge: int {
		lower,
		upper,
		underflow = -1,
		overflow = -2,
		undefined = -3,
		ambiguous = -4		// only used by LUT to signal that it needs fallback
	};
	
	/* HistoBins provides a container for storing and using bins
	 *
	 * Provides a stand-alone binning container. Can be create directly from configs.
	 */
	class HistoBins : public IObject
	{
	public:
		
		virtual const char* classname() const { return "HistoBins"; }

		/* Makes a vector of bins with fixed width
		 * Divides the range high - low into a fixed number of bins from low to high"
		 */
		static vector<double> makeNBins( int nBins, double low, double high ){
			DEBUG( "HistoBins", "(nBins=" << nBins << ", low=" << low << ", high=" << high << ")" );
			vector<double> bins;
			double step 	= (high - low ) / (double) nBins;
			double binEdge 	= low;
			for (int iBin = 0; iBin < nBins; iBin++ ){
				bins.push_back( binEdge );
				binEdge += step;
			}
			bins.push_back( high ); // last bin edge is upper limit
		
			return bins;
		}	// makeNBins
		
		/* Makes a vector of bin edges for bins of a fixed width
		 * @binWidth the width of each bin. The last bin may be smaller.
		 * @low the low edge of the bins
		 * @high the high edge of the bins
		 * @return vector of bin edges from low to high
		 */
		static vector<double> makeFixedWidthBins( double binWidth, double low, double high, bool canExtendMax = true ){
			DEBUG( "HistoBins", "(binWidth=" << binWidth << ", low=" << low << ", high=" << high << " , canExtendMax=" << bts(canExtendMax) << ")" );
			vector<double> bins;
			int nBins = ceil(( high - low ) / binWidth);
			double binEdge 	= low;
			for (int iBin = 0; iBin < nBins; iBin++ ){
				bins.push_back( binEdge );
				binEdge += binWidth;
			}
			if ( false == canExtendMax )
				bins.push_back( high ); // last bin edge is upper limit
			else 
				bins.push_back( binEdge );
		
			return bins;
		}	// binsFrom


		static vector<double> makeQuantileBins( vector<double> &values, int nBins, double min=-1.0, double max=-1.0 ){
			DEBUG( "HistoBins", "( data.size=" << values.size() << ", nBins=" << nBins << ", min=" << min << ", max=" << max << ")" );
			int nEntries = values.size();
			if ( nEntries <= 0 )
				return makeNBins( nBins, min, max );

			std::sort( values.begin(), values.end() );
			vector<double> binEdges;

			double step = (double)nEntries / (double)nBins;
			if ( abs( -1.0 - min ) < 0.001 ) 
				min = values[0];
			if ( abs( -1.0 - max ) < 0.001 || max < min) 
				max = values[values.size() - 1 ];
			binEdges.push_back( min );

			for ( int i = 1; i < nBins; i++ ){
				int iVal = step * i;
				if ( iVal > nEntries ){
					// warn?
					break;
				}
				double vat = values.at( iVal );	
				binEdges.push_back( vat );
			}
			binEdges.push_back( max );
			
			return binEdges;

		}

		/* Finds the bin containing a given value
		 * @bins 		The bin edges to search in
		 * @val 		The value for which the corresponding bin is desired
		 * @includeEdge	The edge to include in the bin, either lower edge or higher edge
		 *
		 * @return 		Underflow 	: -1
		 * 				Overflow 	: -2
		 * 				Undefined 	: -3
		 * 				Otherwise 	: Bin Index starting at 0
		 */
		static int findBin( vector<double> &bins, double val, BinEdge includeEdge = BinEdge::lower ){
            string sbe = "invalid";
            if ( BinEdge::lower == includeEdge )
                sbe = "lower";
            else
                sbe = "upper";
            DEBUG( "HistoBins", "( vector<double> , value=" << val << ", binEdge=" << sbe << " ) " );


			int n = bins.size();

			if ( n < 2 )
				return -3;

			// overflow and underflow
			if ( includeEdge == BinEdge::lower ){
				if ( val < bins[ 0 ] )
					return -1;
				else if ( val >= bins[ n - 1 ] )
					return -2;
			} else if ( includeEdge == BinEdge::upper ){
				if ( val <= bins[ 0 ] )
					return -1;
				else if ( val > bins[ n - 1 ] )
					return -2;
			} else {
				return -3;
			}

			// if not overflow / underflow
			// then find the corresponding bin
			for ( int i = n-2; i >= 0; i-- ){
				if ( val >= bins[ i ] && includeEdge == BinEdge::lower )
					return i;
				else if ( val > bins[ i ] && includeEdge == BinEdge::upper )
					return i;
			}

			return -3;

		}	// findBin

		/* Finds the bin via a LUT
		 *
		 * This method exports the range lookup to a LUT **unless** the value is near a bin-boundary, then it falls back to normal 
		 */
		static int findBin( double val, unordered_map<int, int> &_binIndexLUT, double _param){
			int ival = transformToInt( _param, val );
			if ( _binIndexLUT.count( ival ) <= 0 ) return -3;
			if ( _binIndexLUT.size() > 0 && _param > 0.0 ){
				int iHere = _binIndexLUT[ ival ];
				int iPrev = _binIndexLUT[ ival - 1 ]; 
				int iNext = _binIndexLUT[ ival + 1 ]; 
				if ( iHere != iPrev || iHere != iNext )
					return (int)BinEdge::ambiguous;
				return iHere;
			} else {
				ERROR( "HistoBins", "Invalid LUT size / param " << _binIndexLUT.size() << " / " << _param );
			}
			return (int)BinEdge::undefined;
		}	// findBin

		/* Bin Width from vector of bin edges and bin index
		 * @bins 		vector of bin edges
		 * @binIndex 	bin Index
		 * @return 		bin width or 0.0 for error (also issues an ERROR(...) log)
		 */
		static double binWidth( vector<double> &bins, unsigned int binIndex = 0 ){

			if ( binIndex >= bins.size() ){
				ERROR( "HistoBins", "Bin Index " << binIndex << " out of range ( 0, " << bins.size() << " )" );
				return 0.0;
			}
			return (bins[ binIndex + 1 ] - bins[ binIndex ]);
		}

		static double findBinWidth( vector<double> &bins, double value, BinEdge includeEdge = BinEdge::lower ){
			int binIndex = findBin( bins, value, includeEdge );
			return binWidth( bins, binIndex );
		}

		/* Finds the bin containing a given value
		 * @val The value for which the corresponding bin is desired 
		 * @includeEdge 	which edge of the bin to include, upper or lower
		 * @return 			the index of the bin see static implementation for more detail
		 */
		int findBin( double val, BinEdge includeEdge = BinEdge::lower ){
			int iLUT = -4;
			if ( useLUT )
				iLUT = findBin( val, binIndexLUT, smallestBinWidth );
			if ( iLUT != (int)BinEdge::ambiguous ) return iLUT;

			// if the LUT got an ambiguouse result then fallback to the normal method
			return findBin( bins, val, includeEdge );
		} // findBin

		/* Bin Width
		 * @binIndex 		The index of the bin whose width you want to know
		 * @return 			The bin width or 0.0 for error ( also issues an ERROR(...) log)
		 */
		double binWidth( int binIndex = 0 ){
			return binWidth( bins, binIndex );
		}
		double findBinWidth( double value, BinEdge includeEdge = BinEdge::lower ) {
			return findBinWidth( bins, value, includeEdge );
		}

		/*
		 * Length of underlying vector containing the bin edges
		 */
		int length() {
			return bins.size();
		}
		/*
		 * Size of underlying vector containing the bin edges
		 */
		int size() {
			return bins.size();
		}
		/*
		 * Number of bins stored in the underlying vector of bin edges. Equal to size() - 1.
  		 */
		int nBins(){
			int _nBins = bins.size() - 1;
			if ( _nBins < 0 ) _nBins = 0;
			return _nBins;
		}
		
		int nBinEdges() {
			return bins.size();
		}

		/* Constructor : fixed width bins
		 *	Creates bins with a nominal bin width. The final bin may be smaller than the nominal size if width does not evenly divide the range
		 */
		HistoBins( double min, double max, double width ){
			this->bins = makeFixedWidthBins( width, min, max );
		}

		HistoBins() {
			
		}

		/* Creates a HistoBins object from a vector of bin edges
		 *
		 */
		HistoBins( vector<double> bins ){
			this->bins = bins;
            width = -1;
		}

		/*Creates histogram bins from an xml config node 
		 * Used heavily by HistoBook for config based histogram creation. 
		 *
		 * Example node:
		 * ``` xml
		 *  <Bins width="1" min="0" max="800" />
		 *  <Bins nBins="10" min="0" max="800" />
		 *  <Bins>10, 12, 14, 16, 18, 20</Bins>
		 * ```
		 */
		HistoBins( XmlConfig &_config, string _nodePath, string _lm = "" ){
			load( _config, _nodePath, _lm );
		} // Constructor

		void load( XmlConfig &_config, string _nodePath, string _lm = "" ) {
			DEBUG( classname(), "(" << _config.getFilename() << ", " << _nodePath << ", " << _lm << " )"  );
			// return;

			// get the bins as an array of edges
			if ( _config.exists( _nodePath ) && _config.getDoubleVector( _nodePath ).size() >= 2 ){
				bins = _config.getDoubleVector( _nodePath );
				min = bins[ 0 ];
				max = bins[ nBins() ];
				width = -1;	// not loading from fixed width
				numberofBins = -1; // not loading from nbins
				DEBUG( classname(), "Found vector of bin edges @ " << _nodePath );
				return;
			}  

			string wt = ":width";
			string mint = ":min";
			string maxt = ":max";
			string nt = ":nBins";

			// Specail case : for modifier "x" also allow no modifier - this makes th histobook able to make 1D histos without a modifier in the xml
			// skip for others so that we dont make the same bins for other modifiers
			if ( "" == _lm || "x" == _lm ){
				getValuesFromConfig( _config, _nodePath, wt, nt, mint, maxt );
				if ( goodValues() ){
					DEBUG( classname(), "Found HistoBins @ " << _nodePath << " with " << wt << nt << mint << maxt );
					if ( width > 0 ){
						bins = makeFixedWidthBins( width, min, max );
						max = bins[ nBins() ];
					}
					else 
						bins = makeNBins( numberofBins, min, max );
					return;
				}
			}

			// something like "width_y, max_y, min_y, nBins_y"
			string mml = "_" + _lm;
			getValuesFromConfig( _config, _nodePath, wt + mml, nt + mml, mint + mml, maxt + mml );
			if ( goodValues() ){
				DEBUG( classname(), "Found HistoBins @ " << _nodePath << " with " << wt + mml << nt + mml << mint + mml << maxt + mml );
				if ( width > 0 ){
					bins = makeFixedWidthBins( width, min, max );
					max = bins[ nBins() ];
				}
				else 
					bins = makeNBins( numberofBins, min, max );
				return;
			}
			
			
			TRACE( classname(), "Could not make HistoBins @ " << _nodePath );
		}


		


		/* Gets a bin edge from the underlying vector of bin edges"
		 * @nIndex the index of the bin edge to get
		 */
		double& operator[] ( const int nIndex ){
			return bins[ nIndex ];
		}

		

		/* Converts a single bin to a string representation
		 * @_binIndex 	index of the bin to report
		 */
		string toString( int _binIndex, BinEdge includeEdge = BinEdge::lower  ){
			string str = "";
			string close = "";
			if ( includeEdge == BinEdge::lower ){
				str += "["; close = ")";
			} else if ( includeEdge == BinEdge::upper ){
				str += "("; close = "]";
			} 

			if ( _binIndex >= 0 && _binIndex < nBins() ){
				return str + dts( bins[ _binIndex ] ) + " --> " + dts( bins[ _binIndex + 1 ] ) + close;
			}
			return "()";
		}

		/* Converts the bin edges into a human readable string
		 * @return A string represenation of the bin edges.
		 */
		string toString() {
			if ( numberofBins > 0 )
				return "< " + ts( nBins() ) + " bins ( " + ts(min) + "->" + ts(max) + " )  >";
			else if ( width > 0 )
				return "< " + ts( nBins() ) + ", width=" + dts(width) + ", bins ( " + ts(min) + "->" + ts(max) + " )  >";
			else {
				string ba = "< " + ts( nBins() ) + " bins { ";

				for ( unsigned int i = 0; i < bins.size(); i++  ){
					if ( i+1  < bins.size() )
						ba += ("(" + dts( bins[i] ) +"->" + dts( bins[i+1] ) + ")" );
					if ( i+2  < bins.size() )
						ba += ", ";
				}
				ba += " } >";
				return ba;
			}

			return "";
		}

		/* Destructor
		 *
		 */
		~HistoBins(){}
		
		// Returns a copy of the vector of bin edges
		vector<double> getBins(){ return bins; }
		
		// TODO: make the bin edges protected
		// Vector of bin edges
		vector<double> bins;	

		unordered_map<int, int> binIndexLUT;
		unordered_map<int, double> binWidthLUT;
		bool useLUT = false;
		double smallestBinWidth;
		double transformedMin;
		double transformedMax;
		static int transformToInt( double param, double value ){
			int out = (int)((value / param ) + 0.5);
			return out;
		}
		static double transformFromInt( double param, int i ){
			double out = (double)i * param;
			return out;
		}

		int transformToInt( double val ){
			return transformToInt( smallestBinWidth, val );
		}
		double transformFromInt( int i ){
			return transformFromInt( smallestBinWidth, i );
		}


		void makeLUTs(){
			smallestBinWidth = numeric_limits<double>::max();
			for ( int i = 0; i < bins.size(); i++ ){
				double bw = binWidth( i );
				if ( bw > 0 && bw < smallestBinWidth ) smallestBinWidth = bw;
			}

			min = bins[0];
			max = bins[ bins.size() - 1];
			INFO( classname(), "Smallest Bin Width = " << smallestBinWidth );
			smallestBinWidth = smallestBinWidth / 5.0;
			transformedMin = transformToInt( smallestBinWidth, min );
			transformedMax = transformToInt( smallestBinWidth, max );

			
			for ( int i = transformedMin - 1; i <= transformedMax + 1; i++ ){
				double tv = transformFromInt( smallestBinWidth, i );
				
				int iBin = findBin( tv );
				DEBUG( classname(), "int i = " << i << " -> " << tv << ", iBin=" << iBin );
				binIndexLUT[ i ] = iBin;
				if ( iBin >= 0 )
					binWidthLUT[ i ] = binWidth( iBin );
			}
			INFO( classname(), "LUT::size=" << binIndexLUT.size() << " == nBins=" << nBins()  );
			useLUT = true;
		}

		double minimum() {
			return min;
		}
		double maximum() {
			return max;
		}

protected:
		void getValuesFromConfig( XmlConfig &config, string &nodePath, 
			string widthTag =":width", string nBinsTag=":nBins", string minTag = ":min", string maxTag=":max" ){

			DEBUG( classname(), nodePath );
			DEBUG( classname(), widthTag << " " << nBinsTag << " " << minTag << " " << maxTag );
			min = 1;
			max = 0;
			width = 0.0;

			min = config.getDouble( nodePath + minTag, min );
			max = config.getDouble( nodePath + maxTag, max );
			width = config.getDouble( nodePath + widthTag, width );



			if ( config.exists( nodePath + nBinsTag ) ){
				numberofBins = config.getInt( nodePath + nBinsTag );
				width = -1;
				// width = ( max - min ) / (double)n;
			}
			DEBUG( classname(), "min=" << min << ", max=" << max << ", width=" << width );
		}

		bool goodValues(){
			if ( max > min && (width > 0 || numberofBins > 0 ) )
				return true;
			return false;
		}

		
		// Width of bins if fixed with or fixed number of bins is used
		double width = -1;
		int numberofBins = -1;
		// min and max of bin range
		double min = -1, max = -1;

	};

}






#endif