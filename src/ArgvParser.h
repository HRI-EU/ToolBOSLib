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


#ifndef ARGVPARSER_H
#define ARGVPARSER_H


/*!
 * \page ArgvParser_About Argv commandline parser
 *
 * The ArgvParser (ArgvParser.h) library provides a parser for command line
 * arguments given to the main program.
 *
 * <h3>Example:</h3>
 *
 * Given a main function of a C program:
 * \code
 * int main( int argc, char* argv[] )
 * \endcode
 *
 * You can pass \c argc and \c argv to an instance of ArgvParser, which allows
 * to parse the command line arguments easily and to identify options. <br>
 * We support options with short names, i.e., options like <tt>"-s"</tt> that
 * consist of <tt>"-"</tt> and a single character, and options with long names
 * like <tt>"--long-name"</tt>. The latter consist of <tt>"--"</tt> and a
 * character string which does not contain spaces. A single option can have
 * both a short and a long name. <br>
 * The module also supports the generation of help messages that describe all
 * possible command line options.
 *
 * The normal call to define the parameters is:
 * \code
 * int main( int argc, char* argv[] )
 * {
 *   ...
 *   ArgvParser_setArguments( parser, argc, argv );
 *   ...
 * }
 * \endcode
 */


#include <Any.h>
#include <Base.h>


