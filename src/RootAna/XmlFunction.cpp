#include "XmlFunction.h"
#include "Utils.h"
#include "XmlString.h"

namespace jdb {

	int XmlFunction::instances = 0; 

	XmlFunction::XmlFunction(){
	}

	XmlFunction::XmlFunction( XmlConfig * cfg, string nodePath, string pMod, string eMod, string formMod, string covMod, string nameMod ){
		set( *cfg, nodePath, pMod, eMod, formMod, covMod, nameMod );
	}
	XmlFunction::XmlFunction( XmlConfig &cfg, string nodePath, string pMod, string eMod, string formMod, string covMod, string nameMod ){
		set( cfg, nodePath, pMod, eMod, formMod, covMod, nameMod );
	}

	void XmlFunction::set( XmlConfig &cfg, string nodePath, string pMod, string eMod, string formMod, string covMod, string nameMod ){

		string name = cfg.getXString( nodePath + nameMod, "xmlfunction_" + ts(instances) );
		
		string ocn = cfg.cn( nodePath + ":" );
			
			string rformula = cfg.getString( formMod.substr( 1 ) );
		

			// Preprocess the formula and add the needed stuffs
			XmlString xstr;
			map<string, int> paramCount;
			int iParam = 0;
			for ( string tk : xstr.tokens( rformula ) ){
				if ( paramCount.count( tk ) > 0 ) continue;

				int cpi = iParam;
				// setup the mapping to the ROOT style parameter index
				// if ( cfg.exists( tk ) && XmlFunction::isBracketedParamIndex( cfg.getString( tk ) )){
				// 	DEBUG( classname(), "Found token " << tk );
				// 	cpi = XmlFunction::unbracketParamIndex( cfg.getString( tk ) );
				// } else {
				
				if ( cfg.exists( tk )){
					// move the VALUE here into the pN one to store the value for the parameter
					INFO( classname(), "Initial value for " << tk << " --> " << "p" << iParam );
					cfg.set( "p" + ts( iParam ), cfg.getString( tk ) );
				}

				
				cfg.set( tk, XmlFunction::bracketParamIndex( iParam ) );
				paramCount[ tk ] ++;
				

				if ( cfg.exists( "p" + ts(cpi) + "Name" ) ){

				} else {
					cfg.set( "p" + ts(cpi) + "Name", tk );
				}
				iParam++;
			}
			// Now get the interpolated string with all the thingz
			string formula = cfg.getXString( formMod.substr( 1 ) );
		cfg.cn( ocn );


		INFO( classname(), "TF1 <\"" << name << "\" = " << formula << "> with " << iParam );
		func = unique_ptr<TF1> (new TF1( name.c_str(), formula.c_str() ) );
	 

	 	// Parameter values
		int p = 0;
		while ( cfg.exists( nodePath + pMod + ts(p) ) || p < iParam ){
			double val = cfg.getDouble( nodePath + pMod + ts(p) );
			
			func->SetParameter( p, val );

			// set errors if they are there
			if ( cfg.exists( nodePath + eMod + ts(p) ) ){
				double error = cfg.getDouble( nodePath + eMod + ts(p) );
				func->SetParError( p, error );

				DEBUG( classname(), "p" << p << " = " << val << " +/- " << error )
			} else {
				DEBUG( classname(), "p" << p << " = " << val )
			}

			p++;
		}

		// Parameter names - may be auto built from keys in formula. If they are explicitly given then thay take precendence
		p = 0;
		while ( cfg.exists( nodePath + pMod + ts(p) + "Name" ) || p < iParam ){
			pNameToIndex[ cfg.getXString( nodePath + pMod + ts(p) + "Name" ) ] = p;
			pIndexToName[ p ] = cfg.getXString( nodePath + pMod + ts(p) + "Name" );
			INFO( classname(), pIndexToName[p] << " = p[" << p << "]");
			p++;
		}

		if ( cfg.exists( nodePath + covMod ) ){
			cov = cfg.getDoubleVector( nodePath + covMod );
			DEBUG( classname(), "cov array length : " << cov.size() );
		}

		if ( cfg.exists( nodePath + ":min" ) && cfg.exists( nodePath + ":max" ) ){
			func->SetRange( cfg.getDouble( nodePath + ":min" ), cfg.getDouble( nodePath + ":max" ) );
		}



	}

}
