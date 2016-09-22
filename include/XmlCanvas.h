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
using namespace jdb;

#include "TCanvas.h"



class XmlCanvas {

protected:
    string name, title;

    int pxWidth, pxHeight;
    int divX, divY;

    TCanvas * rootCanvas = nullptr;
    map<string, unique_ptr<XmlPad> > pads;

public:
    string getName() { return name; }

    XmlCanvas( XmlConfig * cfg, string _nodePath ) {
        TRACE( "" );
        if ( cfg && cfg->exists( _nodePath ) ) {
            string preNode = cfg->cn(_nodePath);

            pxWidth = cfg->getInt( ":width", cfg->getInt( ":w", 800 ) );
            pxHeight = cfg->getInt( ":height", cfg->getInt( ":h", 800 ) );

            name = cfg->getString( ":name", cfg->getString( ":n", "c" ) );
            title = cfg->getString( ":title", cfg->getString( ":t", "c" ) );

            DEBUG( "name=" << name << ", title=" << title <<", width=" << pxWidth << ", height=" << pxHeight );
            rootCanvas = new TCanvas( name.c_str(), title.c_str(), pxWidth, pxHeight );

            createPads( cfg, _nodePath );

            cfg->cn(preNode);
        } else {
            TRACE("No XmlConfig Given : Creating Default Canvas" );
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

    void activatePad( string padName ){

        if ( pads.find( padName ) != pads.end() ){
            pads[ padName ]->cd();
        } else {
            ERROR( "Requested pad : " << padName << " unavailable" )
            rootCanvas->cd();
        }
    }

protected:
    void createPads( XmlConfig * cfg, string _nodePath ){
        TRACE( "" );
        vector<string> children = cfg->childrenOf( _nodePath, "Pad" );
        DEBUG( "Found " << children.size() );
        for ( string path : children ){
            DEBUG( "Creating Pad From " << path );
            this->cd();
            cfg->cn( path );
            string name = cfg->getString( ":name", cfg->getString( ":n", "" ) );
            DEBUG( "Creating Pad named " << name );
            if ( "" != name )
                pads[ name ] = unique_ptr<XmlPad>( new XmlPad( cfg, "" ) );
        }

    }

};

#endif //ROOBARB_XMLCANVAS_H
