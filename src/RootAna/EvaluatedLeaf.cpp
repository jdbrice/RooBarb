#include "DataSource.h"

namespace jdb{

	EvaluatedLeaf::EvaluatedLeaf( XmlConfig * _cfg, string _nodePath ){

		lg.info(__FUNCTION__) << "EvaluatedLeaf( XmlConfig, " << _nodePath << " )" << endl;
		cfg = _cfg;
		nodePath = _nodePath;

		string name = cfg->getString( nodePath + ":name" );
		string value = cfg->getString( nodePath + ":value" );



		fun = new TF1( "cvf", value.c_str(), -10, 10 );

		lg.info( __FUNCTION__ ) << name << " = " << value << " : ( ";
		for ( int i = 0; i < 10; i ++ ){
			if ( cfg->exists( nodePath + ":p" + ts( i ) ) ){
				lg.info("", false ) << "[" << i << "] = " << cfg->getString( nodePath + ":p" + ts( i ) ) << "  ";
				params.push_back( cfg->getString( nodePath + ":p" + ts( i ) ) );
			}
		}
		lg.info( "", false ) << " ) " << endl;
	}

	double EvaluatedLeaf::eval( DataSource * ds, int iTrack ){

		for ( unsigned int i = 0; i < params.size(); i ++ ){
			double p = ds->get( params[ i ], iTrack );
			fun->SetParameter( i, p );
		}
		return fun->Eval( 0 );
	}

}