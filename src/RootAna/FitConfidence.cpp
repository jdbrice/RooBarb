#include "FitConfidence.h"


namespace jdb{
	TH1 * FitConfidence::fitCL( TF1 * f, string name, double cl, int nPoints, double x1, double x2 ){

		// calculate instead
		if ( -1.0 == x1  && -1.0 == x2 )
			f->GetRange( x1, x2 );
		INFO( FitConfidence::classname(), "Range (" << x1 <<", " << x2 << " )" );
		
		if ( nPoints < 2 )
			nPoints = 100;
		INFO( FitConfidence::classname(), "Plotting with " << nPoints << plural( nPoints, " point", " points" ) );

		// histo to hold CL
		TH1F * hCL = new TH1F( name.c_str(), name.c_str(), nPoints, x1, x2 ); 
		(TVirtualFitter::GetFitter())->GetConfidenceIntervals(hCL, cl);

		return hCL;
	}

	TGraphAsymmErrors * FitConfidence::fitUncertaintyBand( TF1* f, double e_high, double e_low, int nPoints, double x1, double x2 ){
		// calculate instead
		if ( -1.0 == x1  && -1.0 == x2 )
			f->GetRange( x1, x2 );
		INFO( FitConfidence::classname(), "Range (" << x1 <<", " << x2 << " )" );
		if ( nPoints < 2 )
			nPoints = 100;
		INFO( FitConfidence::classname(), "Plotting with " << nPoints << plural( nPoints, " point", " points" ) );
		TGraphAsymmErrors *g = new TGraphAsymmErrors( nPoints );

		float step = (x2 - x1) / (float)nPoints;
		for ( int i = 0; i < nPoints; i++ ){
			float x = step * (i+1);
			float y = f->Eval( x );

			g->SetPoint( i, x, y );
			g->SetPointEYlow( i, e_low );
			g->SetPointEYhigh( i, e_high );
		}
		return g;
	}

	void FitConfidence::calcCholesky( int nP, double * fCov, double* fCovSqrt ){

		double *C = fCovSqrt;
		double *V = fCov;

		// calculate sqrt(V) as lower diagonal matrix
		for( int i = 0; i < nP; ++i ) {
			for( int j = 0; j < nP; ++j ) {
				C[i*nP+j] = 0;
			}
		}

		for( int j = 0; j < nP; ++j ) {
			// diagonal terms first
			double Ck = 0;
			for( int k = 0; k < j; ++k ) {
				Ck += C[j*nP+k] * C[j*nP+k];
			} // k
			C[j*nP+j] = sqrt( fabs( V[j*nP+j] - Ck ) );

			// off-diagonal terms
			for( int i = j+1; i < nP; ++i ) {
				Ck = 0;
				for( int k = 0; k < j; ++k ) {
					Ck += C[i*nP+k] * C[j*nP+k];
				} //k
				if( C[ j * nP + j ] != 0 ) 
					C[ i * nP + j ] = ( V[ i * nP + j ] - Ck ) / C[ j * nP + j ];
				else 
					C[ i * nP + j ] = 0;
			}// i
		} // j
	} // calcCholesky


	double FitConfidence::randomSqrtCov( double xx, TF1 * f, int nP, double * fCovSqrt ){
		double * z = new double[nP];
		double * x = new double[nP];
		double * p = new double[nP];

		for( int i = 0; i < nP; ++i ) {
			z[i] = gRandom->Gaus( 0.0, 1.0 );
			p[i] = f->GetParameter(i);
		}

		for( int i = 0; i < nP; ++i ) {
			x[i] = 0;
			for( int j = 0; j <= i; ++j ) {
				x[i] += fCovSqrt[i*nP+j] * z[j];
			} // j
		}

		for( int i = 0; i < nP; ++i ) {
			f->SetParameter( i, x[i] + p[i] );
		}

		double value = f->Eval(xx);
		for( int i = 0; i < nP; ++i ) {
			f->SetParameter( i, p[i] );
		}

		delete [] z;
		delete [] x;
		delete [] p;
		return value;
	}


