#<cldoc:jdb::ChainLoader>

Static use class for loading .root files into a TChain

Static class used for loading files into TChains.
Can be used to load an entire directory, or files from a filelist.lis

---

**Headers** : ChainLoader.h 
**Source** : ChainLoader.cpp


### ChainLoader Example usage

To load the first N files in a directory:
``` cpp
TChain * chain = new TChain( "treeName" );
ChainLoader::load( chain, "path_to_dir_containg_.roots", N );
```

A single file can be loaded by replacing path_to_dir with the path_to_root_file:
``` cpp
TChain * chain = new TChain( "treeName" );
ChainLoader::load( chain, "path_to_data.root" );
```

To load the files i through k (optional) in a directory:
``` cpp
TChain * chain = new TChain( "treeName" );
ChainLoader::loadRange( chain, "path_to_dir_containg_.roots", i, k );
```

If you are running parallel or want to load from a file list use:

``` cpp
TChain * chain = new TChain( "treeName" );
ChainLoader::loadList( chain, "path_to_dir_listfile.lis", maxFilesToLoad );
```