#ifndef XML_TFILE_H
#define XML_TFILE_H

#include "XmlConfig.h"
#include "IObject.h"

#include "XmlString.h"


namespace jdb {

	/*
	 * filename like : basePath + url
	 * to allow project wide path setup
	 */

	class XmlTFile : public IObject
	{
	public:
		virtual const char* classname() const { return "XmlTFile"; }
		XmlTFile() {}
		XmlTFile( XmlConfig &cfg, string nodePath, bool openFile = true ) { load( cfg, nodePath, openFile ); }
		~XmlTFile() { }

		TFile * getTFile() { return tfile; }

		void load( 	XmlConfig &cfg, 
					string nodePath,
					bool openFile    = true,
					string _basePath = "",
					string _umod     = ":url",
					string _mmod     = ":mode",
					string _tmod     = ":title",
					string _cmod     = ":compressionLevel" ){

			url              = cfg.getString( nodePath + _umod, url );
			mode             = cfg.getString( nodePath + _mmod, mode );
			title            = cfg.getString( nodePath + _tmod, title );
			compressionLevel = cfg.getInt( nodePath + _cmod, compressionLevel );

			INFO( classname(), "URL : " << url );

			url = XmlString().format( cfg, url );
			INFO( classname(), "URL : " << url );

			if ( tfile && tfile->IsOpen() ){
				tfile->Close();
				delete tfile;
				tfile = nullptr;
			}

			if ( openFile ){
				string full_url = this->getQualifiedUrl();
				tfile = new TFile( full_url.c_str(), mode.c_str(), title.c_str(), compressionLevel );	
			}
		}

		string getQualifiedUrl() { return path + url; }

	protected:

		string url           = "tfile.root";
		string path          = "";
		string mode          = "READ";
		string title         = "";
		int compressionLevel = 1;

		TFile * tfile = nullptr;
		
	};
}

#endif