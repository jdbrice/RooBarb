#ifndef ROOBARB_FIT_CONFIDENCE_H
#define ROOBARB_FIT_CONFIDENCE_H

#include "TH1.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMatrixDSym.h"
#include "TFitResult.h"
#include "TRandom.h"
#include "TVirtualFitter.h"

#include <string>
using namespace std;

#include "Reporter.h"
#include "Utils.h"


namespace jdb {

	/* Utilities for calculating and drawing fit confidence levels
	 *
	 */
	class FitConfidence
	{
	public:
		// Classname used in the static functions
		// mirror of the IObject version
		static const char* classname() { return "FitConfidence"; }
		
		// Unused - static class
		FitConfidence(){}
		// Unused - static class
		~FitConfidence(){}


		/* Draws the given function with its 95% confidence level and returns the histogram containing the confidence level region
		 * @f 			Function to draw
		 * @nPoints 	Number of points in graph, default = 100
		 * @cl 			Confidence level to use as fraction of 1.0, default = 0.95 ( 95% )
		 * @name 		Name of histogram storing results
		 * @x1 			low range of function
		 * @x2 			high range of function, if x1 == x2 == -1 then the fit range is used
		 *
		 * @return 	TH1 * for region of confidence 
		 */
		static TH1 * fitCL( TF1 * f, string name, double cl = 0.95, int nPoints = 100, double x1 = -1.0, double x2 = -1.0 );

		/* Computes the Cholesky parameters for the given function
		 * @nP 			Number of parameters
		 * @fCov 		Covariance matrix - passed in
		 * @fCovSqrt	Sqrt(Cov) matrix - passed out
		 *
		 */
		static void calcCholesky( int nP, double * fCov, double* fCovSqrt );

		/* Calculates random variations in a function from the sqrt of the cov matrix
		 * @xx 			x value at which to evaluate function
		 * @f 			TF1 * to function
		 * @nP 			number of parameters
		 * @fCovSqrt 	sqrt(cov) given by calcCholesky
		 *
		 * @return 		function evaluated within a random gaussian distribution at the given point
		 */
		static double randomSqrtCov( double xx, TF1 * f, int nP, double * fCovSqrt );

		/* Calculates the Cholesky Uncertainty at a given point for a given function
		 * @xx 			value at which the uncertainty should be calculated
		 * @fitResult 	Fit result pointer
		 * @f 			TF1 Fit pointer
		 * @nSamples 	Number of samples to use
		 *
		 * @return 	The symmetric uncertainty at the value xx
		 */
		static double choleskyUncertainty( double xx, TFitResultPtr fitResult, TF1 * f, int nSamples );

		/* Calculates the Cholesky Uncertainty at a given point for a given function
		 * @xx 			value at which the uncertainty should be calculated
		 * @fCov 		covariance matrix
		 * @f 			TF1 Fit pointer
		 * @nSamples 	Number of samples to use
		 *
		 * @return 	The symmetric uncertainty at the value xx
		 */
		static double choleskyUncertainty( double xx, double * fCov, TF1 * f, int nSamples );


		/* Calculate and draw Cholesky uncertainty bands for the given function
		 * @fitResult		Fit Result pointer
		 * @f 				TF1 Fit pointer
		 * @nSamples 		Number of samples to use, default = 50
		 * @nPoints 		Number of points in graph, default = 100
		 * @Reporter 		pointer to a reporter to use for saving reports
		 * @x1 				low range of function
		 * @x2 				high range of function, if x1 == x2 == -1 then the fit range is used
		 *
		 * @return 			TGraphErrors containg the uncertainties
		 */
		static TGraphErrors *choleskyBands( TFitResultPtr fitResult, TF1 * f, int nSamples = 50, int nPoints = 100, Reporter * rp = nullptr, double x1 = -1.0, double x2 = -1.0 );
		
	};
}// namespace


#endif