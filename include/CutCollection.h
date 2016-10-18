#ifndef CUT_COLLECTION_H
#define CUT_COLLECTION_H

#include "XmlConfig.h"
#include "Utils.h"
#include "XmlRange.h"
#include "XmlPoint.h"
#include "Logger.h"


#include "IConfig.h"
#include "IObject.h"


#include <memory>
using namespace jdb;

namespace jdb{

	/* Collection of range cuts
	 * Provides a collection of range cuts in a lookup table by name
	 */
	class CutCollection : public IConfig, public IObject {
	public:

		/* @inherit */
		virtual const char* classname() const { return "CutCollection"; }
		/* Default 
		*/
		CutCollection() {};
		/* Create from config path
		 * Creates all cuts <Range> children of "_nodePath"
		 *
		 * @_config XmlConfig
		 * @_nodePath path to Ranges
		 */
		CutCollection( XmlConfig _config, string _nodePath ){ init( _config, _nodePath ); }
		/* Default 
		*/
		~CutCollection(){}

		// Map that holds range cuts
		map< string, shared_ptr<XmlRange> > ranges;


		/* Find out if a named range exists
		 *
		 */
		bool has( string name ){
			if ( ranges.count( name ) > 0 )
				return true;
			return false;
		}

		/* Get a range cut
		 * 
		 * @name the name of the cut to get
		 * @return XmlRange object
		 */
		shared_ptr<XmlRange> operator[]( string name ) {
			if ( ranges.count( name ) > 0 )
				return ranges[ name ];

			ERROR( classname(), name << " DNE" );
			return nullptr;
		} // operator[]


		/* Reports the cut names and ranges
		 *
		 */
		void report() {
			for ( auto k : ranges ){
				INFO( classname(), k.first << " : " << k.second->toString() );
			}
		} // report

		void init( XmlConfig _config, string _nodePath ){

			vector<string> children = _config.childrenOf( _nodePath, "Range" );

			for ( string p : children ){
				

				if ( _config.exists( p + ":name" ) ){
					string name = _config.getString( p + ":name" );	
					shared_ptr<XmlRange> cr = shared_ptr<XmlRange>( new XmlRange( &_config, p ) );	

					ranges[ name ] = cr;

					DEBUG( classname(), "Added cut range '" << name << "' " );
					DEBUG( classname(), cr->toString() );
				}	
			}
		} // init

		void setDefault( string name, double min, double max ){
			if ( false == has( name ) ){
				ranges[ name ] = shared_ptr<XmlRange>( new XmlRange( ) );
				ranges[name]->min = min;
				ranges[name]->max = max;
				DEBUG( classname(), "Setting Default for[ " << name << " = (" << min << ", " << max << ")" );
			}
		}

	protected:

		
	};
}

#endif