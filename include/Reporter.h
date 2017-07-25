#ifndef REPORTER_H
#define REPORTER_H

#include "TCanvas.h"
#include "Logger.h"
#include "XmlConfig.h"

// Interfaces
	#include "IObject.h"
	#include "IConfig.h"


namespace jdb {
	/* Create PDF Reports in ROOT
	 * A utility class to easily create multi-page PDF reports in ROOT
	 */
	class Reporter : public IObject, public IConfig
	{
	public:
		/* Creates a Reporter
		 * @filename The filename of the report
		 * @w the width of the pages
		 * @h the height of the pages
		 */
		Reporter( string filename, int w = 791, int h = 1024 );
		/* Create a Reporter from an Xml Config
		 * @config Xml Config instance
		 * @np Path to Node
		 * @prefix Prefix to prepend to filename (usually for multiple jobs)
		 */
		Reporter( XmlConfig &config, string np, string prefix = "" );
		/* Create a Reporter from an existing Canvas
		 * @filename The filename of the report
		 * @_canvas existing TCanvas object
		 */
		Reporter( string filename, TCanvas * _canvas );
		~Reporter();

		virtual const char * classname() const { return "Reporter"; }

		/* Creates a new page in the report
		 * @dx The number of divisions horizontally
		 * @dy The number of divisions vertically
		 */
		void newPage( int dx = 1, int dy = 1, float marginX = 0.01, float marginY = 0.01);
		/* Change the current pad
		 * @pad The pad number to set active
		 *
		 * Pad 0 is the whole page, Pad 1 is the first division, etc.
		 */
		void cd( int pad );
		/* Change the current pad
		 * @padX The index of the pad in the horizontal direction
		 * @padY The index of the pad in the vertical direction
		 *
		 * Indexed at 1, so ( 1, 1 ) is the top left
		 */
		void cd( int padX, int padY);
		/* Pushes the active pad
		 * The next pad is set active. 
		 * If needed the page is saved and a new page is started with the same divisions.
		 */
		void next();
		/* Saves the current page
		 * 
		 */
		void savePage( string name = "" );

		/* Saves the given Canvas as a page in the report
		 *
		 */
		void savePage( TCanvas *_canvas );
		/* Saves an image to the given filename. 
		 * Must have suffix to determine format, ie .png, .jpg etc."
		 */
		void saveImage( string name );

		/* Update the ROOT undertanding of contents on the current canvas
		 *
		 */
		void update(){
			canvas->Update();
		}

		/* Get the canvas used by the Reporter
		 * @return the TCanvas instance
		 */
		TCanvas * getCanvas() { return canvas; }
		void setCanvas( TCanvas * _canvas ){
			this->canvas = _canvas;
		}

		void close();

	protected:

		// TCanvas used for drawing and saving graphics
		TCanvas* canvas;
		// Index of the current Pad
		int currentPad;
		// The number of horizontal and vertical divisions on the page
		int dx, dy;
		// Filename of the report
		string filename;
		// Number of instances of the Reporter running
		static int instances;
		bool isOpen;

	};
	
}



#endif