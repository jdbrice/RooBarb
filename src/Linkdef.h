#ifdef __ROOTCLING__
// turns off dictionary generation for all
#pragma link off all class;
#pragma link off all function;
#pragma link off all global;
#pragma link off all typedef;
// link to nested scope
#pragma link C++ nestedclass;

#pragma link C++ namespace jdb;
#pragma link C++ defined_in jdb;

#pragma link C++ class jdb::Logger+;
#pragma link C++ class jdb::Reporter-;
#pragma link C++ class jdb::XmlConfig+;
#pragma link C++ class jdb::XmlString+;
#pragma link C++ class jdb::XmlBinnedData+;

#pragma link C++ class jdb::LoggerConfig-;
#pragma link C++ class jdb::TaskTimer-;
#pragma link C++ class jdb::TaskProgress-;

#pragma link C++ class jdb::ChainLoader-;
#pragma link C++ class jdb::HistoBins-;
#pragma link C++ class jdb::HistoBook-;

#pragma link C++ class jdb::TreeAnalyzer-;
#pragma link C++ class jdb::HistoAnalyzer-;

#pragma link C++ class jdb::DataSource-;
#pragma link C++ class jdb::RooPlotLib-;

#pragma link C++ class jdb::TaskRunner+;
#pragma link C++ class jdb::FitConfidence+;

#endif