#ifndef JDB_XML_FUNCTION_H
#define JDB_XML_FUNCTION_H

// ROOBARB
#include "XmlConfig.h"
#include "Utils.h"
#include "IObject.h"

// STL
#include <memory>

// ROOT
#include "TF1.h"


namespace jdb{

	/* Loads a TF1 or subclass function from an Xml Config
	 *
	 * Loads a function from an XmlConfig.
	 */
	class XmlFunction : public IObject { 
	protected:
		// The ROOT TF1 object backing the function
		shared_ptr<TF1> func = nullptr;
		vector<double> cov;

		map<string, int> pNameToIndex;
		map<int, string> pIndexToName;

		// instance count to make sure wee keep a unique ROOT name on our function
		static int instances;
	public:
		virtual const char * classname() const { return "XmlFunction"; }

		/* Creates a function from xml config
		 *
		 * @cfg 		XmlConfig object
		 * @nodePath 	path to node
		 * @pMod 		Default : ':p' - the modifier to get the parameter from node path
		 * @eMod 		Default : ':e' - the modifier to get the parameter error from node path
		 * @formMod 	Default : ':formula' - the modifier to get the formula from node path
		 */
		XmlFunction( XmlConfig * cfg, string nodePath, string pMod = ":p", string eMod = ":e", string formMod = ":formula", string covMod = ":cov", string nameMod=":name");
		XmlFunction( XmlConfig &cfg, string nodePath, string pMod = ":p", string eMod = ":e", string formMod = ":formula", string covMod = ":cov", string nameMod=":name");
		XmlFunction();
		void set( XmlConfig &cfg, string nodePath, string pMod = ":p", string eMod = ":e", string formMod = ":formula", string covMod = ":cov", string nameMod=":name");
		/* Destructor
		 *
		 */
		~XmlFunction() {}


		void setParameter( string name, double value ){
			if ( pNameToIndex.count( name ) > 0 && pNameToIndex[ name ] >= 0 )
				func->SetParameter( pNameToIndex[ name ], value );
			else {
				ERROR( classname(), "Cannot set parameter : " << quote( name ) );
			}
		}

		/* Evaluate the function
		 * Checks for the valid state of function before eval
		 * 
		 * @x 		dependent variable in function
		 * @return 	evaluated value of function or 0.0 if function is undefined
		 */
		double eval( double x ){
			if ( func != nullptr ){
				return func->Eval( x );
			}
			return 0.0;
		}

		shared_ptr<TF1> getTF1() { return func; }

		vector<double> getCov() const { return cov; }

		vector<string> getParameterNames() {
			vector<string> names;
			for ( auto kv : pNameToIndex ) {
				names.push_back( kv.first );
			}
			return names;
		}

		double getParameter( int i ){
			return func->GetParameter( i );
		}
		double getParameter( string name ){
			if ( pNameToIndex.count( name ) > 0 && pNameToIndex[ name ] >= 0 ){
				return func->GetParameter( pNameToIndex[ name ] ) ;
			}
			return std::numeric_limits<double>::quiet_NaN();
		}

		string toString(){

			string line = (string)func->GetTitle() + " : ( ";
			for ( int i = 0; i < func->GetNpar(); i++ ){
				if ( pIndexToName.count( i ) > 0 ){
					line += pIndexToName[i] + "=";
				}
				line += dts(func->GetParameter( i ));
				if ( i != func->GetNpar() - 1 )
				 line += ", ";
			}
			line += " ) ";
			return line;
		}

		static string bracketParamIndex( int _p ){
			return "[" + ts(_p) + "]";
		}
		static int unbracketParamIndex( string _p ){
			// expect "[NNN]"
			string::size_type start = _p.find( "[" );
			string::size_type stop = _p.find( "]" );
			return atoi(_p.substr( start+1, (int)(stop - start - 1)).c_str()  );
		}

		static bool isBracketedParamIndex( string _p ){
			if ( _p.find( "[" ) != string::npos && _p.find( "]" ) != string::npos )
				return true;
			return false;
		}

		static string toXml( TF1 * f, map<string, string> _opts = {{ "", "" }} ) {

			string line = "<XmlFunction";

			// now add the optional attributes passed in
			for ( auto kv : _opts ){
				if ( "" == kv.first ) continue;
				line += " " + kv.first + "=" + quote( kv.second );
			}


			line += " formula=\"";
			line += f->GetTitle();
			line += "\"";

			for ( int ip = 0; ip < f->GetNpar(); ip++ ){

				line += " p" + ts(ip) + "=\"";
				line += dts(f->GetParameter(ip));
				line += "\"";

				line += " e" + ts(ip) + "=\"";
				line += dts(f->GetParError(ip));
				line += "\"";
			}

			


			line += " />";
			return line;
		}


		string toXml(){
			string line = "<XmlFunction ";
			line += "formula=\"";
			line += func->GetTitle();
			line += "\"";

			for ( int ip = 0; ip < func->GetNpar(); ip++ ){

				line += " p" + ts(ip) + "=\"";
				line += dts(func->GetParameter(ip));
				line += "\"";

				line += " e" + ts(ip) + "=\"";
				line += dts(func->GetParError(ip));
				line += "\"";

				line += " p" + ts(ip) + "Name=\"";
				line += pIndexToName[ ip ];
				line += "\"";
			}

			line += " />";
			return line;
		}



#ifdef __CINT__
		ClassDef( jdb::XmlFunction, 1 )
#endif
	};

}


#endif