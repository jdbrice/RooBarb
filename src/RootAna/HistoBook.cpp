

#include "HistoBook.h"
#include "Logger.h"

// ROOT
#include "TKey.h"
#include "TObject.h"
#include <TApplication.h>

#ifdef __CINT__
ClassImp( jdb::HistoBook );
#endif

namespace jdb{

	/**
	 * Creates a histobook and allows the root filename to be set. optionally read from an existing root
	 * file and include everything into the working space
	 * @param name  root filename
	 * @param input input filename
	 * @param inDir input starting directory
	 */
	HistoBook::HistoBook( string name, string input, string inDir ){
		this->filename = name;
		this->inputFilename = input;
		this->inputDir = inDir;

		initialize();
	}	// Constructor

	/**
	 * Constructor that allows th input of a config file
	 * @param name name of file to use for saving root data
	 * @param con  The config file to use for all config relates calls
	 */
	HistoBook::HistoBook( string name, XmlConfig config, string input, string inDir){
		this->filename = name;
		this->config = config;
		this->inputFilename = input;
		this->inputDir = inDir;

		initialize();
	}	// Constructor

	void HistoBook::initialize(){
		DEBUG( classname(), "" );

		// append .root to end if needed
		if (this->filename.find(  ".root") == std::string::npos){
			this->filename = this->filename + ".root";
		}

		// open the output file
		this->file = new TFile( this->filename.c_str(), "recreate" );
		this->file->cd();

		if ( this->file->IsOpen() ){
			DEBUG( classname(), "Output File : " << this->filename << " opened" );
		} else {
			ERROR( classname(), "Output File : " << this->filename << " not opened" );
		}

		// set our default global style
		globalStyle();
		
		// if an input was given merge it into the live record
		mergeIn( this->inputFilename, this->inputDir );

		this->currentDir = "";


		// default to auto save on exit
		saveOnExit( true );

		nullHisto = new TH1C( "NULL", "NULL", 1, 0, 1 );
		nullHisto->SetDirectory( 0 );
	}


	/**
	 * Merges in a root file
	 * 
	 * @param _filename Filename of root file to load
	 * @param _dir      Optional: sub directory to load
	 */
	void HistoBook::mergeIn( string _filename, string _dir ){
		DEBUG( classname(), "(" << _filename << ", " << _dir << " )"  );

		if (_filename.find(  ".root") == std::string::npos){
			_filename = _filename + ".root";
		}

		if ( _filename.length() >= 6 ){
			DEBUG( classname(), "Loading : " << _dir << " from " << _filename );
			
			TFile * fin = new TFile( _filename.c_str() );
			
			this->cd ( _dir );
			
			fin->cd( _dir.c_str() );
			loadRootDir( gDirectory, _dir );
		}
	}

	/**
	 * Destructor
	 * Frees the legend and ensures that the data is written to file
	 * and the file is closed properly
	 */
	HistoBook::~HistoBook(){
		DEBUG( classname(), "" );

		if ( saveAllOnExit  ){
			save();
		} else {
			WARN( classname(), "Not Saving Book" );
		}
		file->Close();
		DEBUG( classname(), "Shutting Down" );
	}	// Destructor

	void HistoBook::save(bool saveAllInDirectory ) const  {
		// TODO: fix hard code below!
		DEBUG( classname(), " Save to File " << filename );
		
		if ( true )
			file->Write();
		else {
			for ( auto k : book ){
				if ( k.second ){
					DEBUG(classname(),  "Writing " << k.first << " to file" );
					k.second->Write();
				} else {
					ERROR( classname(), "Cannot Write " << k.first << " to file : null" );
				}
			}	
		}
	}	// save

	/**
	 * Loads a directory into the histobook
	 */
	void HistoBook::loadRootDir( TDirectory* tDir, string path ){
		DEBUG( classname(), " Loading from directory " << path );

		TList* list;

		if ( tDir ){
			list = tDir->GetListOfKeys();
		} else {
			DEBUG( classname(), " Bad Directory " << path );
			return;
		}

		TIter next(list);
		TKey* key;
		TObject* obj;

		while ( (key = (TKey*)next()) ) {

			obj = key->ReadObj() ;

			if ( 0 == strcmp(obj->IsA()->GetName(),"TDirectoryFile") ){
				TDirectoryFile* dir = (TDirectoryFile*)obj;

				string nPath = path + dir->GetName();
				if ( path == (string) "" )
					nPath = path + dir->GetName();
				else
					nPath = path + "/" + dir->GetName();

				cd( nPath );
				loadRootDir( dir, nPath );
			} else if ( obj ){
				if (    (strcmp(obj->IsA()->GetName(),"TProfile")!=0) && (!obj->InheritsFrom("TH2") && (!obj->InheritsFrom("TH1"))) ) {
					// not a 1d or 2d histogram
				} else {
					// add it to the book

					add( obj->GetName(), (TH1*)obj->Clone( obj->GetName() ) );
				}

			}
		}
	} // loadRootDir

