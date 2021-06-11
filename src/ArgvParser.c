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


#include <ArgvParser.h>

#include <string.h>


#define ARGVPARSER_INIT     (0x900dbfff)
#define ARGVPARSER_VALID    (0x63aaf209)
#define ARGVPARSER_INVALID  (0x40a6c83e)


ArgvParser *ArgvParser_new( void )
{
    ArgvParser *self = (ArgvParser *)NULL;

    self = ANY_TALLOC( ArgvParser );
    ANY_REQUIRE( self );

    return self;
}


int ArgvParser_init( ArgvParser *self )
{
    ANY_REQUIRE( self );

    Any_memset((void *)self, 0, sizeof( ArgvParser ));

    self->argc = -1;
    self->argv = NULL;
    self->descriptors = NULL;
    self->ignoreUnknownOptions = false;
    self->errorMessage[ 0 ] = 0;
    self->exampleCount = 0;
    self->valid = ARGVPARSER_INIT;

    return 0;
}


int ArgvParser_setup( ArgvParser *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT );

    if( self->argc < 0 || self->argv == NULL || self->descriptors == NULL)
    {
        return -1;
    }

    self->curArgIdx = -1;
    self->curOptIdx = ARGVPARSER_NO_OPTION;

    self->valid = ARGVPARSER_VALID;

    /* go to argument 0 (usually the filename of the executable) */
    ArgvParser_advance( self );

    if( ArgvParser_hasErrorOccurred( self ))
    {
        return -1;
    }

    return 0;
}


void ArgvParser_clear( ArgvParser *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT
                 || self->valid == ARGVPARSER_VALID );

    Any_memset((void *)self, 0, sizeof( ArgvParser ));

    self->valid = ARGVPARSER_INVALID;
}


void ArgvParser_delete( ArgvParser *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


void ArgvParser_setArguments( ArgvParser *self, int argc, char *argv[] )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT );
    ANY_REQUIRE( argc >= 0 );
    ANY_REQUIRE( argv );

    self->argc = argc;
    self->argv = argv;
}


int ArgvParser_getNumberOfArguments( const ArgvParser *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT
                 || self->valid == ARGVPARSER_VALID );

    return self->argc;
}


const char *ArgvParser_getArgument( const ArgvParser *self, int idx )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT
                 || self->valid == ARGVPARSER_VALID );
    ANY_REQUIRE( 0 <= idx && idx < self->argc );

    return self->argv[ idx ];
}


void ArgvParser_setOptionDescriptors( ArgvParser *self,
                                      ArgvParserOptionDescriptor *dtors )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT );
    ANY_REQUIRE( dtors );

    self->descriptors = dtors;
}


bool ArgvParser_AreUnknownOptionsIgnored( const ArgvParser *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT
                 || self->valid == ARGVPARSER_VALID );

    return self->ignoreUnknownOptions;
}


void ArgvParser_setIgnoreUnknownOptions( ArgvParser *self, bool flag )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT
                 || self->valid == ARGVPARSER_VALID );
    ANY_OPTIONAL( flag );

    self->ignoreUnknownOptions = flag;
}


int ArgvParser_initAndSetup( ArgvParser *self,
                             int argc, char *argv[],
                             ArgvParserOptionDescriptor *dtors )
{
    int result = ArgvParser_init( self );

    if( result != 0 )
    {
        return result;
    }

    ArgvParser_setArguments( self, argc, argv );
    ArgvParser_setOptionDescriptors( self, dtors );

    result = ArgvParser_setup( self );

    if( result != 0 )
    {
        ArgvParser_clear( self );
    }

    return result;
}


bool ArgvParser_hasErrorOccurred( const ArgvParser *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT
                 || self->valid == ARGVPARSER_VALID );

    return self->errorMessage[ 0 ] != 0;
}


const char *ArgvParser_getErrorMessage( const ArgvParser *self )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_INIT
                 || self->valid == ARGVPARSER_VALID );

    return self->errorMessage;
}


bool ArgvParser_advance( ArgvParser *self )
{
    const char *currentArg = NULL;
    int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_VALID );

    self->curParameter = NULL;

    ++self->curArgIdx;

    if( self->curArgIdx >= self->argc )
    {
        /* end of command line reached */
        self->curOptIdx = ARGVPARSER_EOL;
        return false;
    }

    self->curOptIdx = ARGVPARSER_NO_OPTION;
    self->curParameter = NULL;

    currentArg = self->argv[ self->curArgIdx ];

    if( !( currentArg != NULL && currentArg[ 0 ] == '-' && currentArg[ 1 ] != 0 ))
    {
        /* no option, other argument */
        self->curParameter = currentArg;
        return true;
    }

    /* we have found an option, check that it is valid */
    if( currentArg[ 1 ] != '-' )
    {
        char option = currentArg[ 1 ];

        if( currentArg[ 2 ] != 0 )
        {
            Any_snprintf( self->errorMessage,
                          sizeof( self->errorMessage ) - 1,
                          "more than one option in %s",
                          self->argv[ self->curArgIdx ] );

            self->curOptIdx = ARGVPARSER_ERROR;
            return false;
        }

        for( i = 0;
             !( self->descriptors[ i ].shortName == 0
                && self->descriptors[ i ].longName == NULL);
             ++i )
        {
            if( option == self->descriptors[ i ].shortName )
            {
                self->curOptIdx = i;
                break;
            }
        }
    }
    else /* currentArg starts with "--" */
    {
        const char *option = currentArg + 2;

        for( i = 0;
             !( self->descriptors[ i ].shortName == 0
                && self->descriptors[ i ].longName == NULL);
             ++i )
        {
            const char *longName = NULL;

            longName = self->descriptors[ i ].longName;

            if( longName != NULL && !strcmp( option, longName ))
            {
                self->curOptIdx = i;
                break;
            }
        }
    }

    if( self->curOptIdx >= 0 )
    {
        /* known option found */
        if( self->descriptors[ self->curOptIdx ].hasParameter
            != ARGVPARSER_NO_PARAMETER )
        {
            if( ++( self->curArgIdx ) < self->argc )
            {
                self->curParameter = self->argv[ self->curArgIdx ];
            }
            else
            {
                Any_snprintf( self->errorMessage,
                              sizeof( self->errorMessage ) - 1,
                              "argument required for %s",
                              self->argv[ self->curArgIdx - 1 ] );

                self->curOptIdx = ARGVPARSER_ERROR;
                return false;
            }
        }
    }
    else
    {
        /* unkown option found */
        if( self->ignoreUnknownOptions )
        {
            self->curParameter = currentArg;
        }
        else
        {
            Any_snprintf( self->errorMessage,
                          sizeof( self->errorMessage ) - 1,
                          "unknown option %s",
                          self->argv[ self->curArgIdx ] );

            self->curOptIdx = ARGVPARSER_ERROR;
            return false;
        }
    }

    return true;
}


