
# Architecture
ARCH          = $(shell root-config --arch)


#------------------------------------------------------------------------------
Headers = 	Logger Reporter XmlConfig ChainLoader HistoBook RooPlotLib Utils \
			TaskFactory TreeAnalyzer HistoAnalyzer SharedTreeAnalyzer\
			DataSource EvaluatedLeaf \
			XmlFunction FitConfidence \
			LoggerConfig XmlPoint XmlRange XmlGraph XmlBinnedData XmlString XmlHistoBook\
			CutCollection \
			TaskRunner IObject IConfig IHistoBookMaker

Headers = 	ANSIColors \
			ChainLoader \
			CommandRunner \
			CutCollection \
			DataSource \
			EvaluatedLeaf \
			FitConfidence \
			HistoAnalyzer \
			HistoBins \
			HistoBook \
			IConfig \
			IHistoBookMaker \
			IObject \
			ITaskRunner \
			Logger \
			LoggerConfig \
			RapidXmlWrapper \
			Reporter \
			RooPlotLib \
			RooPlotter \
			SharedTreeAnalyzer \
			TaskEngine \
			TaskFactory \
			TaskRunner \
			TreeAnalyzer \
			Utils \
			XmlBinnedData \
			XmlCanvas \
			XmlConfig \
			XmlFunction \
			XmlGraph \
			XmlHistoBook \
			XmlHistogram \
			XmlPad \
			XmlPoint \
			XmlRange \
			XmlString \
			XmlTFile 

##-----------------------------------------------------------------------------


incDir        = include
hdrSuf        = h


# Root libs
ROOTCFLAGS    	= $(shell root-config --cflags)
ROOTLDFLAGS    	= $(shell root-config --ldflags)
ROOTLIBS      	= $(shell root-config --libs)
ROOTLDFLAGS    	= $(shell root-config --ldflags)

includes		= -Iinclude -I. 

CXXFLAGS      	= -std=c++11 -fPIC 
CXXFLAGS     	+= $(ROOTCFLAGS) -DROOT6_FEATURES -DJDB_LOG_LEVEL=60

ClassHeaders 	= $(addsuffix .$(hdrSuf), $(addprefix $(incDir)/, $(Headers)))
# ClassHeaders 	+= $(addsuffix .$(hdrSuf), $(addprefix $(incDir)/, $(HeaderOnlyClasses)))
NoDocs 			= include/format.h
ClassHeaderDocs = $(filter-out $(NoDocs),$(ClassHeaders))

#------------------------------------------------------------------------------


doc:
	# @echo "Generating Documentations";			\
	# @echo "HEADERS for DOCS : $(ClassHeaderDocs)\n\n"\
	
	clDoc generate $(includes) $(CXXFLAGS) -- 	\
			--report 							\
			--output html						\
			--merge html/mergedocs/				\
			$(ClassHeaderDocs)
	python ./html/sanitize.py html/xml/

sdoc:
	python ./html/sanitize.py html/xml/

cleandoc:
	@rm -rf html/xml
	@rm html/search.json