	void HistoBook::add( string name, TH1* h ){
		TRACE( classname(), "(name=\"" << name << "\", TH1 * h=" << h );

		string oName = name;
		if ( name.length() <= 1 ){
			WARN( classname(), " Cannot add " << name << " to dir " << currentDir << " : Invalid name" );
			return;
		}

		if ( nullptr == h ){
			WARN( classname(), "Cannot add NULL histogram" );
			return;
		}

		string fullPath  = currentDir + name;

		// dont allow duplicated name overites
		if ( book[ fullPath ] ){
			WARN( classname(), " Cannot add \"" << name << "\" to dir \"" << currentDir << "\": Duplicate exists with fullPath= \"" << fullPath << "\"" );
			return;
		}

		// save the histo to the map
		book[ fullPath ] = h;
		DEBUG( classname(), "Adding \"" << fullPath <<"\"" );

		// this is kept for legacy
		//add( name, (TObject*)h );
	} 	// add

	void HistoBook::addClone( string name, TH1 * h ){
		add( name, (TH1*)h->Clone( name.c_str() ) );
	}

	void HistoBook::add( string name, TObject* h ){

		DEBUG(classname(),  " Adding " << name );

		string oName = name;
		if ( name.length() <= 1 || !h ){
			WARN( classname(), " Cannot add " << name << " to dir " << currentDir << " : Invalid name" );
			return;
		}

		name = currentDir + name;

		// dont allow duplicated name overites
		if ( book[ name ] ){
			WARN( classname(), " Cannot add " << name << " to dir " << currentDir << " : Duplicate exists" );
			return;
		}

		// save the histo to the map
		book[ name ] = h;

		cd();
		h->Write();
	} 	// add

	void HistoBook::addClone( string name, TObject * h ){
		add( name, (TObject*)h->Clone( name.c_str() ) );
	}

	void HistoBook::mkdir( string path){
		path = sanitizePath( path );
		char* csdir = (char*)path.c_str();
		TRACE( classname(), "Creating Directory \"" << path << "\"" );
		file->mkdir( csdir );
	}

	string HistoBook::cd( string sdir  ){

		sdir = sanitizePath( sdir );
		
		DEBUG( classname(), "cd \"" << sdir << "\"" );

		string old = currentDir;

		char* csdir = (char*)sdir.c_str();
		file->cd();

		// make it if it does not exist
		if ( !file->GetDirectory( csdir ) )
			mkdir( sdir );
		
		file->cd( csdir );

		// set current directory
		currentDir = sdir;

		return old;
	}	// cd

	string HistoBook::sanitizePath( string path ) const{
		DEBUG( classname(), "(path = \"" << path << "\")" );
		
		// change unset paths to cwd
		if ( "UNSET_PATH" == path )
			path = currentDir;

		if ( 0 == path.compare( 0, 1, "/" ) && path.length() >= 2 )
			path = path.substr( 1 );
		
		if ( path.length() >= 1 && 0 != path.compare( path.length() - 1, 1, "/" ) )
			path = path + "/";
		
		// but we don't want "/" as the base path
		if ( "/" == path )
			path = "";

		return path;
	}	// sanitizePath
	
