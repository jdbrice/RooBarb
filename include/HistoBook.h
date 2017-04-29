
#ifndef HISTOBOOK_H
#define HISTOBOOK_H

#include <map>
#include <string>
#include <sstream>
#include <stdarg.h>

// JDB
#include "XmlConfig.h"
#include "Logger.h"
#include "HistoBins.h"
// Interface
	#include "IConfig.h"
	#include "IObject.h"


// ROOT 
#include "TROOT.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TLegend.h"
#include "TFile.h"
#include "TStyle.h"
#include "TPad.h"
#include "TClass.h"
#include "TObject.h"


using namespace std;

namespace jdb{


	/* A book keeper and helper for storing and using ROOT histograms
	 *
	 */
	class HistoBook : public IConfig, public IObject {

	protected:

		// Current directory in output file
		string currentDir;
	
		// Map of hitogram names to paths in the Xml Config File for automaticlly created histos
		std::map<string, string> configPath;

		// Map of histogram names to objects
		std::map<string, TObject*> book;

		// Filename of output file
		string filename;

		// filename for input 
		string inputFilename;

		// directory to read in input file
		string inputDir;

		// Output file
		TFile *file;

		// Save on Exit or not
		bool saveAllOnExit;

		TH1 * nullHisto;

	public:
		
		static vector<double> contentVector( TH1* _h ){
			vector<double> c;
			if ( _h == nullptr ) return c;

			TAxis* x = _h->GetXaxis();
			for ( int i = 1; i <= x->GetNbins(); i++ ){
				c.push_back( _h->GetBinContent( i ) );
			}
			return c;
		}

		static vector<double> errorVector( TH1* _h ){
			vector<double> c;
			if ( _h == nullptr ) return c;

			TAxis* x = _h->GetXaxis();
			for ( int i = 1; i <= x->GetNbins(); i++ ){
				c.push_back( _h->GetBinError( i ) );
			}
			return c;
		}

		static vector<double> contentVector( TH2* _h ){
			vector<double> c;
			if ( _h == nullptr ) return c;

			TAxis* x = _h->GetXaxis();
			TAxis* y = _h->GetYaxis();
			for ( int j = 1; j <= y->GetNbins(); j++ ){
				for ( int i = 1; i <= x->GetNbins(); i++ ){
					c.push_back( _h->GetBinContent( i, j ) );
				}
			}
			return c;
		}

		static vector<double> errorVector( TH2* _h ){
			vector<double> c;
			if ( _h == nullptr ) return c;

			TAxis* x = _h->GetXaxis();
			TAxis* y = _h->GetYaxis();
			for ( int j = 1; j <= y->GetNbins(); j++ ){
				for ( int i = 1; i <= x->GetNbins(); i++ ){
					c.push_back( _h->GetBinError( i, j ) );
				}
			}
			return c;
		}

		static void writeVector( TH1* _h, vector<double> _contents, vector<double> _errors = {} ){
			if ( nullptr == _h ) return;
			TAxis* x = _h->GetXaxis();
			for ( int i = 1; i <= x->GetNbins(); i++ ){
				if ( i >= _contents.size() ) break;
				_h->SetBinContent( i, _contents[i-1] );
				if ( i >= _errors.size() ) continue;
				_h->SetBinError( i, _errors[i-1] );
			}
		}

		static void writeVector( TH2* _h, vector<double> _contents, vector<double> _errors = {} ){
			if ( nullptr == _h ) return;
			TAxis* x = _h->GetXaxis();
			TAxis* y = _h->GetYaxis();

			int Nbx = x->GetNbins();
			int Nby = y->GetNbins();
			for ( int j = 1; j <= Nby; j++ ){
				for ( int i = 1; i <= Nbx; i++ ){
					int iBin = _h->GetBin( i, j );
					int iData = i-1 + Nbx * (j-1); 
					if ( iData < _contents.size() )
						_h->SetBinContent( iBin, _contents[iData] );
					if ( iData < _errors.size() )
						_h->SetBinError( iBin, _errors[iData] );
				}
			}
		}


