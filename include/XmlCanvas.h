//
// Created by James Brandenburg on 6/4/15.
//

#ifndef ROOBARB_XMLCANVAS_H
#define ROOBARB_XMLCANVAS_H

// STL
#include <map>
using namespace std;

#include "Logger.h"
#include "XmlConfig.h"
#include "XmlPad.h"

#include "TCanvas.h"


namespace jdb{
    class XmlCanvas : public IObject{

    protected:
        string name, title;

        int pxWidth, pxHeight;
        int nCol, nRow;

        TCanvas * rootCanvas = nullptr;
        map<string, shared_ptr<XmlPad> > pads;

    public:
    virtual const char * classname() const { return "XmlCanvas"; }
        string getName() { return name; }

        XmlCanvas( XmlConfig &cfg, string _nodePath ) {
            if ( cfg.exists( _nodePath ) ) {
                string preNode = cfg.cn(_nodePath);

                pxWidth = cfg.getInt( ":width", cfg.getInt( ":w", 800 ) );
                pxHeight = cfg.getInt( ":height", cfg.getInt( ":h", 800 ) );

                name = cfg.getString( ":name", cfg.getString( ":n", "c" ) );
                title = cfg.getString( ":title", cfg.getString( ":t", "c" ) );

                nCol = cfg.getInt( ":columns", cfg.getInt( ":cols", cfg.getInt( ":nCols", 12 ) ) );
                nRow = cfg.getInt( ":rows", 12 );

                DEBUGC( "name=" << name << ", title=" << title <<", width=" << pxWidth << ", height=" << pxHeight );
                rootCanvas = new TCanvas( name.c_str(), title.c_str(), pxWidth, pxHeight );

                createPads( cfg, _nodePath );

                cfg.cn(preNode);
            } else {
                TRACEC("No XmlConfig Given : Creating Default Canvas" );
                rootCanvas = new TCanvas( "XmlCanvas", "XmlCanvas", 800, 1200 );
            }

        }

        void cd( int iPad = 0 ) {
            if ( !rootCanvas )
                return;
            rootCanvas->cd(iPad);
        }

        void saveImage( string iname ) {
            if ( !rootCanvas )
                return;

            rootCanvas->Print( iname.c_str() );
        }

        XmlPad* activatePad( string padName ){

            if ( pads.find( padName ) != pads.end() ){
                pads[ padName ]->cd();
                return pads[ padName ]->getPad();
            } else {
                rootCanvas->cd();
            }
            return nullptr;
        }

        TCanvas * getCanvas(){
            return rootCanvas;
        }

    protected:
        void createPads( XmlConfig &cfg, string _nodePath ){
            
            vector<string> children = cfg.childrenOf( _nodePath, "Pad" );
            DEBUGC( "Found " << children.size() );
            for ( string path : children ){
                DEBUGC( "Creating Pad From " << path );
                this->cd();
                cfg.cn( path );
                string name = cfg.getString( ":name", cfg.getString( ":n", "" ) );
                DEBUGC( "Creating Pad named " << name );
                if ( "" != name )
                    pads[ name ] = shared_ptr<XmlPad>( new XmlPad( cfg, "", nRow, nCol ) );
            }

        }

    };
}// namespace

#endif //ROOBARB_XMLCANVAS_H