	TH1 * HistoBook::make( string _type, string _name, string _title, HistoBins &_bx, HistoBins &_by, HistoBins &_bz ){
		DEBUG( classname(), "type=" << _type << " name=\"" << _name << "\" title=\"" << _title << "\" bx=" << _bx.toString() << ", by=" << _by.toString() << ", bz=" << _bz.toString() );

		int nD = 0;
		if ( (_bx.nBins() > 0) && (_by.nBins() <= 0) && (_bz.nBins() <= 0) )
			nD = 1;
		if ( (_bx.nBins() > 0) && (_by.nBins() > 0) && (_bz.nBins() <= 0) )
			nD = 2;
		if ( (_bx.nBins() > 0) && (_by.nBins() > 0) && (_bz.nBins() > 0) )
			nD = 3;

		TRACE( classname(), "nDimensions = " << nD );

		if ( 1 == nD ){
			if ( "C" == _type )
				return new TH1C( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data() );
			if ( "S" == _type )
				return new TH1S( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data() );
			if ( "I" == _type )
				return new TH1I( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data() );
			if ( "F" == _type )
				return new TH1F( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data() );
			if ( "D" == _type )
				return new TH1D( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data() );
		} // make 1Ds
		else if ( 2 == nD ){
			if ( "C" == _type )
				return new TH2C( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data() );
			if ( "S" == _type )
				return new TH2S( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data() );
			if ( "I" == _type )
				return new TH2I( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data() );
			if ( "F" == _type )
				return new TH2F( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data() );
			if ( "D" == _type )
				return new TH2D( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data() );
		} // make 2Ds
		else if ( 3 == nD ){
			if ( "C" == _type )
				return new TH3C( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data(), _bz.nBins(), _bz.bins.data() );
			if ( "S" == _type )
				return new TH3S( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data(), _bz.nBins(), _bz.bins.data() );
			if ( "I" == _type )
				return new TH3I( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data(), _bz.nBins(), _bz.bins.data() );
			if ( "F" == _type )
				return new TH3F( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data(), _bz.nBins(), _bz.bins.data() );
			if ( "D" == _type )
				return new TH3D( _name.c_str(), _title.c_str(), _bx.nBins(), _bx.bins.data(), _by.nBins(), _by.bins.data(), _bz.nBins(), _bz.bins.data() );
		} // make 2Ds

		WARN( classname(), "Unable to make Histogram " << _type << ", " << _name << ", " << _title );
		return nullptr;
	}

	void HistoBook::make( string nodeName ){
		make( config, nodeName );
	}	//make
	void HistoBook::make( XmlConfig config, string nodeName ){

		if ( config.exists( nodeName ) ){

			string hName = config.tagName( nodeName );
			// backward compatible
			if ( config.exists( nodeName + ":name" ) )
				hName = config.getString( nodeName + ":name" );
			if ( "" == hName )
				hName = nodeName;

			// store the path in the config file
			configPath[ hName ] = nodeName;

			string type 	= config.getString( nodeName + ":type", "D" );
			string hTitle 	= config.getString( nodeName + ":title", hName );


			HistoBins* bx;
			HistoBins* by;
			HistoBins* bz;


			// make the HistBins object from the config path
			if ( config.exists( nodeName + ":bins_x" ) )
				bx = new HistoBins( config, config.getString( nodeName + ":bins_x" ) );
			else 
				bx = new HistoBins( config, nodeName, "x" );
			if ( config.exists( nodeName + ":bins_y" ) )
				by = new HistoBins( config, config.getString( nodeName + ":bins_y" ) );
			else 
				by = new HistoBins( config, nodeName, "y" );
			if ( config.exists( nodeName + ":bins_z" ) )
				bz = new HistoBins( config, config.getString( nodeName + ":bins_z" ) );
			else 
				bz = new HistoBins( config, nodeName, "z" );


			// check if histogram exists, if it does the copy in the book won't be over-written but the ROOT one will if they have the same names
			TH1 * tmp = nullptr;

			if ( !exists( hName ) ){
				tmp = make( type, hName, hTitle, (*bx), (*by), (*bz) );

				if ( nullptr != tmp ){
					add( hName, tmp );
				} else if ( config.exists( nodeName + ":bins_x" ) || config.exists( nodeName + ":bins_y" ) || config.exists( nodeName + ":bins_z" ) ) {
					ERROR( classname(), "could not make histogram : " << hName );
					if ( config.exists( nodeName + ":bins_x" ) ){
						ERROR( classname(), "x bins : " << bx->toString() );
					}
					if ( config.exists( nodeName + ":bins_y" ) ){
						ERROR( classname(), "y bins : " << by->toString() );
					}
					if ( config.exists( nodeName + ":bins_z" ) ){
						ERROR( classname(), "z bins : " << bz->toString() );
					}
					// ERROR( classname(), "histo : " << tmp );
				}
			} else {
				WARN( classname(), "Duplicate " << hName << " Cannot Add" );
			}

			delete bx;
			delete by;
			delete bz;
		}

	}	// make
	void HistoBook::makeAll( XmlConfig config, string nodeName ){

		vector<string> paths = config.childrenOf( nodeName );
		TRACE( classname(), " Found " << paths.size() << " histogram paths " );

		

		for ( unsigned int i=0; i < paths.size(); i++ ){
			if ( "Include" == config.tagName( paths[ i ] ) ) continue;

			// check the parent node for subdir info
			string parentNode = config.pathToParent( paths[ i ] );
			// look for a dir to cd into
			DEBUG( classname(), "Looking for subdir at " << parentNode + ":dir" );
			string usedir = config.getString( parentNode + ":dir", "NADIR" );
			DEBUG( classname(), "Found " << usedir );
			if ( "NADIR" !=  usedir  ){
				cd( usedir );
			}
			
			make( config, paths[ i ] );
		}
	}	//makeAll
	void HistoBook::makeAll( string nodeName ){
		makeAll( config, nodeName );
	}	//makeAll

