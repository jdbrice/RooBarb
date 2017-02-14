//
// Created by James Brandenburg on 6/14/15.
//

#ifndef ROOBARB_ROOPLOTTER_H
#define ROOBARB_ROOPLOTTER_H

// STL
#include <map>
#include <memory>
#include <functional>
using namespace std;

// RooBarb
#include "XmlCanvas.h"
#include "RooPlotLib.h"
using namespace jdb;

// ROOT
#include <TFile.h>


class RooPlotter {

protected:

    XmlConfig * cfg;
    map< string, shared_ptr<XmlCanvas> > canvas;

    RooPlotLib rpl;

    vector<string> reportsToMake;

public:

    RooPlotter( string cfgFileName ){
        DEBUG("")
        cfg = new XmlConfig( cfgFileName );

        if ( cfg->exists( "Make" ) ){
            reportsToMake = cfg->getStringVector( "Make" );

            for ( auto k : reportsToMake ){
                makeReport( k );
            }
        }

    }
    ~RooPlotter(){
        delete cfg;
    }


    void makeReport( string rpName ){
        vector<string> paths = cfg->childrenOf( "", 1 );
        TRACE( "Found " << paths.size() << " paths to consider" );
        for ( string path : paths ){
            if ( "Report" != cfg->tagName( path ) ) continue;
            string name = cfg->getString( path + ":name", cfg->getString( path + ":n", "" ) );
            if ( rpName == name ){

                TRACE( "Making : " << rpName )


                // open a TFile to get data
                string datafile = cfg->getString( path + ":data", cfg->getString( path + ":d" ) );
                if ( "" == datafile ){
                    ERROR( "Provide data file with data=\"file.root\"" )
                    continue;
                }

                shared_ptr<TFile> data = shared_ptr<TFile>( new TFile( datafile.c_str(), "READ" ) );


                string filename = name + cfg->getString( "suffix", ".pdf" );
                makeCanvases( path  );
                TCanvas * pseudo = new TCanvas( "ps", "ps", 100, 100 );
                pseudo->Print( (filename +"[").c_str() );

                vector<string> pages = cfg->childrenOf( path, "Page" );
                for ( string page : pages ){
                    makePage( page, filename, data );
                }

                pseudo->Print( (filename +"]").c_str() );
                delete pseudo;

            }
        }
    }
    void makeCanvases( string np ){
        DEBUG("")
        vector<string> paths = cfg->childrenOf( np, "Canvas" );
        for ( string path : paths  ){

            shared_ptr<XmlCanvas> can = shared_ptr<XmlCanvas>( new XmlCanvas( (*cfg), path ) );
            if ( can->getName() != "" ) {
                TRACE( can->getName() << " Created" )
                canvas[can->getName()] = can;
            }
        }
    }

    void makePage( string np, string filename, shared_ptr<TFile> data ){
        string nCanvas = cfg->getString( np + ":canvas" );
        if ( canvas.end() == canvas.find( nCanvas ) ){
            ERROR("Canvas " << nCanvas << " unavailable" )
            return;
        }

        vector<string> plots = cfg->childrenOf( np, "Plot" );
        DEBUG( "Found " << plots.size() << " plots" );
        for ( string plot : plots ){
            makePlot( plot, nCanvas, data );
        }

        canvas[ nCanvas ]->saveImage( filename );
        string nExport = cfg->getString( np + ":export", "" );
        if ( "" != nExport  ){
            DEBUG( "Exporting " << nCanvas << " to " << nExport )
            canvas[ nCanvas ]->saveImage( nExport );
        }

    }

    void makePlot( string np, string nCanvas, shared_ptr<TFile> data ){
        DEBUG("(" << np << ", " << nCanvas << ")")
        string objName = cfg->getString( np + ":name", cfg->getString( np + ":n", "" ) );
        if ( "" == objName /* TODO: check for obj DNE */ ){
            ERROR( objName << " unavailable" )
        }
        string nPad = cfg->getString( np + ":on" );
        canvas[ nCanvas ]->activatePad( nPad );

        TObject * obj = data->Get( objName.c_str() );

        string stylePath = np;
        if ( cfg->exists( np + ":style" ) )
            stylePath = findStyle( cfg->getString( np + ":style" ) );


        rpl.style( obj ).set( cfg, stylePath ).draw();


    }

    // TODO: take lambda as query
    string findStyle( string name ) {
        DEBUG( "Looking for style name=" << name )
        vector<string> styles = cfg->childrenOf("");
        DEBUG( styles.size() << " paths to consider" )
        for ( string path : styles ){
            DEBUG( path )
            if ( "Style" != cfg->tagName(path) ) continue;
            DEBUG( path )

            if ( name == cfg->getString( path + ":name", cfg->getString( path + ":n", "" ) ) ){
                DEBUG( "Found Style @ " << path )
                return path;
            }

        }
        return "";
    }

};


#endif //ROOBARB_ROOPLOTTER_H