#if defined(__cplusplus)
extern "C" {
#endif


#define ARGVPARSER_NO_PARAMETER       (  0 )
#define ARGVPARSER_PARAMETER_REQUIRED (  1 )
#define ARGVPARSER_NO_OPTION          ( -1 )
#define ARGVPARSER_ERROR              ( -2 )
#define ARGVPARSER_EOL                ( -3 )
#define ARGVPARSER_UNKNOWN_OPTION     ( -4 )


typedef struct ArgvParserOptionDescriptor
{
    char shortName;
    const char *longName;
    int hasParameter;
    const char *helpParameterName;
    const char *helpText;
}
ArgvParserOptionDescriptor;


typedef struct ArgvParser
{
    unsigned long valid;

    int argc;
    char **argv;

    ArgvParserOptionDescriptor *descriptors;
    BaseBool ignoreUnknownOptions;

    char errorMessage[64];

    int curArgIdx;
    int curOptIdx;
    const char *curParameter;

    unsigned int exampleCount;
    char examples[10][80];
}
ArgvParser;


ArgvParser *ArgvParser_new( void );

int ArgvParser_init( ArgvParser *self );

int ArgvParser_setup( ArgvParser *self );

void ArgvParser_clear( ArgvParser *self );

void ArgvParser_delete( ArgvParser *self );


/*!
 * \brief Specify the arguments to parse.
 * \param self Pointer to an ArgvParser object.
 * \param argc Number of arguments.
 * \param argv Array of arguments, must contain \a argc entries, indexed from
 *             zero.
 *
 * Usually you use for \a argc and \a argv the values from the call to the main
 * function of your program:
 * \code
 * int main( int argc, char* argv[] )
 * {
 *   ...
 *   ArgvParser_setArguments( parser, argc, argv );
 *   ...
 * }
 * \endcode
 *
 * This method should only be called between ArgvParser_init() and
 * ArgvParser_setup().
 */
void ArgvParser_setArguments( ArgvParser *self, int argc, char *argv[] );

int ArgvParser_getNumberOfArguments( const ArgvParser *self );

const char *ArgvParser_getArgument( const ArgvParser *self, int idx );

/*!
 * \brief Specify the arguments to parse.
 * \param self Pointer to an ArgvParser object.
 * \param dtors List of ArgvParserOptionDescriptor, terminated by an all-zero
 *              struct.
 *
 * We give an example for an option list that can passed as argument \a dtors:
 * \code
 * static ArgvParserOptionDescriptor optionDescriptors[] =
 *   {
 *     { 'h', "help", ARGVPARSER_NO_PARAMETER, NULL,
 *       "display this help" },
 *
 *     { 's', NULL, ARGVPARSER_PARAMETER_REQUIRED, "par",
 *       "short option with parameter" },
 *
 *     { 0, "long", ARGVPARSER_PARAMETER_REQUIRED, "par",
 *       "long option with parameter" },
 *
 *     { 'b', "both", ARGVPARSER_NO_PARAMETER, NULL,
 *       "both short and long option, without parameter" },
 *
 *     { 0, NULL, 0, NULL, NULL }
 *   };
 * \endcode
 * A corresponding command line could look as follows:
 * \code
 * my-program -b -s arg1 --long arg2
 * \endcode
 * An option descriptor consists of the following entries.
 * First, we have a \c char that describes the short name of the option, short
 * options must be preceeded by a single <tt>"-"</tt> on the command line.
 * If you do not want to allow a short name for a certain option use \c 0.
 * Then we specify the the long name as <tt>const char*</tt>, long options are
 * arguments preceeded by <tt>"--"</tt>.
 * The third entry in the option struct determines whether this option requires
 * a parameter or not. If an option requires a paramter the argument in
 * <tt>argv[]</tt> that follows the option is considered as argument for that
 * option. In case, you do not want to provide a long name, use a \c NULL
 * pointer here.
 * If an option takes a parameter you should specify in the forth entry of the
 * descriptor a name to be used for the parameter when help text is generated.
 * The fifth and final contains the help text that will be displayed for that
 * option when ArgvParser_displayOptionHelp() is called.
 * For the example above we would obtain the following help message:
\verbatim
 -h / --help: display this help
 -s par: short option with parameter
 --long par: long option with parameter
 -b / --both: both short and long option, without parameter
\endverbatim
 *
 * \see ArgvParser_initAndSetup
 *
 * \attention
 * This method should only be called between ArgvParser_init() and
 * ArgvParser_setup().
 */
void ArgvParser_setOptionDescriptors( ArgvParser *self,
                                      ArgvParserOptionDescriptor *dtors );

bool ArgvParser_AreUnknownOptionsIgnored( const ArgvParser *self );

/*!
 * \brief Specify if unknown options should be ignored.
 * \param self Pointer to an ArgvParser object.
 * \param flag Flag that indicates if unknown options should be ignored.
 *
 * By default, unknown options are considered as error. If this \a flag is set
 * to \c false then an unknown option is not considered as error anymore.
 * When the parser encounters an unknown option ArgvParser_getCurrentArgument()
 * will return #ARGVPARSER_UNKNOWN_OPTION.
 */
void ArgvParser_setIgnoreUnknownOptions( ArgvParser *self, bool flag );

/*!
 * \brief Initialize an ArgvParser object.
 * \param self Pointer to an ArgvParser object.
 * \param argc Parameter passed to ArgvParser_setArguments().
 * \param argv Parameter passed to ArgvParser_setArguments().
 * \param dtors Parameter passed to ArgvParser_setOptionDescriptors().
 * \return Zero upon success, negative error code otherwise.
 *
 * This function allows to initialize an ArgvParser object with one function
 * call. It is a convenience function, which does basically the same as calling
 * ArgvParser_init(), ArgvParser_setArguments(),
 * ArgvParser_setOptionDescriptors(), and finally ArgvParser_setup().<br>
 * If this function fails you must not apply ArgvParser_clear() to the object.
 * (Only call ArgvParser_delete() if appropriate.)
 */
int ArgvParser_initAndSetup( ArgvParser *self,
                             int argc, char *argv[],
                             ArgvParserOptionDescriptor *dtors );

bool ArgvParser_hasErrorOccurred( const ArgvParser *self );

const char *ArgvParser_getErrorMessage( const ArgvParser *self );

/*!
 * \brief Parse the next argument.
 * \param self Pointer to an ArgvParser object.
 * \return False if an error has occurred or the parser has moved behind the
 *         last argument, true otherwise.
 *
 * In order to obtain information about the current argument use
 * ArgvParser_getCurrentArgument().
 */
bool ArgvParser_advance( ArgvParser *self );

/*!
 * \brief Get information about the current argument.
 * \param self Pointer to an ArgvParser object.
 * \param shortName Address of the <tt>char</tt> for storing the short name.
 * \param longName  Address of the <tt>const char*</tt> for storing the long name.
 * \param parameter Address of the <tt>const char*</tt> for storing the parameter.
 * \return If the current argument contains a valid option then the index of
 *         this option in the option descriptors is returned.
 *         Otherwise the result is a negative special value (see below).
 *
 * The arguments \a shortName, \a longName and \a parameter are all optional,
 * i.e., if the specified address is NULL the respective argument is ignored.
 * <br>
 * If the last call to ArgvParser_advance() has returned \c true the result of
 * ArgvParser_getCurrentArgument() is either a non-negative index or a
 * negative value. If the result is a non-negative index \e i then the
 * current argument contains the \e ith option from the option
 * descriptors specified by ArgvParser_setOptionDescriptors().
 * In that case \a shortName and \a longName are set to the short and the long
 * name of the option, and \a parameter is set to the parameter if the option
 * requires a parameter.<br>
 * If the result is negative it can be one of the following:
 * - #ARGVPARSER_NO_OPTION: The current argument does not contain an option.
 *   This argument is stored in \a parameter.
 * - #ARGVPARSER_UNKNOWN_OPTION: The current argument contains an unknown
 *   option. The full argument (including <tt>"-"</tt> signs) is stored in
 *   \a parameter.
 *   (This result can only occur if ArgvParser_setIgnoreUnknownOptions() has
 *   been called earlier.)
 *
 * If the last call to ArgvParser_advance() has returned \c false the result is
 * one of these:
 * - #ARGVPARSER_ERROR: Indicates an error.
 * - #ARGVPARSER_EOL: ArgvParser_advance() has been called after the last
 *   argument in <tt>argv[]</tt> has been processed.
 */
int ArgvParser_getCurrentArgument( const ArgvParser *self,
                                   char *shortName,
                                   const char **longName,
                                   const char **parameter );

/*!
 * \brief Display a help text describing all options.
 * \param self Pointer to an ArgvParser object.
 * \param indentation Number of spaces to print before each line of text.
 * \param bugtrackURL Support URL (such as JIRA issue tracking system etc.)
 *
 * This function prints a help message that describes all valid options to
 * \c stdout.
 */
void ArgvParser_displayOptionHelp( const ArgvParser *self, int indentation );

void ArgvParser_addExample( ArgvParser *self, const char *command );

void ArgvParser_showHelp( ArgvParser *self,
                          const char *programDescription,
                          const char *usageSyntax,
                          const char *bugtrackURL );


#if defined(__cplusplus)
}
#endif


#endif


/* EOF */
