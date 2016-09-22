#ifndef ROOBARB_XML_POINT_H
#define ROOBARB_XML_POINT_H


// RooBarb
#include "XmlConfig.h"
#include "Utils.h"
#include "IObject.h"
using namespace jdb;


#include <memory>


namespace jdb{
	
	/*Config Point easily loads (x, y) points from XmlConfigs
	 *
	 * This class is a utility class used for quickly loading and aliasing point data in an xml config
	 *
	 * **Headers** : XmlPoint.h
	 *
	 * XML should contain a node like:
	 * ``` xml
 	 * <Point x="100" y="100" />
	 * ```
	 * 
	 */
	class XmlPoint  : public IObject {
	public:
		virtual const char * classname() const { return "XmlPoint"; }

		// x value
		double x;
		// y value
		double y;

	
		/*Load point data from an XmlConfig
		 *@cfg XmlConfig instance to load data from
		 *@nodePath Path to the XmlNode containg point data
		 *@dX Default = 0 : default x value if not found in node
		 *@dY Default = 0 : default y value if not found in node
		 *@attrX Default = ":x" : attribute tag to search for x value
		 *@attrY Default = ":y" : attribute tag to search for y value
		 *
		 * This class reads in data from an XmlConfig instance. The underlying xml should contain a node at *nodePath* like 
		 * ``` xml
		 * <Point x="100" y="100" />
		 * ```
		 */
		XmlPoint( XmlConfig * cfg, std::string nodePath, double dX = 0, double dY = 0, string attrX = ":x", string attrY = ":y"){
			set( (*cfg), nodePath, dX, dY, attrX, attrY );
		}

		XmlPoint( XmlConfig &cfg, std::string nodePath, double dX = 0, double dY = 0, string attrX = ":x", string attrY = ":y"){
			set( cfg, nodePath, dX, dY, attrX, attrY );
		}

		XmlPoint() {
			set( 0.0, 0.0 );
		}


		~XmlPoint(){}

		/* From Config
		 * 
		 */
		void set( XmlConfig &config, std::string nodePath, double dX = 0, double dY = 0, string attrX = ":x", string attrY = ":y" ) {
			
			if ( config.exists( nodePath ) ){
				x = config.getDouble( nodePath + attrX, dX );
				y = config.getDouble( nodePath + attrY, dY );
			} else {
				x = dX;
				y = dY;
			}
		}

		void set( double _x, double _y ){
			x = _x;
			y = _y;
		}

		/* Outputs the point as a human readable string
		 *
		 * @return A string representation of the point data in the format "( x, y )"
		 */
		string toString(){
			return ("( " + dts( x ) + ", " + dts( y ) + " )");
		}

		string toXml( string _nodeName ="XmlPoint" ){
			return "<" + _nodeName + " x=\"" + dts( x ) + "\" y=\"" + dts( y ) + "\" />";
		}

#ifdef __CINT__
		ClassDef( jdb::XmlPoint, 1 )
#endif
	};

}




#endif