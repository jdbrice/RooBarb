
#ifndef CHAINLOADER_H
#define CHAINLOADER_H

#include "dirent.h"
#include "TChain.h"
#include "Logger.h"
#include "IObject.h"


namespace jdb{

	/* 
	 * see mergedoc/jdb::ChainLoader
	 */
	class ChainLoader : public IObject {

	public:

		virtual const char * classname() const { return "ChainLoader"; }
		/* Loads files in a directory to the TChain
		 * @chain an initialized TChain to recieve the files
		 * @ntdir path to directory containing .root files or the path to a single .root file
		 * @maxFiles maximum number of files to load
		 *
		 * Loads .root files containing TTrees into 
		 * a TChain from the given directory. Make sure
		 * that the TChain was created with the correct
		 * tree name.
		 *
		 */
		static void load( TChain * chain, string ntdir, int maxFiles = -1 );

		/* Loads files in a directory to the TChain
		 * @chain an initialized TChain to recieve the files
		 * @ntdir path to directory containing .root files
		 * @firstFile the index of the first file to include
		 * @lastFile the index of the last file to include
		 *
		 * Loads .root files containing TTrees into 
		 * a TChain from the given directory. Make sure
		 * that the TChain was created with the correct
		 * tree name.
		 */
		static void loadRange( TChain * chain, string ntdir, int firstFile, int nFiles = 100000 );
		
		
		/* Loads files in a directory to the TChain
		 * @chain an initialized TChain to recieve the files
		 * @listFile list of files to add to chain. Should 
		 * use full pathto file
		 * @maxFiles maximum number of files to load
		 *
		 * Loads .root files containing TTrees into 
		 * a TChain from the given list file. This 
		 * method allows parallel jobs to work on separate
		 * file lists. Make sure that the TChain was 
		 * created with the correct tree name.
		 */
		static void loadList( TChain * chain, string listFile, int maxFiles = -1 );


		/* Loads files in a filelist to a TChain
		 * @chain an initialized TChain to recieve the files
		 * @listFile list of files to add to chain. Should 
		 * use full path to file
		 * @index	job index. 
		 *        	files processed is : index * splitBy --> (index+1) * splitBy - 1
		 *
		 * Loads .root files containing TTrees into 
		 * a TChain from the given list file. This 
		 * method allows parallel jobs to work on separate
		 * file lists. Make sure that the TChain was 
		 * created with the correct tree name.
		 */
		static void loadListRange( TChain * chain, string listFile, int index, int splitBy = 50 );
	};


}


#endif

