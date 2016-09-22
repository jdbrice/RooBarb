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
		XmlFunction( XmlConfig * cfg, string nodePath, string pMod = ":p", string eMod = ":e", string formMod = ":formula", string covMod = ":cov");
		XmlFunction( XmlConfig &cfg, string nodePath, string pMod = ":p", string eMod = ":e", string formMod = ":formula", string covMod = ":cov");
		XmlFunction();
		void set( XmlConfig &cfg, string nodePath, string pMod = ":p", string eMod = ":e", string formMod = ":formula", string covMod = ":cov");
		/* Destructor
		 *
		 */
		~XmlFunction() {}

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

		string toString(){

			string line = (string)func->GetTitle() + " : ( ";
			for ( int i = 0; i < func->GetNpar(); i++ ){
				line += dts(func->GetParameter( i ));
				if ( i != func->GetNpar() - 1 )
				 line += ", ";
			}
			line += " ) ";
			return line;
		}

		static string toXml( TF1 * f ) {

			string line = "<XmlFunction ";
			line += "formula=\"";
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
#ifdef __CINT__
		ClassDef( jdb::XmlFunction, 1 )
#endif
	};

}


#endif