	void HistoBook::clone( string existing, string create ){
		DEBUG( classname(), " Cloning " << existing << " into " << create );

		if ( get( existing ) ){
			TH1* nHist = (TH1*)get( existing )->Clone( create.c_str() );
			// add the new one
			add( create, nHist );
		} else {
			WARN( classname(), existing << " Does Not Exist " );
		}
	}	//clone

	void HistoBook::clone( string ePath, string existing, string cPath, string create ){
		
		ePath = sanitizePath( ePath );
		cPath = sanitizePath( cPath );

		DEBUG( classname(), " Cloning " << existing << " into " << create );
		
		if ( get( existing, ePath ) ){
			string oDir = cd( cPath ); 
			TH1* nHist = (TH1*)get( existing, ePath )->Clone( create.c_str() );
		
			// add the new one
			add( create, nHist );

			cd( oDir );
		} else {
			WARN( classname(), existing << " Does Not Exist " );
		}
	}	//clone

	void HistoBook::clone( string ePath, string existing, string create ){
		ePath = sanitizePath( ePath );
		DEBUG( classname(), " Cloning " << existing << " into " << create );
		
		if ( get( existing, ePath ) ){
			TH1* nHist = (TH1*)get( existing, ePath )->Clone( create.c_str() );
		
			// add the new one
			add( create, nHist );
		} else {
			WARN( classname(), existing << " Does Not Exist " );
		}
	}	//clone

	bool HistoBook::exists( string name, string sdir ) const {
		sdir = sanitizePath( sdir );
		DEBUG( classname(), "( name=\"" << name << "\" dir=\"" << sdir << "\")");

		if ( book.count( sdir + name ) <= 0 ) {
			DEBUG( classname(), sdir + name  << " Does Not Exist " ) 
			return false;
		}

		return true;
	}

	TH1* HistoBook::operator[]( string name ) {
		return get( name );
	}

	TH1* HistoBook::get( string name, string sdir  ){
		sdir = sanitizePath( sdir );
		DEBUG( classname(), "( name=\"" << name << "\" dir=\"" << sdir << "\")");

		string fullPath = sdir + name;
		DEBUG( classname(), "fullPath=\"" << fullPath << "\"" );

		if ( !exists( name, sdir ) ){
			WARN( classname(), fullPath  << " Does Not Exist " );
			return nullHisto;
		}

		return (TH1*)book[ fullPath ];
	}	//get

	TH2* HistoBook::get2D( string name, string sdir  ){
		return ((TH2*)get( name, sdir ));
	}	//get2D
	TH3* HistoBook::get3D( string name, string sdir ){
		return (( TH3* ) get( name, sdir ));
	}	//get3D

	void HistoBook::fill( string name, double bin, double weight ){
		// if ( exists( name ) ){
		get( name )->Fill( bin, weight );
		// } else {
			// WARN( classname(), "\"" << name << "\" Does Not Exist, cannot fill " );
		// }
	}	//fill

	void HistoBook::fill( string name, double binx, double biny, double weight ){
		if ( is2D( name )  ){
			get2D( name )->Fill( binx, biny, weight );
		} else if ( is3D(name) ){
			get3D( name )->Fill( binx, biny, weight );
		} else {
			WARN( classname(), "\"" << name << "\": Cannot fill 2D or 3D" );
		}
	}	//fill

