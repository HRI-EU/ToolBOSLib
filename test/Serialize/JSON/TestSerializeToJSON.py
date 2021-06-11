#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  launches the unit testing
#
#  Copyright (C)
#  Honda Research Institute Europe GmbH
#  Carl-Legien-Str. 30
#  63073 Offenbach/Main
#  Germany
#
#  UNPUBLISHED PROPRIETARY MATERIAL.
#  ALL RIGHTS RESERVED.
#
#


import json
import unittest

from ToolBOSCore.Util               import Any
from ToolBOSCore.Util               import FastScript
from ToolBOSCore.Util.VersionCompat import StringIO


class TestSerializeToJSON( unittest.TestCase ):

    def setUp( self ):
        if not FastScript.getEnv( 'VERBOSE' ) == 'TRUE':
            Any.setDebugLevel( 1 )


    def test_jsonSerialization( self ):
        bbmlFile   = 'TestSerializeToJSON.bbml'
        cmlFile    = 'TestSerializeToJSON.cml'
        outputFile = 'serialized.json'


        # run RTBOS to create JSON-serialized output file
        Any.requireIsFileNonEmpty( bbmlFile )
        cmd    = 'RTBOS.sh %s' % bbmlFile
        output = StringIO() if Any.getDebugLevel() <= 3 else None
        FastScript.remove( outputFile )             # ensure it's not there
        FastScript.execProgram( cmd, stdout=output, stderr=output )


        # verify output file has been created
        Any.requireIsFileNonEmpty( outputFile )


        # check result
        data       = json.load( open( outputFile ) )
        arrayBlock = data['bBDMArrayBlockF32']

        self.assertEqual( arrayBlock['m_dims'], 1 )
        self.assertEqual( arrayBlock['m_totalSize'], 2 )
        self.assertEqual( len(arrayBlock['m_data']), arrayBlock['m_totalSize'] )
        self.assertEqual( arrayBlock['elementsPerDimension']['elementsPerDimension'],
                          [ 2, 0, 0, 0 ] )

        for block in arrayBlock['m_data']:
            self.assertEqual( block['owner'], 1 )
            self.assertEqual( len( block['data']['data'] ), 16 )
            self.assertEqual( block['size']['width'], 4 )
            self.assertEqual( block['size']['height'], 4 )


        # clean-up
        FastScript.remove( 'LibIndex' )
        FastScript.remove( outputFile )
        FastScript.remove( cmlFile )


if __name__ == '__main__':
    unittest.main()