		static TH1* cloneBinRange( TH1 *_h, string name, int bX1=0, int bX2 = -1 ){
			if (nullptr == _h ) return nullptr;
			TH1 * hClone = (TH1*)_h->Clone( name.c_str() );
			TAxis * x = _h->GetXaxis();
			int b1 = bX1;
			if ( b1 < 0 ) b1 = 0;
			int b2 = bX2;
			if (b2 > x->GetNbins() + 1 || -1 == b2) b2 = x->GetNbins() + 1;

			hClone->Reset();

			for ( int iB = b1; iB < b2; iB++ ){
				hClone->SetBinContent( iB, _h->GetBinContent( iB ) );
				hClone->SetBinError( iB, _h->GetBinError( iB ) );
			}
			return hClone;
		}

		static TH1* cloneRange( TH1 *_h, string name, float x1=0, float x2 = -1 ){
			if (nullptr == _h ) return nullptr;
			TAxis * x = _h->GetXaxis();
			int b1 = x->FindBin( x1 );
			int b2 = x->FindBin( x2 );

			return cloneBinRange( _h, name, b1, b2 );
		}

		static void cloneBinRange( TH1 *_hFrom, TH1* _hTo, int bX1=0, int bX2 = -1 ){
			if (nullptr == _hFrom || nullptr == _hTo) return;
			TAxis * x = _hFrom->GetXaxis();
			int b1 = bX1;
			if ( b1 < 0 ) b1 = 0;
			int b2 = bX2;
			if (b2 > x->GetNbins() + 1 || -1 == b2) b2 = x->GetNbins() + 1;

			for ( int iB = b1; iB < b2; iB++ ){
				_hTo->SetBinContent( iB, _hFrom->GetBinContent( iB ) );
				_hTo->SetBinError( iB, _hFrom->GetBinError( iB ) );
			}
		}
		static void cloneRange( TH1 *_hFrom, TH1* _hTo, float x1=0, float x2 = -1 ){
			if (nullptr == _hFrom || nullptr == _hTo ) return;
			TAxis * x = _hFrom->GetXaxis();
			int b1 = x->FindBin( x1 );
			int b2 = x->FindBin( x2 );
			
			cloneBinRange( _hFrom, _hTo, b1, b2 );

			return;
		}

		static TH1* relativeErrors( TH1* _h, string name ){
			if ( nullptr == _h ) return nullptr;
			TH1* hre = (TH1*)_h->Clone( name.c_str() );
			hre->Reset();
			for ( int ib = 1; ib < _h->GetXaxis()->GetNbins()+1; ib++ ){
				if ( 0 < _h->GetBinContent( ib ) )
					hre->SetBinContent( ib, _h->GetBinError( ib ) / _h->GetBinContent( ib ) );
			}
			return hre;
		}


		HistoBook() {}
		HistoBook( string name, string input = "", string inDir = "" );		
		HistoBook( string name, XmlConfig config, string input = "", string inDir = "");
		~HistoBook();

		virtual void setup( string name, string input="", string inDir="" ){
			this->filename = name;
			this->inputFilename = input;
			this->inputDir = inDir;

			initialize();
		}

		virtual void setup( string name, XmlConfig config, string input="", string inDir=""){
			this->filename = name;
			this->config = config;
			this->inputFilename = input;
			this->inputDir = inDir;

			initialize();
		}

		virtual const char* classname() const { return "HistoBook"; }

		void initialize();
		void mergeIn( string _filename, string _dir );

		unsigned int size() const { return book.size(); }

		TFile * getOutputFile() { return file; }
		/*
		 *Changes into the given directory. 
		 *If the dir DNE it is created, or it is simply set as the current. 
		 *Subdirectories can be used if the entire path is given, paths are never relative
		 *
		 */
		string cd( string dir = "" );
		void mkdir( string path );
		string sanitizePath( string path ) const;



		string ls( bool print = true );
		
		void add( string name, TH1 * );
		TH1* addClone( string name, TH1 * );
		void add( string name, TObject* );
		void addClone( string name, TObject * );
		TH1* get( string name, string sdir = "UNSET_PATH" );
		TH1 * operator[]( string name );

		TH2* get2D( string name, string sdir = "UNSET_PATH" );
		TH3* get3D( string name, string sdir = "UNSET_PATH" );



