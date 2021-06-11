/*
 *  Copyright (c) Honda Research Institute Europe GmbH
 *
 *  This file is part of ToolBOSLib.
 *
 *  ToolBOSLib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ToolBOSLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ToolBOSLib. If not, see <http://www.gnu.org/licenses/>.
 */


/*--------------------------------------------------------------------------*/
/* Includes                                                                 */
/*--------------------------------------------------------------------------*/


#include <stdlib.h>
#include <ESC.h>
#include <CuTest.h>


/*--------------------------------------------------------------------------*/
/* HowTo                                                                    */
/*--------------------------------------------------------------------------*/
//
//  1.  Write testcase functions
//      (see examples below)
//
//  2.  List each testcase in main() function
//
//  3.  Compile package and run test program like:
//      $ RunFromSourceTree.sh ./test/\${MAKEFILE_PLATFORM}/unittest
//
//
//----------------------------------------------------------------------------
//
//
//  Assertions provided by CuTest:
//
//      CuFail( testcase, msg );
//      CuAssert( testcase, msg, cond );
//      CuAssertTrue( testcase, cond );
//
//      CuAssertStrEquals( testcase, expected, actual );
//      CuAssertStrEquals_Msg( testcase, msg, expected, actual );
//      CuAssertIntEquals( testcase, expected, actual );
//      CuAssertIntEquals_Msg( testcase, msg, expected, actual );
//      CuAssertDblEquals( testcase, expected, actual, maxDelta );
//      CuAssertDblEquals_Msg( testcase, msg, expected, actual, maxDelta );
//      CuAssertPtrEquals( testcase, expected, actual );
//      CuAssertPtrEquals_Msg( testcase, msg, expected, actual );
//
//      CuAssertPtrNotNull( testcase, ptr );
//      CuAssertPtrNotNullMsg( testcase, msg, ptr );

/*--------------------------------------------------------------------------*/
/* Error codes                                                              */
/*--------------------------------------------------------------------------*/

#define ESC_TEST_DIVIDE_DIV_BY_ZERO (UINT64_C( 0x10 ))
#define ESC_TEST_FOO_DIVIDE_FAILED  (UINT64_C( 0x11 ))

/*--------------------------------------------------------------------------*/
/* Functions to be tested                                                   */
/*--------------------------------------------------------------------------*/

ESCStatus divide( int a, int b, float *result )
{
    ESC_RETURN_ON_NULL_PTR( result, ESC_GENERIC_ARGUMENT_IS_NULL );
    ESC_RETURN_ON( b == 0, ESC_TEST_DIVIDE_DIV_BY_ZERO );

    *result = (float)(a) / (float)(b);

    return ESC_NO_ERROR;
}

ESCStatus foo( int a, int b )
{
    float     result;
    ESCStatus status;

    status = divide( a, b, &result );
    ESC_RETURN_ON_ERROR( status, ESC_TEST_FOO_DIVIDE_FAILED );

    printf( "%d / %d = %f\n", a, b, result );

    return ESC_NO_ERROR;
}

/*--------------------------------------------------------------------------*/
/* Testcases                                                                */
/*--------------------------------------------------------------------------*/


void Unittest_normal( CuTest *testcase )
{
    ESCStatus status;

    status = foo( 1, 2 );
    CuAssertSliEquals_Msg( testcase,
                           "Return value of foo(1,2)",
                           ESC_NO_ERROR,
                           status );
}


void Unittest_error( CuTest *testcase )
{
    ESCStatus status;

    printf( "Please ignore error messages in the following block\n" );
    printf( "---------------------------------------------------\n" );
    status = foo( 1, 0 );
    CuAssertSliEquals_Msg( testcase,
                           "Return value of foo(1,0)",
                           (ESC_TEST_DIVIDE_DIV_BY_ZERO << 8) | (ESC_TEST_FOO_DIVIDE_FAILED),
                           status );

    status = divide( 3, 4, NULL );
    CuAssertSliEquals_Msg( testcase,
                           "Return value of divide(3,4,NULL)",
                           ESC_GENERIC_ARGUMENT_IS_NULL,
                           status );
    printf( "---------------------------------------------------\n" );
}



/*--------------------------------------------------------------------------*/
/* Boilerplate main function                                                */
/*--------------------------------------------------------------------------*/


int main( int argc, char* argv[] )
{
    CuSuite  *suite  = CuSuiteNew();
    CuString *output = CuStringNew();

    (void)argc;
    (void)argv;

    /* TODO: list each testcase here */
    SUITE_ADD_TEST( suite, Unittest_normal );
    SUITE_ADD_TEST( suite, Unittest_error );

    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );
    CuSuiteDetails( suite, output );
    fprintf( stderr, "%s\n", output->buffer );

    return EXIT_SUCCESS;
}


/* EOF */