	void HistoBook::fill( string name, double binx, double biny, double binz, double weight ){
		if ( is3D( name ) ){
			get3D( name )->Fill( binx, biny, binz, weight );
		} else {
			WARN( classname(), "\"" << name << "\": Cannot fill 3D" );
		}
	}	//fill

	void HistoBook::fill( string name, string binLabel, double weight ){
		get( name )->Fill( binLabel.c_str(), weight );
	}	//fill

	bool HistoBook::setBinContent( string name, int bin, double content ){
		if ( !exists( name ) )
			return false;
		get( name )->SetBinContent( bin, content );
		return true;
	}
	bool HistoBook::setBinError( string name, int bin, double error ){
		if ( !exists( name ) )
			return false;
		get( name )->SetBinError( bin, error );
		return true;
	}
	bool HistoBook::setBin( string name, int bin, double content, double error ){
		if ( !exists( name ) )
			return false;
		get( name )->SetBinContent( bin, content );
		get( name )->SetBinError( bin, error );
		return true;
	}

	void HistoBook::globalStyle(){

		gStyle->SetCanvasColor(kWhite);     // background is no longer mouse-dropping white
	  	gStyle->SetPalette(1,0);            // blue to red false color palette. Use 9 for b/w
	  	gStyle->SetCanvasBorderMode(0);     // turn off canvas borders
	  	gStyle->SetPadBorderMode(0);
	  	gStyle->SetPaintTextFormat("5.2f");  // What precision to put numbers if plotted with "TEXT"

	  	// For publishing:
	  	gStyle->SetLineWidth(2.);
	  	gStyle->SetTextSize(0.7);
	  	gStyle->SetLabelSize(0.05,"xy");
	  	gStyle->SetTitleSize(0.05,"xy");
	  	gStyle->SetTitleOffset(1.0,"x");
	  	gStyle->SetTitleOffset(1.0,"y");
	  	gStyle->SetPadTopMargin(0.1);
	  	gStyle->SetPadRightMargin(0.1);
	  	gStyle->SetPadBottomMargin(0.16);
	  	gStyle->SetPadLeftMargin(0.21);

	  	gStyle->SetFillColor(-1);
		gStyle->SetFillStyle(4000);
	}	//globalStyle

	string HistoBook::ls( bool print ){
		
		stringstream lss;
		for( auto k : book ) {
			if ( print ){
				INFO( classname(), "[\"" << k.first << "\"] = " << k.second );
			}
			lss << "[\"" << k.first << "\"] = " << k.second << endl;
		}
		return lss.str();
	}
	bool HistoBook::is1D(string name, string sdir){
		DEBUG( classname(), "( name=\"" << name << "\" dir=\"" << sdir << "\")");
		if ( exists( name, sdir) ){
			TH1* h = get( name, sdir);
			if ( h->GetNbinsY() <= 1 && h->GetNbinsZ() <= 1 && h->GetNbinsX() >= 1 ){
				return true;
			}
		}
		return false;
	} 
	bool HistoBook::is2D(string name, string sdir){
		DEBUG( classname(), "( name=\"" << name << "\" dir=\"" << sdir << "\")");
		if ( exists( name, sdir) ){
			TH1* h = get( name, sdir);
			if ( h->GetNbinsY() >= 2 && h->GetNbinsZ() <= 1 && h->GetNbinsX() >= 1 ){
				return true;
			}
		}
		return false;
	} 
	bool HistoBook::is3D(string name, string sdir){
		DEBUG( classname(), "( name=\"" << name << "\" dir=\"" << sdir << "\")");
		if ( exists( name, sdir) ){
			TH1* h = get( name, sdir);
			if ( h->GetNbinsY() >= 2 && h->GetNbinsZ() >= 2 && h->GetNbinsX() >= 1 ){
				return true;
			}
		}
		return false;
	} 

	void HistoBook::removeFromDir( string name, string sdir ){
		sdir = sanitizePath( sdir );
		DEBUG( classname(), "( name=\"" << name << "\" dir=\"" << sdir << "\")");
		TH1 * h = get( name, sdir );	
		if ( h ){
			TRACE( classname(), "Removing \"" << name << "\" from directory " );
			h->SetDirectory( 0 );
		}
	}

} // jdb namespace