		/*
		 * This method checks for existance unlike using get(...)->Fill(...). 
		 * If the histo DNE then an error is reported through the Logger and execution continues.
		*/
		void fill( string name, double bin, double weight = 1.0 );
		void fill( string name, double binx, double biny, double weight);
		void fill( string name, double binx, double biny, double binz, double weight);


		/* Fill a histogram by name
		 * Checks for existance
		 */
		void fill( string name, string binLabel, double weight = 1 );

		bool setBinContent( string name, int bin, double content );
		bool setBin( string name, int bin, double content, double error );
		bool setBin( string name, int binX, int binY, double content, double error );
		bool setBinError( string name, int bin, double error );
		

		static TH1 * make( string type, string name, string title, HistoBins &bx, HistoBins &by, HistoBins &bz );
		// TH1 * make( string type, string name, string title, HistoBins &bx, HistoBins &by, HistoBins &bz ){
		// 	return HistoBook::make( type, name, title, bx, by, bz );
		// }
		TH1 * make( string type, string name, string title, HistoBins &bx, HistoBins &by ){
			HistoBins bz;
			return HistoBook::make( type, name, title, bx, by, bz );
		}
		TH1 * make( string type, string name, string title, HistoBins &bx ){
			HistoBins by;
			HistoBins bz;
			return HistoBook::make( type, name, title, bx, by, bz );	
		}
		/*
		 * Makes a histogram from a node in a config file 
		 * 
		 */
		void make( XmlConfig config, string nodeName );
		
		/*
		 *Makes a single histogram from the class config file given during construction
		 *
		 * 
		 */
		void make( string nodeName );

		/*
		 *Makes all histograms that are children of the given parent 
		 *node in the class config file given during construction"
		 */
		void makeAll( string nodeName );
		void makeAll( XmlConfig config, string nodeName );

		void clone( string existing, string create );
		void clone( string ePath, string existing, string create );
		void clone( string ePath, string existing, string cPath, string create );

		

		TDirectory* getDirectory( ) const { return gDirectory; }

		/*
		 * Saves all histograms and other Root objects attached to the current file to 
		 * the permanent file given during construction
		 */
		void save( bool saveAllInDirectory = false ) const;

		void saveOnExit( bool doIt = true ) {
			INFO( classname(), "Auto Save on exit set to " << doIt );
			saveAllOnExit = doIt;
		}

		bool exists( string name, string sdir = "UNSET_PATH" ) const;

		bool is1D(string name, string sdir = "UNSET_PATH");
		bool is2D(string name, string sdir = "UNSET_PATH");
		bool is3D(string name, string sdir = "UNSET_PATH");

		void removeFromDir( string name, string sdir = "UNSET_PATH" );


		string report() const {
			string rp = "";
			for (auto i : book ){
				rp += ( "\"" + i.first + "\" == (" + i.second->ClassName() + "*) " + i.second->GetTitle() + "\n");
			}
			return rp;
		}

		static vector< double > toArray( TH1D * h ) {
			double * values = h->GetArray();
			vector<double> a;
			a.assign( values, values + h->GetSize() );
			return a;
		}

		static vector< vector<double> > toArray( TH2D * h2 ){
			
			int nX = h2->GetNbinsX();
			int nY = h2->GetNbinsY();
			
			vector< vector<double> > a( nX, vector<double>(nY)  );

			for ( int iX = 1; iX <= nX; iX++ ){
				for ( int iY = 1; iY <= nY; iY++ ){
					a[iX-1][iY-1] = h2->GetBinContent( iX, iY );
				}
			}
			return a;

		}

		static void weightByBinWidth( TH1 * _h ){
			int nX = _h->GetNbinsX();
			for ( int i = 1; i <= nX; i++ ){
				double bc = _h->GetBinContent( i );
				double bw = _h->GetBinWidth( i );
				double be = _h->GetBinError( i );

				_h->SetBinContent( i, bc / bw );
				_h->SetBinError( i, be / bw );
			}
		}


	private:
		void globalStyle();
		void loadRootDir( TDirectory*, string path = "" );

#ifdef __CINT__
		ClassDef( jdb::HistoBook, 1 )
#endif
	};	
}






#endif