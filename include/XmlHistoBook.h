#ifndef XML_HISTO_BOOK_H
#define XML_HISTO_BOOK_H

#include "HistoBook.h"
	#include "IObject.h"
#include "Logger.h"
#include "XmlString.h"

#include <memory>


namespace jdb {
	class XmlHistoBook : public IObject
	{
	public:
		virtual const char* classname() const {return "XmlHistoBook"; }
		XmlHistoBook( XmlConfig &cfg, string nodePath ) {  set( cfg, nodePath ); }
		XmlHistoBook() {}
		~XmlHistoBook() {}

		shared_ptr<HistoBook> getBook() { return book; }

		void set( XmlConfig &cfg, string nodePath ){
			DEBUG( classname(), "cfg=" << cfg.getFilename() << ", nodePath=" << nodePath );
			
			if ( !cfg.exists( nodePath ) ){
				WARN( classname(), "Could not fine node at " << nodePath );
				return;
			}

			// searchs :url first, if not uses the value in the node itself, if nothing there (but node exists?) then default to "histobook.root"
			string _url = XmlString().format( cfg, 
					cfg.getString( 
						nodePath + ":url", 
						cfg.getString( nodePath, "histobook.root" ) 
					) 
				);

			INFO( classname(), "url=" << _url );
			book = shared_ptr<HistoBook>( new HistoBook( _url, cfg ) );
		}

	protected:
		shared_ptr<HistoBook> book = nullptr;
		
	};
}

#endif