int ArgvParser_getCurrentArgument( const ArgvParser *self,
                                   char *shortName,
                                   const char **longName,
                                   const char **parameter )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_VALID );
    ANY_OPTIONAL( shortName );
    ANY_OPTIONAL( longName );
    ANY_OPTIONAL( parameter );

    if( shortName )
    {
        if( self->curOptIdx >= 0 )
        {
            *shortName = self->descriptors[ self->curOptIdx ].shortName;
        }
        else
        {
            *shortName = 0;
        }
    }

    if( longName )
    {
        if( self->curOptIdx >= 0 )
        {
            *longName = self->descriptors[ self->curOptIdx ].longName;
        }
        else
        {
            *longName = NULL;
        }
    }

    if( parameter )
    {
        *parameter = self->curParameter;
    }

    return self->curOptIdx;
}


void ArgvParser_displayOptionHelp( const ArgvParser *self, int indentation )
{
    ArgvParserOptionDescriptor *descriptor = NULL;
    int i = 0;
    int spaces = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_VALID );
    ANY_OPTIONAL( indentation );

    descriptor = self->descriptors;

    while( !( descriptor->shortName == 0 && descriptor->longName == NULL))
    {
        const char *paramName = "param";
        spaces = 25;

        for( i = 0; i < indentation; ++i )
        {
            printf( " " );
        }

        if( descriptor->shortName != 0 )
        {
            printf( "-%c", descriptor->shortName );

            if( descriptor->longName != NULL)
            {
                printf( "  " );
            }
        }

        if( descriptor->longName != NULL)
        {
            printf( "--%s", descriptor->longName );
            spaces -= Any_strlen( descriptor->longName );
        }

        if( descriptor->hasParameter != ARGVPARSER_NO_PARAMETER )
        {
            if( descriptor->helpParameterName != NULL)
            {
                paramName = descriptor->helpParameterName;
                spaces -= Any_strlen( descriptor->helpParameterName );
            }

            printf( " %s", paramName );
        }
        else
        {
            spaces++;  /* +1 to be equal with the one in the previous:  printf( " %s", paramName );  */
        }

        while( spaces > 0 )
        {
            printf( " " );
            spaces--;
        }

        if( descriptor->helpText != NULL)
        {
            printf( "%s", descriptor->helpText );
        }

        printf( "\n" );

        ++descriptor;
    }
}


void ArgvParser_addExample( ArgvParser *self, const char *command )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_VALID );
    ANY_REQUIRE_MSG( self->exampleCount < 10, "max. 10 examples allowed" );
    ANY_REQUIRE_MSG( Any_strlen( command ) < 80, "example exceeds 70 characters" );

    Any_strncpy( self->examples[ self->exampleCount ], command, 79 );
    self->exampleCount++;
}


void ArgvParser_showHelp( ArgvParser *self,
                          const char *programDescription,
                          const char *usageSyntax,
                          const char *bugtrackURL )
{
    unsigned int i = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( self->valid == ARGVPARSER_VALID );
    ANY_REQUIRE( programDescription );
    ANY_REQUIRE( usageSyntax );
    ANY_REQUIRE_MSG( Any_strlen( usageSyntax ) < 70, "usageSyntax exceeds 70 characters" );
    ANY_REQUIRE( bugtrackURL );
    ANY_REQUIRE_MSG( Any_strlen( bugtrackURL ) < 50, "bugtrackURL exceeds 50 characters" );

    printf( "\n%s\n", programDescription );

    printf( "\nUsage:    %s\n", usageSyntax );

    printf( "Options:\n" );
    ArgvParser_displayOptionHelp( self, 10 );  /* 10: indentation */

    if( self->exampleCount > 0 )
    {
        printf( "\nExamples:\n" );

        for( i = 0; i < self->exampleCount; i++ )
        {
            printf( "          %s\n", self->examples[ i ] );
        }
    }

    printf( "\nPlease report bugs on JIRA (%s).\n", bugtrackURL );
}


/* EOF */