	double FitConfidence::choleskyUncertainty( double xx, TFitResultPtr fitResult, TF1 * f, int nSamples ){
		int nP = f->GetNpar();

		TMatrixDSym cov = fitResult->GetCovarianceMatrix();
		double *covArray = new double[ nP * nP ]; // number of parameters x number of parameters
		covArray = cov.GetMatrixArray();

		return choleskyUncertainty( xx, covArray, f, nSamples );
	}


	double FitConfidence::choleskyUncertainty( double xx, double * fCov, TF1 * f, int nSamples ){
		int nP = f->GetNpar();
		INFO( FitConfidence::classname(), "Num Params : " << nP  );
		
		double *fCovSqrt = new double[ nP * nP ];
		calcCholesky( nP, fCov, fCovSqrt );

		double yerr = 0;

		TH1D *hDistributionAtX = new TH1D("hDistributionAtX","",200,f->Eval(xx) - .2,f->Eval(xx) + .2);
		
		for (int n = 0; n < nSamples; n++ ) {
			double val = randomSqrtCov(xx,f,nP,fCovSqrt);
			hDistributionAtX->Fill( val );
		}
		yerr = hDistributionAtX->GetRMS();
		hDistributionAtX->Delete();

		return yerr;

	}

	TGraphErrors *FitConfidence::choleskyBands( TFitResultPtr fitResult, TF1 * f, int nSamples, int nPoints, Reporter* rp,  double x1, double x2 ){

		int nP = f->GetNpar();
		// INFO( FitConfidence::classname(), "Num Params : " << nP  );
		TMatrixDSym cov = fitResult->GetCovarianceMatrix();
		double *covArray = new double[ nP * nP ]; // number of parameters x number of parameters
		covArray = cov.GetMatrixArray();

		// for ( int i = 0; i < 9; i++ ){
		// 	INFO( FitConfidence::classname(), "[" << i << "] = " << covArray[ i ] );
		// }

		double *fCov = new double[ nP * nP ];
		fCov = cov.GetMatrixArray();
		double *fCovSqrt = new double[ nP * nP ];
		calcCholesky( nP, fCov, fCovSqrt );

		// for ( int i = 0; i < 9; i++ ){
		// 	INFO( FitConfidence::classname(), "[" << i << "] = " << fCovSqrt[ i ] );
		// }

		

		// calculate instead
		if ( -1.0 == x1  && -1.0 == x2 )
			f->GetRange( x1, x2 );

		double step = ( x2 - x1 ) / (double) nPoints;

		double x[ nPoints + 1 ];
		double y[ nPoints + 1 ];
		// double yup[ nPoints + 1 ];
		// double ydown[ nPoints + 1 ];
		double yerr[ nPoints + 1 ];
		vector<double> samples;

		int i = 0;
		for (double xx = x1; xx < x2; xx+= step) {
			x[i] = xx;
			TH1D *hDistributionAtX = new TH1D("hDistributionAtX","",200,f->Eval(x[i]) - .2,f->Eval(x[i]) + .2);
			for (int n = 0; n < nSamples; n++ ) {
				double val = randomSqrtCov(x[i],f,nP,fCovSqrt);
				hDistributionAtX->Fill( val );
				samples.push_back( val );
			}
			//hDistributionAtX->DrawCopy();

			y[i] 		= f->Eval(x[i]);
			yerr[i] 	= hDistributionAtX->GetRMS();
			
			// cross check - should always give the same result
			// but very innefficient
			//yerr[i] = choleskyUncertainty( xx, fitResult, f, nSamples );

			// rp.savePage();
			hDistributionAtX->Delete();
			i++;
		} 

		TGraphErrors * g = new TGraphErrors( i - 1, x, y, 0, yerr );

		return g;
	}
}// jdb