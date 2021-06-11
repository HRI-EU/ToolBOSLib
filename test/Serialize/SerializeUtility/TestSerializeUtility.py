#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  Copyright (c) Honda Research Institute Europe GmbH
#
#  This file is part of ToolBOSLib.
#
#  ToolBOSLib is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ToolBOSLib is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with ToolBOSLib. If not, see <http://www.gnu.org/licenses/>.
#
#



import os
import random
import re
import tempfile
import unittest

from ToolBOSCore.Platforms          import Platforms
from ToolBOSCore.Util               import FastScript
from ToolBOSCore.Util               import Any
from ToolBOSCore.Util.VersionCompat import StringIO


class TestSerializeUtility( unittest.TestCase ):

    def setUp( self ):
        # for better human overview on test progress, we only mind warnings
        # and critical messages (suppress regular progress info)
        if not FastScript.getEnv( 'VERBOSE' ) == 'TRUE':
            Any.setDebugLevel( 1 )


    def test_serializeUtility_convert( self ):
        tcRoot   = FastScript.getEnv( 'TOOLBOSCORE_ROOT' )
        platform = Platforms.getHostPlatform()
        exe      = os.path.join( tcRoot, 'bin', platform, 'ConvertSerializedData' )
        inFile   = 'BBDMArrayBlockF32-binary.ser'
        outFile  = tempfile.mkstemp( prefix='test-' )[1]
        cmd      = '%s -i %s -f Ascii -o %s' % ( exe, inFile, outFile )
        tmp      = StringIO()

        try:
            FastScript.execProgram( cmd, stdout=tmp, stderr=tmp )
        except OSError:
            raise RuntimeError( "Please compile this package first!" )

        output  = tmp.getvalue()

        # typical error could be:
        #
        # [... SerializeUtility.c:809 Error] Could not load data library
        # 'BBDMArrayBlockF32' (libBBDMArrayBlockF32.so): Reason
        # 'libBBDMArrayBlockF32.so: cannot open shared object file:
        # No such file or directory'
        #
        # [... SerializeUtility.c:653 Error] The input file
        # 'test/miscInputFiles/BBDMArrayBlockF32-binary.ser'
        # does not contain valid data


        # check result
        self.assertTrue( output.find( 'libBBDMArrayBlockF32.so'     ) == -1 )
        self.assertTrue( output.find( 'Could not load data library' ) == -1 )
        self.assertTrue( output.find( 'No such file or directory'   ) == -1 )
        self.assertTrue( output.find( 'Aborting serialization function' ) > 0 ) # at EOF
        self.assertTrue( output.find( 'does not contain valid data' ) == -1 )

        Any.requireIsFileNonEmpty( outFile )
        content = FastScript.getFileContent( outFile )

        # there must be 3x "HRIS" in the ASCII representation of this file
        matches = re.findall( 'HRIS', content )
        self.assertTrue( len(matches) == 3 )


        # clean-up
        FastScript.remove( outFile )


    def test_serializeUtility_createSerializedData( self ):
        random.seed( self )

        tcRoot   = FastScript.getEnv( 'TOOLBOSCORE_ROOT' )
        platform = Platforms.getHostPlatform()
        exe      = os.path.join( tcRoot, 'bin', platform, 'CreateSerializedData' )
        count   = random.randint( 3, 20 )
        outFile = tempfile.mkstemp( prefix='test-' )[1]
        cmd     = '%s -t BBDMArrayBlockF32 -c %d -r -f Ascii -o %s' % ( exe, count, outFile )
        tmp     = StringIO()

        try:
            FastScript.execProgram( cmd, stdout=tmp, stderr=tmp )
        except OSError:
            raise RuntimeError( "Please compile this package first!" )

        output  = tmp.getvalue()

        # typical error could be:
        #
        # [... SerializeUtility.c:809 Error] Could not load data library
        # 'BBDMBaseF32' (libBBDMBaseF32.so): Reason 'libBBDMBaseF32.so:
        # cannot open shared object file: No such file or directory'
        #
        # SerializeUtility.c:1083 BBDMBaseF32: unsupported datatype
        # (BBDMBaseF32_new() not found)


        # check result
        self.assertTrue( output.find( 'libBBDMArrayBlockF32.so'        ) == -1 )
        self.assertTrue( output.find( 'cannot open shared object file' ) == -1 )
        self.assertTrue( output.find( 'No such file or directory'      ) == -1 )
        self.assertTrue( output.find( 'unsupported datatype'           ) == -1 )

        Any.requireIsFileNonEmpty( outFile )
        content = FastScript.getFileContent( outFile )

        # we must find "count"-times the keyword "HRIS" inside the file
        matches = re.findall( 'HRIS', content )
        self.assertTrue( len(matches) == count )


        # clean-up
        FastScript.remove( outFile )


    def test_serializeUtility_printSerializedData( self ):
        tcRoot   = FastScript.getEnv( 'TOOLBOSCORE_ROOT' )
        platform = Platforms.getHostPlatform()
        exe      = os.path.join( tcRoot, 'bin', platform, 'PrintSerializedData' )
        inFile  = os.path.join( 'BBDMArrayBlockF32-binary.ser' )
        outFile = os.path.join( 'BBDMArrayBlockF32-ascii.ser' )
        cmd     = '%s -f %s' % ( exe, inFile )
        tmp     = StringIO()

        Any.requireIsFileNonEmpty( inFile )
        Any.requireIsFileNonEmpty( outFile )

        try:
            # do not redirect stderr to file, otherwise we can't compare with
            # the expected output (ANY_LOG timestamps would differ), there is
            # at least stderr-message "Aborting serialization function" at EOF
            FastScript.execProgram( cmd, stdout=tmp )
        except OSError:
            raise RuntimeError( "Please compile this package first!" )

        expected = FastScript.getFileContent( outFile )
        output   = tmp.getvalue()

        # typical error could be:
        #
        # [... SerializeUtility.c:809 Error] Could not load data library
        # 'BBDMArrayBlockF32' (libBBDMArrayBlockF32.so): Reason
        # 'libBBDMArrayBlockF32.so: cannot open shared object file:
        # No such file or directory'
        #
        # [... SerializeUtility.c:591 Error] The input file
        # 'test/miscInputFiles/BBDMArrayBlockF32-binary.ser' does not
        # contain valid data


        # check result
        self.assertTrue( output.find( 'libBBDMArrayBlockF32.so'        ) == -1 )
        self.assertTrue( output.find( 'cannot open shared object file' ) == -1 )
        self.assertTrue( output.find( 'No such file or directory'      ) == -1 )
        self.assertTrue( output.find( 'does not contain valid data'    ) == -1 )

        self.assertEqual( expected, output )


if __name__ == '__main__':
    unittest.main()


# EOF
