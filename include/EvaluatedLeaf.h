#ifndef EVALUATED_LEAF_H
#define EVALUATED_LEAF_H

#include "XmlConfig.h"
#include "Logger.h"
#include "Utils.h"
using namespace jdb;

#include "TF1.h"
#include <string>
using namespace std;

#include "TF1.h"



namespace jdb{

	class DataSource;

	/* DataSource leaf evaluated from a function string
	 *
	 * Allows a DataSource to seamlessly provide access to data computed from other leaves
	 * For instance, suppose a tree offers vertexX and vertexY values but the vertexR is not provided.
	 * An evaluated leaf such as:
	 * ``` xml
	 * 	<DataSource ...>
	 * 		<EvaluatedLeaf name="vertexR" value="TMath::Sqrt( [0]*[0] + [1]*[1] )" p0="vertexX" p1="vertexY"/>  
	 * 	</DataSource>
	 * ```
	 * Provides runtime access for DataSource ds to ds.get( "vertexR" );
	 * 
	 */
	class EvaluatedLeaf{

	protected:
		// The Function used to evaluate the given expression 
		TF1 * fun;
		// The XML config
		XmlConfig * cfg;
		// Node Path
		string nodePath;
		// Logger instance
		Logger lg;
		// Name of the leaves used as parameters
		vector<string> params;
	public:

		/* Creates a calculated variable from existing ones
			@tm TreeMap of data
			@cfg An XmlConfig containg data
			@nodePath Path to node containing a "<EvaluatedLeaf />" node
		*/
		EvaluatedLeaf( XmlConfig * _cfg, string _nodePath );
		~EvaluatedLeaf() {
			delete fun;
		};

		/**
		 * Evaluates the calculated value of the leaf
		 * @return the evaluated value in double format
		 */
		double eval( DataSource * ds, int i = 0 );

	protected:


	};


}

#endif