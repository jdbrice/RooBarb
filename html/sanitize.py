#!/usr/bin/python

# for command line args
import sys
# for regex
import re
#for JSON parsing
import json
# for dir searching
import glob

import xml.etree.ElementTree as ET
ET.register_namespace('', "http://jessevdk.github.com/cldoc/1.0")

stdns = ""

if  len( sys.argv ) < 2 :
	
	print "Usage:"
	print "sanitize_std path/to/xml/"
	exit()

files = glob.glob( sys.argv[1] + "*.xml" )


def sanitize_file( filename ):
	e = ET.parse( filename )
	rn =  e.getroot()

	for t in rn.findall( ".//{http://jessevdk.github.com/cldoc/1.0}type" ) :
		rawType = t.get( "name" )
		fType = re.sub( r'std::__1::', stdns, rawType, 1000 );
		fType = re.sub( r'basic_string<char>', "string", fType, 1000 );
		fType = re.sub( r'\n', "", fType, 1000 );
		
	
		mIndex = fType.find( "map" )
		vIndex = fType.find( "vector" )

		if mIndex <= 5 and mIndex >= 0:
			pl1 = fType.find( "," )
			pl2 = fType.find( ",", pl1+1 )
			fType = fType[ :pl2 ] + ">"

		if vIndex <= 5 and vIndex >= 0:
			pl1 = fType.find( "," )
			fType = fType[ :pl1 ] + ">" 

		t.set( 'name', fType )

	e.write( filename )


for f in files :
	print "Sanitizing ", f
	sanitize_file( f )