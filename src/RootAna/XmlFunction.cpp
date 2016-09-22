#include "XmlFunction.h"
#include "Utils.h"

namespace jdb {

	int XmlFunction::instances = 0; 

	XmlFunction::XmlFunction(){
	}

	XmlFunction::XmlFunction( XmlConfig * cfg, string nodePath, string pMod, string eMod, string formMod, string covMod ){
		set( *cfg, nodePath, pMod, eMod, formMod, covMod );
	}
	XmlFunction::XmlFunction( XmlConfig &cfg, string nodePath, string pMod, string eMod, string formMod, string covMod ){
		set( cfg, nodePath, pMod, eMod, formMod, covMod );
	}

	void XmlFunction::set( XmlConfig &cfg, string nodePath, string pMod, string eMod, string formMod, string covMod ){

		string formula = cfg.getString( nodePath + formMod );
		func = unique_ptr<TF1> (new TF1( ("xmlfunction_" + ts(instances)).c_str(), formula.c_str() ) );
	 
		int p = 0;
		while ( cfg.exists( nodePath + pMod + ts(p) ) ){
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

		if ( cfg.exists( nodePath + covMod ) ){
			cov = cfg.getDoubleVector( nodePath + covMod );
			DEBUG( classname(), "cov array length : " << cov.size() );
		}
	}

}
