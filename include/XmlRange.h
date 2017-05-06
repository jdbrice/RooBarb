#ifndef JDB_XML_RANGE_H
#define JDB_XML_RANGE_H

#include "XmlConfig.h"
#include "Utils.h"
#include "IObject.h"
using namespace jdb;

#include <limits>       // std::numeric_limits
#include <memory>
using namespace std;

namespace jdb{

	/*XmlRange easily loads (min -> max) ranges from XmlConfigs
	 *
	 * This class is a utility class used for quickly loading and aliasing range data in an xml config
	 *
	 * **Headers** : XmlRange.h
	 *
	 * ``` xml
	 * <Range min="100" max="100" />
	 * ```
	 */
	class XmlRange : public IObject
	{
	public:
		virtual const char* classname() const { return "XmlRange"; }
		//Minimum value in range
		double min;
		//Maximum value in range
		double max;

		/*Load range data from an XmlConfig
		 *@cfg XmlConfig instance to load data from
		 *@nodePath Path to the XmlNode containg range data
		 *@dMin Default = 0 : default x value if not found in node
		 *@dMax Default = 0 : default y value if not found in node
		 *@attrMin Default = ":min" : attribute tag to search for x value
		 *@attrMax Default = ":max" : attribute tag to search for y value
		 *
		 * This class reads in data from an XmlConfig instance. The underlying xml should contain a node at *nodePath* like 
		 * ``` xml
		 * <Range name="danny" min="100" max=":100" />
		 * ```
		 */
		XmlRange( XmlConfig * _cfg, string _nodePath, double dMin = 0, double dMax = 0, string attrMin = ":min", string attrMax = ":max"){
			loadConfig( *_cfg, _nodePath, dMin, dMax, attrMin, attrMax );
		}

		XmlRange( XmlConfig &_cfg, string _nodePath, double dMin = 0, double dMax = 0, string attrMin = ":min", string attrMax = ":max"){
			loadConfig( _cfg, _nodePath, dMin, dMax, attrMin, attrMax );
		}

		/* Default empty constructor
		 * initializes min and max to 0
		 */
		XmlRange( ) {
			min = 0;
			max = 0;
		}
		~XmlRange(){}

		void loadConfig( XmlConfig &_cfg, string np, double dMin = 0, double dMax = 0, string attrMin = ":min", string attrMax = ":max" ){
			min = _cfg.getDouble( np + attrMin, dMin );
			max = _cfg.getDouble( np + attrMax, dMax );
		}

		/* Outputs the range as a human readable string
		 *
		 * @return A string representation of the range data in the format "( min -> max )"
		 */
		string toString(){
			if ( std::numeric_limits<double>::max()  == max && std::numeric_limits<double>::lowest() == min ){
				return ( "( -inf -> inf )" );
			} else if ( std::numeric_limits<double>::max()  == max )
				return ("( " + dts( min ) + " -> inf )");
			else if ( std::numeric_limits<double>::lowest() == min )
				return ("( -inf -> " + dts( max ) + " )");
			else		
				return ("( " + dts( min ) + " -> " + dts( max ) + " )");
		}

		/* Test if the given value is in the inclusve range
		 * Test if value is in [ min, max] inclusive lower and upper bound
		 * @val 	Value to test
		 * @return 	true if in range, false otherwise
		 */
		bool inInclusiveRange( double val ){

			if ( val > max || val < min )
				return false;

			return true;
		}

		/* Test if the given value is in the exclusive range
		 * Test if value is in ( min, max ) exclusive lower and upper bound
		 * @val 	Value to test
		 * @return 	true if in range, false otherwise
		 */
		bool inExclusiveRange( double val ){

			if ( val >= max || val <= min )
				return false;

			return true;
		}

		/* Test if the given value is in the exclusive lower inclusive upper range
		 * Test if value is in ( min, max ] exclusive lower and inclusive upper bound
		 * @val 	Value to test
		 * @return 	true if in range, false otherwise
		 */
		bool inExclusiveLowerInclusiveUpperRange( double val ){

			if ( val > max || val <= min )
				return false;

			return true;
		}

		/* Test if the given value is in the inclusive lower exclusive upper range
		 * Test if value is in [ min, max ) inclusive lower and exclusive upper bound
		 * @val 	Value to test
		 * @return 	true if in range, false otherwise
		 */
		bool inInclusiveLowerExclusiveUpperRange( double val ){

			if ( val >= max || val < min )
				return false;

			return true;
		}

		/* Is value above max
		 * Test to see if value is above upper range
		 *
		 * @val 	Value to test
		 * @includeUpperEdge Default: true - include or exclude upper edge
		 * @return 	true if past the upper edge
		 */
		bool above( double val, bool includeUpperEdge = true ){
			if ( val > max )
				return true;
			if ( includeUpperEdge && val == max )
				return true;
			return false;
		} 

		/* Is value below min
		 * Test to see if value is below lower range
		 *
		 * @val 	Value to test
		 * @includeLowerEdge Default: true - include or exclude lower edge
		 * @return 	true if below the lower edge
		 */
		bool below( double val, bool includeLowerEdge = true ){
			if ( val < min )
				return true;
			if ( includeLowerEdge && val == min )
				return true;
			return false;
		} 
#ifdef __CINT__
		ClassDef( jdb::XmlRange, 1)
#endif
	};

}

#endif