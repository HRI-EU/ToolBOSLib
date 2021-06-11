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


/* all platforms and compilers */

#include <errno.h>

#include <Any.h>
#include <BaseMath.h>
#include <FileSystem.h>
#include <UString.h>


/* On POSIX only. */
#if !defined(__windows__)

#include <sys/sysmacros.h>
#include <fnmatch.h>

#endif

/* On Win32 only. */
#if defined(__windows__)
#include <direct.h>
#endif

/* On all except Win32/MSVC. */
#if !defined(__msvc__)

#include <libgen.h>
#include <dirent.h>

#endif


#if defined(__msvc__)
#include <WinBase.h>
#endif


/*! \brief Reference length for a file line. */
#define FILESYSTEM_FILELINE_LENGTH 256

/*! \brief Length of a file buffer. */
#define FILESYSTEM_FILEBUFFER_LENGTH 4096

/*! \brief String terminating character. */
#define FILESYSTEM_NUL_CHARACTER '\0'

/*! \brief Maximum number of entries in a list. */
#define FILESYSTEM_DIRENTRIES_COUNT 128

/*! \brief Constant string representing the current directory. */
#define FILESYSTEM_CURRENT_FOLDER "."

/*! \brief Macro to cover the recurring patter of checking an handle. */
#define FILESYSTEM_BREAK_IF( __handle, __cfr )\
        {\
          if( __cfr == __handle )\
          {\
              break;\
          }\
        }\

/*! \brief Macro covering the recurring patter of opening a file. */
#define FILESYSTEM_RETURN_IF_FOPEN_FAILS( __filePath, __mode, __handle )\
        do\
        {\
          __handle = fopen( __filePath, __mode );\
          if( __handle == NULL )\
          {\
            return FILESYSTEM_STATUS_UNABLETOOPEN;\
          }\
        }\
        while(0)

#ifndef S_ISREG
#define S_ISREG( x ) (((x) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISDIR
#define S_ISDIR( x ) (((x) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISCHR
#define S_ISCHR( x ) (((x) & S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISBLK
# if defined( S_IFBLK )
#   define S_ISBLK(x) (((x) & S_IFMT) == S_IFBLK)
# else
#   define S_ISBLK( x ) 0
# endif
#endif

#ifndef S_ISFIFO
# if defined( S_IFFIFO )
#   define S_ISFIFO(x) (((x) & S_IFMT) == S_IFFIFO)
# else
#   define S_ISFIFO( x ) 0
# endif
#endif

#ifndef S_ISLNK
# ifdef S_IFLNK
#   define S_ISLNK(x) (((x) & S_IFMT) == S_IFLNK)
# else
#   define S_ISLNK( x ) 0
# endif
#endif

#ifndef S_IXUSR
#define S_IXUSR _S_IEXEC
#endif

#ifndef S_IWUSR
#define S_IWUSR _S_IWRITE
#endif


/*!
 * \brief Extend a path to absolute if it is relative.
 *
 * \param path Path to be extended;
 * \param pathSize Size of the buffer.
 *
 * \return  0 in case of success;
 *         -1
 */
BaseI32 FileSystem_implodeIfRelative( char *path, BaseUI32 pathSize );

#if defined( __windows__ )
BaseI32 FileSystem_addTrailingStar( char *adaptedPath,
                                    const char *path,
                                    const BaseUI32 adaptedPathSize );
#endif


BaseI32 FileSystem_getSize( const char *path )
{
    BaseI32     retVal = 0;
    struct stat stats;

    ANY_REQUIRE( path );

    if( stat( path, &stats ) != 0 )
    {
        retVal = FILESYSTEM_STATUS_UNABLETOGETSTATS;
    }
    else
    {
        retVal = stats.st_size;
    }

    return retVal;
}


#if defined(__windows__)
BaseI32 FileSystem_compareDirectories( const char *first,
                                       const char *second )
{
    BaseI32         retVal                            = FILESYSTEM_STATUS_SUCCESS;
    HANDLE          dirHandle1                        = INVALID_HANDLE_VALUE;
    HANDLE          dirHandle2                        = INVALID_HANDLE_VALUE;
    char            fullPath1[FILESYSTEM_PATH_LENGTH] = "";
    char            fullPath2[FILESYSTEM_PATH_LENGTH] = "";
    BaseUI32        len1                              = 0;
    BaseUI32        len2                              = 0;
    BaseUI32        dirNameLen1                       = 0;
    BaseUI32        dirNameLen2                       = 0;
    WIN32_FIND_DATA dirEntry1;
    WIN32_FIND_DATA dirEntry2;

    ANY_REQUIRE( first );
    ANY_REQUIRE( second );

    if( FileSystem_isDirectory( first ) && FileSystem_isDirectory( second ) )
    {
        dirHandle1 = FindFirstFile( first, &dirEntry1 );

        if( INVALID_HANDLE_VALUE != dirHandle1 )
        {
            dirHandle2 = FindFirstFile( second, &dirEntry2 );

            if( INVALID_HANDLE_VALUE != dirHandle2 )
            {
                len1 = Any_strlen( first );
                len2 = Any_strlen( second );

                do
                {
                    FILESYSTEM_BREAK_IF( dirHandle1, INVALID_HANDLE_VALUE );
                    FILESYSTEM_BREAK_IF( dirHandle2, INVALID_HANDLE_VALUE );

                    dirNameLen1 = Any_strlen( dirEntry1.cFileName );
                    dirNameLen2 = Any_strlen( dirEntry2.cFileName );
                    if( ( 0 == dirNameLen1 ) && ( 0 == dirNameLen2 ) )
                    {
                        break;
                    }
                    else if( ( ( 0 == dirNameLen1 ) && ( 0 != dirNameLen2 ) ) ||
                             ( ( 0 != dirNameLen1 ) && ( 0 == dirNameLen2 ) ) )
                    {
                        retVal = FILESYSTEM_STATUS_DIRSAREDIFFERENT;
                        break;
                    }

                    if( ( ( Any_strcmp( ".", dirEntry1.cFileName ) == 0 ) &&
                          ( Any_strcmp( ".", dirEntry2.cFileName ) == 0 ) ) ||
                        ( ( Any_strcmp( "..", dirEntry1.cFileName ) == 0 ) &&
                          ( Any_strcmp( "..", dirEntry2.cFileName ) == 0 ) ) )
                    {
                        continue;
                    }

                    if( Any_strcmp( dirEntry1.cFileName, dirEntry2.cFileName ) == 0 )
                    {
                        if( len1 < FILESYSTEM_PATH_LENGTH )
                        {
                            Any_strncpy( fullPath1, first, FILESYSTEM_PATH_LENGTH - 1 );
                            fullPath1[ len1 ] = FILESYSTEM_NUL_CHARACTER;
                            FileSystem_implode( fullPath1,
                                                FILESYSTEM_PATH_LENGTH,
                                                dirEntry1.cFileName );
                        }
                        else
                        {
                            retVal = FILESYSTEM_STATUS_PATHSIZETOOBIG;
                        }

                        if( ( retVal == 0 ) && ( len2 < FILESYSTEM_PATH_LENGTH ) )
                        {
                            Any_strncpy( fullPath2, second, FILESYSTEM_PATH_LENGTH - 1 );
                            fullPath2[ len2 ] = FILESYSTEM_NUL_CHARACTER;
                            FileSystem_implode( fullPath2,
                                                FILESYSTEM_PATH_LENGTH,
                                                dirEntry2.cFileName );
                        }
                        else
                        {
                            retVal = FILESYSTEM_STATUS_PATHSIZETOOBIG;
                        }
                        retVal = FileSystem_compare( fullPath1, fullPath2 );
                    }
                    else
                    {
                        retVal = FILESYSTEM_STATUS_DIRSAREDIFFERENT;
                        break;
                    }  /* End if( Any_strcmp( dirEntry1... */
                }
                while( ( FindNextFile( dirHandle1, &dirEntry1 ) == TRUE ) &&
                       ( FindNextFile( dirHandle2, &dirEntry2 ) == TRUE ) &&
                       ( retVal == 0 ) );

                FindClose( dirHandle1 );
                FindClose( dirHandle2 );
            }
            else
            {
                FindClose( dirHandle1 );
                retVal = FILESYSTEM_STATUS_UNABLETOGETDIRENTRY;
            }
        }
        else
        {
            retVal = FILESYSTEM_STATUS_UNABLETOGETDIRENTRY;
        }
    }
    else
    {
        retVal = FILESYSTEM_STATUS_NOTADIRECTORY;  /* Items are not directories. */
    }

    return retVal;
}
#else


BaseI32 FileSystem_compareDirectories( const char *first,
                                       const char *second )
{
    BaseI32       retVal                            = FILESYSTEM_STATUS_SUCCESS;
    DIR           *dirHandle1                       = (DIR *)NULL;
    DIR           *dirHandle2                       = (DIR *)NULL;
    struct dirent *dirEntry1                        = (struct dirent *)NULL;
    struct dirent *dirEntry2                        = (struct dirent *)NULL;
    char          fullPath1[FILESYSTEM_PATH_LENGTH] = "";
    char          fullPath2[FILESYSTEM_PATH_LENGTH] = "";
    BaseUI32      len1                              = 0;
    BaseUI32      len2                              = 0;

    ANY_REQUIRE( first );
    ANY_REQUIRE( second );

    if( FileSystem_isDirectory( first ) && FileSystem_isDirectory( second ) )
    {
        dirHandle1 = opendir( first );

        if( NULL != dirHandle1 )
        {
            dirHandle2 = opendir( second );

            if( NULL != dirHandle2 )
            {
                len1 = Any_strlen( first );
                len2 = Any_strlen( second );

                do
                {
                    FILESYSTEM_BREAK_IF( dirHandle1, NULL );
                    dirEntry1 = readdir( dirHandle1 );

                    FILESYSTEM_BREAK_IF( dirHandle2, NULL );
                    dirEntry2 = readdir( dirHandle2 );

                    if( ( dirEntry1 == NULL ) && ( dirEntry2 == NULL ) )
                    {
                        break;
                    }
                    else if( ( ( dirEntry1 == NULL ) && ( dirEntry2 != NULL ) ) ||
                             ( ( dirEntry1 != NULL ) && ( dirEntry2 == NULL ) ) )
                    {
                        retVal = FILESYSTEM_STATUS_DIRSAREDIFFERENT;
                        break;
                    }

                    ANY_REQUIRE( dirEntry1 );
                    ANY_REQUIRE( dirEntry2 );
                    ANY_REQUIRE( dirEntry1->d_name );
                    ANY_REQUIRE( dirEntry2->d_name );

                    if( ( ( Any_strcmp( ".", dirEntry1->d_name ) == 0 ) &&
                          ( Any_strcmp( ".", dirEntry2->d_name ) == 0 ) ) ||
                        ( ( Any_strcmp( "..", dirEntry1->d_name ) == 0 ) &&
                          ( Any_strcmp( "..", dirEntry2->d_name ) == 0 ) ) )
                    {
                        continue;
                    }

                    if( Any_strcmp( dirEntry1->d_name, dirEntry2->d_name ) == 0 )
                    {
                        if( len1 < FILESYSTEM_PATH_LENGTH )
                        {
                            Any_strncpy( fullPath1, first, FILESYSTEM_PATH_LENGTH - 1 );
                            fullPath1[ len1 ] = FILESYSTEM_NUL_CHARACTER;
                            FileSystem_implode( fullPath1,
                                                FILESYSTEM_PATH_LENGTH,
                                                dirEntry1->d_name );
                        }
                        else
                        {
                            retVal = FILESYSTEM_STATUS_PATHSIZETOOBIG;
                        }

                        if( ( retVal == 0 ) && ( len2 < FILESYSTEM_PATH_LENGTH ) )
                        {
                            Any_strncpy( fullPath2, second, FILESYSTEM_PATH_LENGTH - 1 );
                            fullPath2[ len2 ] = FILESYSTEM_NUL_CHARACTER;
                            FileSystem_implode( fullPath2,
                                                FILESYSTEM_PATH_LENGTH,
                                                dirEntry2->d_name );
                        }
                        else
                        {
                            retVal = FILESYSTEM_STATUS_PATHSIZETOOBIG;
                        }

                        if( retVal != FILESYSTEM_STATUS_PATHSIZETOOBIG )
                        {
                            retVal = FileSystem_compare( fullPath1, fullPath2 );
                        }
                    }
                    else
                    {
                        retVal = FILESYSTEM_STATUS_DIRSAREDIFFERENT;
                        break;
                    }  /* End if( Any_strcmp( dirEntry1... */
                }
                while( ( dirEntry1 != NULL ) &&
                       ( dirEntry2 != NULL ) &&
                       ( retVal == 0 ) );

                closedir( dirHandle1 );
                closedir( dirHandle2 );
            }
            else
            {
                closedir( dirHandle1 );
                retVal = FILESYSTEM_STATUS_UNABLETOGETDIRENTRY;
            }
        }
        else
        {
            retVal = FILESYSTEM_STATUS_UNABLETOGETDIRENTRY;
        }
    }
    else
    {
        retVal = FILESYSTEM_STATUS_NOTADIRECTORY;  /* Items are not directories. */
    }

    return retVal;
}


#endif


BaseI32 FileSystem_compareFiles( const char *first,
                                 const char *second )
{
    BaseI32  retVal                                  = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 size1                                   = 0;
    BaseUI32 size2                                   = 0;
    FILE     *f1                                     = (FILE *)NULL;
    FILE     *f2                                     = (FILE *)NULL;
    char     buffer[2][FILESYSTEM_FILEBUFFER_LENGTH] = { "", "" };
    char     currentDir[FILESYSTEM_PATH_LENGTH]      = "";

    ANY_REQUIRE( first );
    ANY_REQUIRE( second );

    retVal = FileSystem_getCWD( currentDir, FILESYSTEM_PATH_LENGTH );

    if( FileSystem_isRegularFile( first ) && FileSystem_isRegularFile( second ) )
    {
        FILESYSTEM_RETURN_IF_FOPEN_FAILS( first, "rb", f1 );  /* Read binary. */

        if( NULL != ( f2 = fopen( second, "rb" ) ) )
        {
            size1 = 0;
            size2 = 0;

            do
            {
                size1 = fread( buffer[ 0 ], 1, FILESYSTEM_FILEBUFFER_LENGTH, f1 );
                size2 = fread( buffer[ 1 ], 1, FILESYSTEM_FILEBUFFER_LENGTH, f2 );
                if( 0 == ( size1 | size2 ) )
                {
                    break;
                }

                if( ( size1 != size2 ) ||
                    ( memcmp( buffer[ 0 ], buffer[ 1 ], size1 ) != 0 ) )
                {
                    retVal = FILESYSTEM_STATUS_FILESAREDIFFERENT;
                    break;
                }
            }
            while( size1 && size2 );
            fclose( f2 );
        }
        else
        {
            retVal = FILESYSTEM_STATUS_UNABLETOOPEN;
        }

        fclose( f1 );
    }
    else
    {
        retVal = FILESYSTEM_STATUS_NOTAREGULARFILE;  /* Items are not files. */
    }

    return retVal;
}


BaseI32 FileSystem_compare( const char *first,
                            const char *second )
{
    BaseI32 retVal = FILESYSTEM_STATUS_SUCCESS;

    ANY_REQUIRE( first );
    ANY_REQUIRE( second );

    if( FileSystem_isRegularFile( first ) &&
        FileSystem_isRegularFile( second ) )
    {
        retVal = FileSystem_compareFiles( first, second );
    }
    else if( FileSystem_isDirectory( first ) &&
             FileSystem_isDirectory( second ) )
    {
        retVal = FileSystem_compareDirectories( first, second );
    }
    else
    {
        retVal = FILESYSTEM_STATUS_INCOMPATIBLETYPES;  /* Comparison between incompatible types. */
    }

    return retVal;
}


BaseI32 FileSystem_copyFile( const char *fileToCopy,
                             const char *targetFile )
{
    BaseI32  retVal                               = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 read                                 = 0;
    BaseUI32 written                              = 0;
    char     buffer[FILESYSTEM_FILEBUFFER_LENGTH] = "";
    FILE     *src                                 = (FILE *)NULL;
    FILE     *dest                                = (FILE *)NULL;

    ANY_REQUIRE( targetFile );
    ANY_REQUIRE( fileToCopy );

    if( FileSystem_isRegularFile( fileToCopy ) )
    {
        FILESYSTEM_RETURN_IF_FOPEN_FAILS( fileToCopy, "rb", src );  /* Read binary. */

        // cannot use FILESYSTEM_RETURN_IF_FOPEN_FAILS() macro here as 'src'
        // would not be freed
        dest = fopen( targetFile, "wb" );

        if( dest == (FILE *)NULL )
        {
            fclose( src );
            return FILESYSTEM_STATUS_UNABLETOOPEN;
        }

        while( ( feof( src ) == 0 ) &&
               ( ferror( src ) == 0 ) &&
               ( ferror( dest ) == 0 ) )
        {
            read    = 0;
            written = 0;

            read = fread( buffer, 1, FILESYSTEM_FILEBUFFER_LENGTH, src );

            written = fwrite( buffer, 1, read, dest );

            if( written != read )
            {
                retVal = FILESYSTEM_STATUS_NOTWRITEABLE;
                break;
            }
        }

        if( ferror( src ) || ferror( dest ) )
        {
            retVal = FILESYSTEM_STATUS_FILEERROR;
        }

        fclose( src );
        fclose( dest );
    }
    else
    {
        retVal = -1;
    }

    return retVal;
}


#if defined( __windows__ )
BaseI32 FileSystem_deleteDirContent( const char *path )
{
    BaseI32         retVal                              = FILESYSTEM_STATUS_SUCCESS;
    BaseI32         impRetVal                           = FILESYSTEM_STATUS_SUCCESS;
    HANDLE          dirHandle                           = INVALID_HANDLE_VALUE;
    BaseBool        isDir                               = false;
    char            adaptedPath[FILESYSTEM_PATH_LENGTH] = "";
    char            fullPath[FILESYSTEM_PATH_LENGTH]    = "";
    WIN32_FIND_DATA dirEntry;

    /* Notice that 'pattern' can be NULL. */
    ANY_REQUIRE( path );

    if( FileSystem_isDirectory( path ) == false )
    {
        return FILESYSTEM_STATUS_NOTADIRECTORY;
    }

    FileSystem_addTrailingStar( adaptedPath, path, FILESYSTEM_PATH_LENGTH );
    dirHandle = FindFirstFile( adaptedPath, &dirEntry );

    if( INVALID_HANDLE_VALUE != dirHandle )
    {
        while( ( TRUE == FindNextFile( dirHandle, &dirEntry ) ) &&
               ( FILESYSTEM_STATUS_SUCCESS == retVal ) )
        {
            Any_strncpy( fullPath, path, FILESYSTEM_PATH_LENGTH - 1 );
            impRetVal = FileSystem_implode( fullPath, FILESYSTEM_PATH_LENGTH,
                                            dirEntry.cFileName );
            if( impRetVal == 0 )
            {
                if( Any_strcmp( ".", dirEntry.cFileName ) == 0 ||
                    Any_strcmp( "..", dirEntry.cFileName ) == 0 )
                {
                    continue;
                }

                isDir = FileSystem_isDirectory( fullPath );
                if( isDir == true )
                {
                    ANY_LOG( FILESYSTEM_LOGLEVEL_DEBUG,
                             "Appliying FileSystem_deleteDirContent recursively on >%s<",
                             ANY_LOG_INFO,
                             fullPath );
                    retVal = FileSystem_deleteDirContent( fullPath );

                    if( retVal == FILESYSTEM_STATUS_SUCCESS )
                    {
                        ANY_LOG( FILESYSTEM_LOGLEVEL_DEBUG,
                                 "Removing >%s<",
                                 ANY_LOG_INFO,
                                 fullPath );
                        retVal = FileSystem_remove( fullPath );
                    }
                }
                else
                {
                    ANY_LOG( FILESYSTEM_LOGLEVEL_DEBUG,
                             "Removing >%s<",
                             ANY_LOG_INFO,
                             fullPath );
                    retVal = FileSystem_remove( fullPath );
                }
            }
            else
            {
                retVal = impRetVal;
            }
        }
    }
    else
    {
        ANY_LOG( FILESYSTEM_LOGLEVEL_DEFAULT, "Invalid handle", ANY_LOG_ERROR );
    }

    FindClose( dirHandle );

    return retVal;
}
#else


BaseI32 FileSystem_deleteDirContent( const char *path )
{
    BaseI32       retVal                           = FILESYSTEM_STATUS_SUCCESS;
    BaseI32       impRetVal                        = FILESYSTEM_STATUS_SUCCESS;
    DIR           *dirHandle                       = (DIR *)NULL;
    BaseBool      isDir                            = false;
    struct dirent *dirEntry                        = (struct dirent *)NULL;
    char          fullPath[FILESYSTEM_PATH_LENGTH] = "";

    /* Notice that 'pattern' can be NULL. */
    ANY_REQUIRE( path );

    if( !FileSystem_isDirectory( path ) )
    {
        return FILESYSTEM_STATUS_NOTADIRECTORY;
    }

    dirHandle = opendir( path );
    ANY_REQUIRE( dirHandle );

    while( ( ( dirEntry = readdir( dirHandle ) ) != NULL ) &&
           ( retVal == FILESYSTEM_STATUS_SUCCESS ) )
    {
        Any_strncpy( fullPath, path, FILESYSTEM_PATH_LENGTH - 1 );
        impRetVal = FileSystem_implode( fullPath, FILESYSTEM_PATH_LENGTH,
                                        dirEntry->d_name );
        if( impRetVal == 0 )
        {
            if( Any_strcmp( ".", dirEntry->d_name ) == 0 ||
                Any_strcmp( "..", dirEntry->d_name ) == 0 )
            {
                continue;
            }

            isDir = FileSystem_isDirectory( fullPath );
            if( isDir )
            {
                ANY_LOG( FILESYSTEM_LOGLEVEL_DEBUG, "rm -r %s", ANY_LOG_INFO, fullPath );
                retVal = FileSystem_deleteDirContent( fullPath );

                if( retVal == FILESYSTEM_STATUS_SUCCESS )
                {
                    retVal = FileSystem_remove( fullPath );
                }
            }
            else
            {
                ANY_LOG( FILESYSTEM_LOGLEVEL_DEBUG, "rm %s", ANY_LOG_INFO, fullPath );
                retVal = FileSystem_remove( fullPath );
            }
        }
        else
        {
            retVal = impRetVal;
        }
    }

    closedir( dirHandle );

    return retVal;
}


#endif


BaseI32 FileSystem_concatenate( const char *targetFile,
                                const char *fileToAppend )
{
    BaseI32  retVal                               = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 read                                 = 0;
    BaseUI32 written                              = 0;
    char     buffer[FILESYSTEM_FILEBUFFER_LENGTH] = "";
    FILE     *src                                 = (FILE *)NULL;
    FILE     *dest                                = (FILE *)NULL;

    ANY_REQUIRE( targetFile );
    ANY_REQUIRE( fileToAppend );

    if( FileSystem_isDirectory( targetFile ) )
    {
        return FILESYSTEM_STATUS_TARGETFILEISADIRECTORY;
    }

    if( FileSystem_isDirectory( fileToAppend ) )
    {
        return FILESYSTEM_STATUS_FILETOAPPENDISADIRECTORY;
    }

    FILESYSTEM_RETURN_IF_FOPEN_FAILS( fileToAppend, "rb", src );  /* Read binary. */

    // cannot use FILESYSTEM_RETURN_IF_FOPEN_FAILS() macro here as 'src'
    // would not be freed
    dest = fopen( targetFile, "ab" );

    if( dest == (FILE *)NULL )
    {
        fclose( src );
        return FILESYSTEM_STATUS_UNABLETOOPEN;
    }

    while( ( feof( src ) == 0 ) &&
           ( ferror( src ) == 0 ) &&
           ( ferror( dest ) == 0 ) )
    {
        read    = 0;
        written = 0;

        read = fread( buffer, 1, FILESYSTEM_FILEBUFFER_LENGTH, src );

        written = fwrite( buffer, 1, read, dest );

        if( written != read )
        {
            retVal = FILESYSTEM_STATUS_WRITEERROR;
            break;
        }
    }

    if( ferror( src ) || ferror( dest ) )
    {
        retVal = FILESYSTEM_STATUS_FILEERROR;
    }

    fclose( src );
    fclose( dest );

    return retVal;
}


BaseI32 FileSystem_which( const char *path,
                          const char *fileName,
                          char *outPath,
                          const BaseUI32 outPathSize )
{
    BaseI32        retVal                                                        = FILESYSTEM_STATUS_GENERICERROR;
    BaseUI32       loops                                                         = 0;
    BaseI32        bRetVal                                                       = false;
    BaseUI32       i                                                             = 0;
    BaseUI32       len                                                           = 0;
    char           fullPath[FILESYSTEM_PATH_LENGTH]                              = "";
    char           *tmp                                                          = (char *)NULL;
    const BaseUI32 listSize                                                      = FILESYSTEM_DIRENTRIES_COUNT;
    const BaseUI32 listElementSize                                               = FILESYSTEM_PATH_LENGTH;
    char           pathList[FILESYSTEM_DIRENTRIES_COUNT][FILESYSTEM_PATH_LENGTH] = { { "" },
                                                                                     { "" } };

    ANY_REQUIRE( path );
    ANY_REQUIRE( fileName );
    ANY_REQUIRE( outPath );
    ANY_REQUIRE( outPathSize > 0 );

    retVal = FileSystem_explodePath( path,
                                     (char *)pathList,
                                     listSize,
                                     listElementSize );

    if( retVal >= 0 )
    {
        loops  = (BaseUI32)retVal;
        for( i = 0; i < loops; i++ )
        {
            tmp = (char *)&( pathList[ i ] );
            ANY_REQUIRE( tmp );

            len = Any_strlen( tmp );

            if( len < FILESYSTEM_PATH_LENGTH )
            {
                Any_strncpy( fullPath, tmp, FILESYSTEM_PATH_LENGTH - 1 );
                fullPath[ len ] = FILESYSTEM_NUL_CHARACTER;
                retVal = FileSystem_implode( fullPath, FILESYSTEM_PATH_LENGTH, fileName );
            }
            else
            {
                retVal = FILESYSTEM_STATUS_PATHSIZETOOBIG;
            }

            if( retVal == FILESYSTEM_STATUS_SUCCESS )
            {
                bRetVal = FileSystem_isExecutable( fullPath );

                if( bRetVal )
                {
                    len = Any_strlen( fullPath );
                    ANY_TRACE( 0, "%s", fullPath );

                    if( len < outPathSize )
                    {
                        Any_strncpy( outPath, fullPath, len );
                        outPath[ len ] = FILESYSTEM_NUL_CHARACTER;
                    }
                    else
                    {
                        retVal = FILESYSTEM_STATUS_OUTPATHTOOSMALL;
                    }

                    retVal = FILESYSTEM_STATUS_SUCCESS;
                    break;
                }
            }
        }
    }

    return retVal;
}


BaseI32 FileSystem_removeSuffixFromFileName( char *fileName,
                                             char *outSuffix,
                                             BaseUI32 suffixSize )
{
    BaseI32  retVal               = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 fileNameLen          = 0;
    char     *localSuffix         = (char *)NULL;
    char     *lastDelimiterSubstr = (char *)NULL;
    BaseUI32 localSuffixLen       = 0;
    BaseBool delimiterIsNull      = true;
    BaseI32  offset               = 0;

    ANY_REQUIRE( fileName );
    ANY_OPTIONAL( outSuffix );

    /* Find out the position of the last directory delimiter. */
    lastDelimiterSubstr = strrchr( fileName, FILESYSTEM_DIR_DELIMITER );

    /* Find out the position of the '.' preceeding the suffix. */
    localSuffix = strrchr( fileName, '.' );

    if( ( ( lastDelimiterSubstr != NULL ) && ( localSuffix != NULL ) ) ||
        ( ( lastDelimiterSubstr == NULL ) && ( localSuffix != NULL ) ) )
    {
        if( lastDelimiterSubstr != NULL )
        {
            delimiterIsNull = true;
        }
        else
        {
            offset = localSuffix - lastDelimiterSubstr;
        }

        /* Extract the offset only if the directory delimiter, when present,
         * is positioned before the '.' separating the suffix of the file.
         */
        if( ( delimiterIsNull ) || ( offset > 0 ) )
        {
            localSuffixLen = Any_strlen( localSuffix ) - 1;
            fileNameLen    = Any_strlen( fileName );

            if( outSuffix != NULL )
            {
                if( localSuffixLen < suffixSize )
                {
                    Any_strncpy( outSuffix, localSuffix + sizeof( char ), localSuffixLen );
                    outSuffix[ localSuffixLen ] = FILESYSTEM_NUL_CHARACTER;
                }
                else
                {
                    retVal = FILESYSTEM_STATUS_SUFFIXTOOSMALL;
                }
            }  /* End if( outSuffix ). */

            fileName[ fileNameLen - localSuffixLen - 1 ] = FILESYSTEM_NUL_CHARACTER;
        }
        else
        {
            if( outSuffix != NULL )
            {
                Any_memset( outSuffix, FILESYSTEM_NUL_CHARACTER, suffixSize );
            }
        }
    }

    return retVal;
}


BaseI32 FileSystem_explodePath( const char *path,
                                char *list,
                                const BaseUI32 listSize,
                                const BaseUI32 listElementSize )
{
    BaseI32 retVal = FILESYSTEM_STATUS_SUCCESS;

    ANY_REQUIRE( path );
    ANY_REQUIRE( list );
    ANY_REQUIRE( listSize > 0 );
    ANY_REQUIRE( listElementSize > 0 );

    retVal = UString_explode( path,
                              list,
                              listSize,
                              listElementSize,
                              FILESYSTEM_PATH_DELIMITER );

    return retVal;
}


BaseI32 FileSystem_decomposePath( const char *path,
                                  char *branch,
                                  BaseUI32 branchSize,
                                  char *leaf,
                                  BaseUI32 leafSize )
{
    BaseI32  retVal                           = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 pathLen                          = 0;
    BaseUI32 bLen                             = 0;
    BaseI32  i                                = 0;
    char     pathCopy[FILESYSTEM_PATH_LENGTH] = "";

    ANY_REQUIRE( path );
    ANY_OPTIONAL( branch );
    ANY_OPTIONAL( leaf );

    Any_memset( pathCopy, 0, FILESYSTEM_PATH_LENGTH );
    Any_strncpy( pathCopy, path, FILESYSTEM_PATH_LENGTH - 1 );

    pathLen = Any_strlen( pathCopy );

    /* Get rid of trailing delimiters except for the
    *  last one in case it's the only char. */
    while( ( pathCopy[ pathLen - 1 ] == FILESYSTEM_DIR_DELIMITER ) &&
           ( pathLen > 1 ) )
    {
        pathCopy[ pathLen - 1 ] = FILESYSTEM_NUL_CHARACTER;
        pathLen -= 1;
    }

    /* Find previous occurrence of delimiter. */
    i = pathLen - 1;
    ANY_REQUIRE( i < FILESYSTEM_PATH_LENGTH - 1 );

    while( i >= 0 )
    {
        if( pathCopy[ i ] == FILESYSTEM_DIR_DELIMITER )
        {
            break;
        }

        i -= 1;
    }

    /* Extract branch and leaf. */
    if( i < 0 )
    {
        if( branch != NULL )
        {
            Any_strncpy( branch, ".", 1 );
            branch[ 1 ] = FILESYSTEM_NUL_CHARACTER;
        }

        if( leaf != NULL )
        {
            if( leafSize > pathLen )
            {
                Any_strncpy( leaf, pathCopy, leafSize - 1 );
                leaf[ pathLen ] = FILESYSTEM_NUL_CHARACTER;
            }
            else
            {
                return FILESYSTEM_STATUS_INSUFFICIENTSIZE;
            }
        }
    }
    else
    {
        if( branch != NULL )
        {
            if( i > 0 )
            {
                Any_strncpy( branch, pathCopy, i );
                branch[ i ] = FILESYSTEM_NUL_CHARACTER;
            }
            else
            {
                bLen = Any_strlen( branch );

                if( bLen < branchSize - 1 )
                {
                    branch[ bLen ]     = FILESYSTEM_DIR_DELIMITER;
                    branch[ bLen + 1 ] = FILESYSTEM_NUL_CHARACTER;
                }
                else
                {
                    return FILESYSTEM_STATUS_INSUFFICIENTSIZE;
                }
            }
        }

        if( leaf != NULL )
        {
            Any_strncpy( leaf, &( pathCopy[ i + 1 ] ), pathLen - 1 );
        }
    }

    return retVal;
}


#if defined(__windows__)
BaseI32 FileSystem_getModificationTime( const char *path,
                                        struct tm *modTime )
{
    BaseI32     retVal = FILESYSTEM_STATUS_SUCCESS;
    struct stat stats;

    ANY_REQUIRE( path );
    ANY_REQUIRE( modTime );

    if( stat( path, &stats ) != 0 )
    {
        retVal = FILESYSTEM_STATUS_UNABLETOGETSTATS;
    }

    if( retVal == FILESYSTEM_STATUS_SUCCESS )
    {
#if defined(__msvc__)
        if( _localtime32_s( modTime, (const __time32_t *)&stats.st_mtime ) != 0 )
        {
            retVal = FILESYSTEM_STATUS_UNABLETOGETLOCALTIME;
        }
#else
        modTime = localtime( &( stats.st_mtime ) );

        if( stats.st_mtime != 0 )
        {
            retVal = FILESYSTEM_STATUS_UNABLETOGETLOCALTIME;
        }
#endif
    }

    return retVal;
}
#else


BaseI32 FileSystem_getModificationTime( const char *path,
                                        struct tm *modTime )
{
    BaseI32     retVal = FILESYSTEM_STATUS_SUCCESS;
    struct stat stats;

    ANY_REQUIRE( path );
    ANY_REQUIRE( modTime );

    if( stat( path, &stats ) != 0 )
    {
        retVal = FILESYSTEM_STATUS_UNABLETOGETSTATS;
    }
    else if( (void *)localtime_r( &stats.st_mtime, modTime ) == NULL )
    {
        retVal = FILESYSTEM_STATUS_UNABLETOGETLOCALTIME;
    }

    return retVal;
}


#endif

#if defined( __windows__ )
BaseBool FileSystem_isExecutable( const char *path )
{
  BaseBool retVal    = false;
  BaseI32 irRetVal   = 0;
  struct stat stats;

  ANY_REQUIRE( path );

  if( stat( path, &stats ) == 0 )
  {
    if( ( stats.st_mode & _S_IFREG ) != 0 )
    {
      irRetVal = 1;
    }

    if( irRetVal == 1 )
    {
      if( ( stats.st_mode & _S_IEXEC ) != 0 )
      {
        retVal = true;
      }
    }
  }

  return retVal;
}
#else


BaseBool FileSystem_isExecutable( const char *path )
{
    BaseBool    retVal   = false;
    BaseI32     irRetVal = 0;
    struct stat stats;
    ANY_REQUIRE( path );

    if( stat( path, &stats ) == 0 )
    {
        irRetVal = S_ISREG( stats.st_mode );

        if( irRetVal == 1 )
        {
            if( ( stats.st_mode & S_IXUSR ) != 0 )
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


#endif

#if defined( __windows__ )
BaseBool FileSystem_isWriteable( const char *path )
{
  BaseBool retVal    = false;
  BaseI32 irRetVal   = 0;
  struct stat stats;

  BaseI32 srv = stat( path, &stats );
  BaseBool bbnf = errno | ENOENT;
  BaseBool bbin = errno | EINVAL;

  ANY_REQUIRE( path );

  if( stat( path, &stats ) == 0 )
  {
    irRetVal = S_ISREG( stats.st_mode ) | S_ISDIR( stats.st_mode );

    if( irRetVal == 1 )
    {
      if( ( stats.st_mode & _S_IWRITE ) != 0 )
      {
        retVal = true;
      }
    }
  }

  return retVal;
}
#else


BaseBool FileSystem_isWriteable( const char *path )
{
    BaseBool    retVal   = false;
    BaseI32     irRetVal = 0;
    struct stat stats;

    ANY_REQUIRE( path );

    if( stat( path, &stats ) == 0 )
    {
        irRetVal = S_ISREG( stats.st_mode ) | S_ISDIR( stats.st_mode );

        if( irRetVal == 1 )
        {
            if( ( stats.st_mode & S_IWUSR ) != 0 )
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


#endif


BaseI32 FileSystem_readFile( const char *path,
                             MemI8 *block,
                             const BaseUI32 offset )
{
    BaseI32  retVal       = FILESYSTEM_STATUS_SUCCESS;
    BaseBool boolVal      = true;
    FILE     *file        = (FILE *)NULL;
    BaseI8   *blockBuffer = (BaseI8 *)NULL;
    BaseUI32 bufferLen    = 0;
    BaseI32  bytesRead    = 0;
    BaseUI32 fileSize     = 0;

    ANY_REQUIRE( path );
    ANY_REQUIRE( block );

    boolVal = FileSystem_isRegularFile( path ) ||
              FileSystem_isCharacterSpecialFile( path );

    if( boolVal )
    {

        blockBuffer = MemI8_getBuffer( block );

        if( blockBuffer == NULL )
        {
            return FILESYSTEM_STATUS_UNABLETORETRIEVEBUFFER;
        }

        bufferLen = MemI8_getLength( block );
        fileSize  = FileSystem_getSize( path );

        if( fileSize > bufferLen )
        {
            ANY_LOG( 0, "Size of the file is %d, too big for the buffer!", ANY_LOG_INFO, fileSize );
            return FILESYSTEM_STATUS_INSUFFICIENTSIZE;
        }

        FILESYSTEM_RETURN_IF_FOPEN_FAILS( path, "r", file );  /* Read. */

        if( fseek( file, offset, SEEK_SET ) != 0 )
        {
            fclose( file );
            return FILESYSTEM_STATUS_UNABLETOSEEK;
        }

        while( ( ferror( file ) == 0 ) &&
               ( feof( file ) == 0 ) )
        {
            bytesRead = fread( blockBuffer, sizeof( BaseI8 ), bufferLen - 1, file );

            if( bytesRead <= 0 || ferror( file ) != 0 )
            {
                break;
            }

            blockBuffer[ bytesRead ] = FILESYSTEM_NUL_CHARACTER;
        }  /* End while( !ferror... ). */

        fclose( file );
    }
    else
    {
        retVal = FILESYSTEM_STATUS_NOTAREGULARFILE;
    }

    return retVal;
}


BaseI32 FileSystem_move( const char *source,
                         const char *dest )
{
    BaseI32  retVal                            = FILESYSTEM_STATUS_GENERICERROR;
    char     destCopy[FILESYSTEM_PATH_LENGTH]  = "";
    char     destCopy2[FILESYSTEM_PATH_LENGTH] = "";
    BaseUI32 len                               = 0;
#if defined( __windows__ )
    /* All these variables are needed because there is no
     * dirname() on Win32.
     */
    char dirPath[ FILESYSTEM_PATH_LENGTH ]   = "";
    char drive[ 8 ]                          = "";
    char fileName[ FILESYSTEM_FILENAME_LENGTH ] = "";
    char extension[ 8 ]                      = "";
#endif

    ANY_REQUIRE( source );
    ANY_REQUIRE( dest );

#if defined( __windows__ )
    /* Again, this block is needed because there is
     * no dirname() on Win32.
     */
#if defined(__msvc__)
    _splitpath_s( source,
                  drive, sizeof( drive ),
                  dirPath, FILESYSTEM_PATH_LENGTH,
                  fileName, FILESYSTEM_FILENAME_LENGTH,
                  extension, sizeof( extension ) );
#else
    _splitpath( source,
                NULL,
                dirPath,
                NULL,
                NULL );
#endif

    /* If the dirPath is empty, it means there is only the name of the
     * file and dirPath should hold the current path ".".
     */
    if( Any_strlen( dirPath ) == 0 )
    {
      Any_strncpy( dirPath, FILESYSTEM_CURRENT_FOLDER, FILESYSTEM_PATH_LENGTH - 1 );
    }
#endif

    len = Any_strlen( dest );
    if( len < sizeof( FILESYSTEM_PATH_LENGTH ) )
    {
        Any_strncpy( destCopy, dest, FILESYSTEM_PATH_LENGTH - 1 );
        Any_strncpy( destCopy2, dest, FILESYSTEM_PATH_LENGTH - 1 );
    }
    else
    {
        return FILESYSTEM_STATUS_PATHSIZETOOBIG;
    }

    if( !FileSystem_isRegularFile( source ) )
    {
        retVal = FILESYSTEM_STATUS_NOTAREGULARFILE;
    }
#if defined( __windows__ )
        /* No dirname() on Win32! Data about the dirPath was
         * collected above.
         */
        else if( FileSystem_isWriteable( dirPath ) == true )
        {
          retVal = rename( source, dest );

          if( retVal != 0 )
          {
            retVal = FILESYSTEM_STATUS_UNABLETOMOVE;
          }
          else
          {
            retVal = FILESYSTEM_STATUS_SUCCESS;
          }
        }
#else
    else if( FileSystem_isWriteable( dirname( destCopy ) ) )
    {
        retVal = rename( source, dest );

        if( retVal != 0 )
        {
            retVal = FILESYSTEM_STATUS_UNABLETOMOVE;
        }
        else
        {
            retVal = FILESYSTEM_STATUS_SUCCESS;
        }
    }
#endif
    else
    {
        retVal = FILESYSTEM_STATUS_NOTWRITEABLE;
    }

    return retVal;
}


BaseI32 FileSystem_remove( const char *path )
{
    BaseI32  retVal = FILESYSTEM_STATUS_SUCCESS;
    BaseBool isDir  = false;

    ANY_REQUIRE( path );

    isDir = FileSystem_isDirectory( path );

    if( isDir )
    {
        retVal = FileSystem_deleteDirContent( path );
    }

    if( retVal == 0 )
    {
        retVal = remove( path );
    }

    if( retVal != 0 )
    {
        retVal = FILESYSTEM_STATUS_GENERICERROR;
        ANY_LOG( FILESYSTEM_LOGLEVEL_DEFAULT,
                 "%s: remove failed (%s)",
                 ANY_LOG_ERROR, path,
                 strerror( errno ) );
    }

    return retVal;
}


BaseI32 FileSystem_implode( char *path,
                            const BaseI32 size,
                            const char *toAdd )
{
    BaseI32  retVal   = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 pathLen  = 0;
    BaseUI32 toAddLen = 0;

    ANY_REQUIRE( path );
    ANY_REQUIRE( toAdd );

    pathLen  = Any_strlen( path );
    toAddLen = BASE_MATH_MIN( (BaseUI32)Any_strlen( toAdd ),
                              ( BaseUI32 )( size - 1 ) );

    if( ( pathLen + toAddLen ) < ( (BaseUI32)size - 1 ) )
    {
        /* Add delimiter to path if original path is not empty
         * and it doesn't terminate with the delimiter. */
        if( pathLen > 0 && pathLen < FILESYSTEM_PATH_LENGTH - 1 )
        {
            if( path[ pathLen - 1 ] != FILESYSTEM_DIR_DELIMITER )
            {
                path[ pathLen ]     = FILESYSTEM_DIR_DELIMITER;
                path[ pathLen + 1 ] = FILESYSTEM_NUL_CHARACTER;
            }
        }

        Any_strncat( path, toAdd, toAddLen );
    }
    else
    {
        retVal = FILESYSTEM_STATUS_INSUFFICIENTSIZE;
    }

    return retVal;
}


FileSystemFileType FileSystem_getFileType( const char *path )
{
    FileSystemFileType fileType = FILESYSTEM_FILETYPE_UNKNOWN;

    if( FileSystem_isRegularFile( path ) )
    {
        fileType = FILESYSTEM_FILETYPE_REGULARFILE;
    }
    else if( FileSystem_isDirectory( path ) )
    {
        fileType = FILESYSTEM_FILETYPE_DIRECTORY;
    }
    else if( FileSystem_isSymLink( path ) )
    {
        fileType = FILESYSTEM_FILETYPE_SYMLINK;
    }
    else if( FileSystem_isNamedPipe( path ) )
    {
        fileType = FILESYSTEM_FILETYPE_NAMEDPIPE;
    }
    else if( FileSystem_isBlockSpecialFile( path ) )
    {
        fileType = FILESYSTEM_FILETYPE_BLOCKSPECIALFILE;
    }
    else if( FileSystem_isCharacterSpecialFile( path ) )
    {
        fileType = FILESYSTEM_FILETYPE_CHARSPECIALFILE;
    }
    else if( FileSystem_isSocket( path ) )
    {
        fileType = FILESYSTEM_FILETYPE_SOCKET;
    }

    return fileType;
}


BaseBool FileSystem_isBlockSpecialFile( const char *path )
{
    BaseBool    retVal     = false;
    BaseI32     modeRetVal = 0;
    struct stat stats;

    ANY_REQUIRE( path );

    if( stat( path, &stats ) == 0 )
    {
        modeRetVal = S_ISBLK( stats.st_mode );

        if( modeRetVal == 1 )
        {
            retVal = true;
        }
    }

    return retVal;
}


BaseBool FileSystem_isCharacterSpecialFile( const char *path )
{
    BaseBool    retVal     = false;
    BaseI32     modeRetVal = 0;
    struct stat stats;

    ANY_REQUIRE( path );

    if( stat( path, &stats ) == 0 )
    {
        modeRetVal = S_ISCHR( stats.st_mode );

        if( modeRetVal == 1 )
        {
            retVal = true;
        }
    }

    return retVal;
}


BaseBool FileSystem_isRegularFile( const char *path )
{
    BaseBool    retVal     = false;
    BaseI32     modeRetVal = 0;
    struct stat stats;

    ANY_REQUIRE( path );

    /* Using lstat() instead of stat() to avoid considering pointed objects
     * in case path is a symbolic link.
     *
     * However, Windows does not know about it.
     */
#if defined(__windows__)
    if( stat( path, &stats ) == 0 )
#else
    if( lstat( path, &stats ) == 0 )
#endif
    {
        modeRetVal = S_ISREG( stats.st_mode );

        if( modeRetVal == 1 )
        {
            retVal = true;
        }
    }

    return retVal;
}


BaseBool FileSystem_isDirectory( const char *path )
{
    BaseBool    retVal     = false;
    BaseI32     modeRetVal = 0;
    struct stat stats;

    ANY_REQUIRE( path );

    if( stat( path, &stats ) == 0 )
    {
        /* The following line has been commented out and replaced by a new one
         * without the check on S_IXUSR because it could be needed to know if a
         * path is a directory even if it doesn't have execution permission on
         * it: e.g. when looping on the elements of a directory there could be a
         * subdirectory that hasn't got execution permission on it but it could
         * be anyway useful to know it is a directory because the application might
         * be interested in opening files only.
         *
         *  modeRetVal = S_ISDIR( stats.st_mode ) && ( stats.st_mode & S_IXUSR ) != 0;
         */
        modeRetVal = S_ISDIR( stats.st_mode );

        if( modeRetVal == 1 )
        {
            retVal = true;
        }
    }

    return retVal;
}


BaseBool FileSystem_isNamedPipe( const char *path )
{
    BaseBool    retVal     = false;
    BaseI32     modeRetVal = 0;
    struct stat stats;

    ANY_REQUIRE( path );

    if( stat( path, &stats ) == 0 )
    {
        modeRetVal = S_ISFIFO( stats.st_mode );

        if( modeRetVal == 1 )
        {
            retVal = true;
        }
    }

    return retVal;
}


BaseBool FileSystem_isSocket( const char *path )
{
    BaseBool retVal     = false;
    BaseI32  modeRetVal = 0;

#if !defined(__windows__)
    struct stat stats;
#endif

    ANY_REQUIRE( path );

#if defined(__windows__)
    return false;
#else
    if( stat( path, &stats ) == 0 )                                            \

    {
        \
    modeRetVal = S_ISSOCK( stats.st_mode );                                  \
                                                                             \
    if( modeRetVal == 1 )                                                    \

        {
            \
      retVal = true;                                                         \

        } \

    }
#endif

    return retVal;
}


BaseBool FileSystem_isSymLink( const char *path )
{
    BaseBool retVal     = false;
    BaseI32  modeRetVal = 0;

#if !defined( __windows__ )
    struct stat stats;
#endif

    ANY_REQUIRE( path );

    /* Using lstat() instead of stat() to avoid considering pointed objects
     * in case path is a symbolic link.
     *
     * However, Windows does not know about it.
     */

#if defined( __windows__ )
    ANY_LOG( FILESYSTEM_LOGLEVEL_CRITICAL,
             "FileSystem_isSymLink() not implemented on Win32",
             ANY_LOG_WARNING );
#else
    if( lstat( path, &stats ) == 0 )
    {
        modeRetVal = S_ISLNK( stats.st_mode );

        if( modeRetVal == 1 )
        {
            retVal = true;
        }
    }
    else
    {
        retVal = false;
    }
#endif

    return retVal;
}


#if defined( __windows__ )
BaseI32 FileSystem_readDirectory( const char *path,
                                  const FileSystemReadDirMode mode,
                                  const char *pattern,
                                  char *list,
                                  const BaseUI32 listSize,
                                  const BaseUI32 listElementSize )
{
    BaseI32         retVal                               = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32        listIndex                            = 0;
    BaseUI32        nameLen                              = 0;
    BaseUI32        nullPos                              = 0;
    BaseUI32        pathLen                              = 0;
    BaseI32         impRetVal                            = 0;
    HANDLE          dirHandle                            = INVALID_HANDLE_VALUE;
    BaseBool        skip                                 = false;
    BaseBool        isFile                               = false;
    BaseBool        isDir                                = false;
    char            adaptedPath[FILESYSTEM_PATH_LENGTH]  = "";
    char            fileName[FILESYSTEM_FILENAME_LENGTH] = "";
    char            fullPath[FILESYSTEM_PATH_LENGTH]     = "";
    char            *tmp                                 = (char *)NULL;
    DWORD           lastError                            = 0;
    WIN32_FIND_DATA dirEntry;

    /* Notice that 'pattern' can be NULL. */
    ANY_REQUIRE( path );
    ANY_REQUIRE( list );
    ANY_REQUIRE( listSize > 0 );
    ANY_REQUIRE( listElementSize > 0 );

    if( FileSystem_isDirectory( path ) == false )
    {
        return FILESYSTEM_STATUS_NOTADIRECTORY;
    }

    /* Add a trailing "\*" to explore the contents of the directory. */
    FileSystem_addTrailingStar( adaptedPath, path, sizeof( adaptedPath ) );

    dirHandle = FindFirstFile( adaptedPath, &dirEntry );

    if( INVALID_HANDLE_VALUE != dirHandle )
    {
        while( ( TRUE == FindNextFile( dirHandle, &dirEntry ) ) &&
               ( listIndex < listSize ) )
        {
            Any_memset( fileName, FILESYSTEM_NUL_CHARACTER, FILESYSTEM_FILENAME_LENGTH );

            skip    = false;
            nameLen = Any_strlen( dirEntry.cFileName );
            nullPos = ( nameLen < FILESYSTEM_FILENAME_LENGTH ) ?
                      nameLen : FILESYSTEM_FILENAME_LENGTH;
            Any_strncpy( fileName, dirEntry.cFileName, nullPos );
            fileName[ nullPos ] = FILESYSTEM_NUL_CHARACTER;

            if( ( Any_strcmp( ".", fileName ) == 0 ) ||
                ( Any_strcmp( "..", fileName ) == 0 ) )
            {
                skip = true;
            }

#if !defined(__windows__)
            if( pattern != NULL )
            {
                if( fnmatch( pattern, fileName, 0 ) != 0 )
                {
                    skip = true;
                }
            }
#endif

            if( ( mode != FILESYSTEM_READDIR_ALL ) && ( skip == false ) )
            {
                pathLen = Any_strlen( path );
                if( pathLen < FILESYSTEM_PATH_LENGTH )
                {
                    Any_strncpy( fullPath, path, FILESYSTEM_PATH_LENGTH - 1 );
                }
                else
                {
                    return FILESYSTEM_STATUS_PATHSIZETOOBIG;
                }

                impRetVal = FileSystem_implode( fullPath,
                                                FILESYSTEM_PATH_LENGTH,
                                                fileName );
                if( impRetVal != 0 )
                {
                    ANY_LOG( FILESYSTEM_LOGLEVEL_DEFAULT,
                             "Unable to extend path >%s< with >%s<",
                             ANY_LOG_WARNING,
                             fullPath,
                             fileName );
                    continue;
                }

                isFile = FileSystem_isRegularFile( fullPath );
                if( ( mode == FILESYSTEM_READDIR_FILES ) && ( isFile == false ) )
                {
                    skip = true;
                }

                isDir = FileSystem_isDirectory( fullPath );
                if( ( mode == FILESYSTEM_READDIR_DIRS ) && ( isDir == false ) )
                {
                    skip = true;
                }

                Any_memset( fullPath, FILESYSTEM_NUL_CHARACTER, FILESYSTEM_PATH_LENGTH );
            }

            if( skip == false )
            {
                nameLen = Any_strlen( fileName );
                nullPos = ( nameLen < listElementSize ) ? nameLen : listElementSize - 1;
                tmp     = list + ( listIndex * listElementSize * sizeof( char ) );
                Any_memset( tmp, '\0', listElementSize );
                Any_strncpy( tmp, fileName, listElementSize - 1 );
                listIndex++;
            }
        }

        if( ERROR_NO_MORE_FILES == ( lastError = GetLastError() ) )
        {
            ANY_LOG( FILESYSTEM_LOGLEVEL_VERBOSE,
                     "There are no more files (last error >%d<)!",
                     ANY_LOG_INFO,
                     lastError );
        }
    }
    else
    {
        ANY_LOG( FILESYSTEM_LOGLEVEL_DEFAULT, "Unable to get handle", ANY_LOG_ERROR );
    }

    /* If no error occurred, return the number of elements in the list. */
    if( FILESYSTEM_STATUS_SUCCESS == retVal )
    {
        retVal = listIndex;
    }

    FindClose( dirHandle );

    return retVal;
}
#else


BaseI32 FileSystem_readDirectory( const char *path,
                                  const FileSystemReadDirMode mode,
                                  const char *pattern,
                                  char *list,
                                  const BaseUI32 listSize,
                                  const BaseUI32 listElementSize )
{
    BaseI32       retVal                               = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32      listIndex                            = 0;
    BaseUI32      nameLen                              = 0;
    BaseUI32      nullPos                              = 0;
    BaseUI32      pathLen                              = 0;
    BaseI32       impRetVal                            = 0;
    DIR           *dirHandle                           = NULL;
    struct dirent *dirEntry                            = NULL;
    BaseBool      skip                                 = false;
    BaseBool      isFile                               = false;
    BaseBool      isDir                                = false;
    char          fileName[FILESYSTEM_FILENAME_LENGTH] = "";
    char          fullPath[FILESYSTEM_PATH_LENGTH]     = "";
    char          *tmp                                 = (char *)NULL;

    /* Notice that 'pattern' can be NULL. */
    ANY_REQUIRE( path );
    ANY_REQUIRE( list );
    ANY_REQUIRE( listSize > 0 );
    ANY_REQUIRE( listElementSize > 0 );

    if( !FileSystem_isDirectory( path ) )
    {
        return FILESYSTEM_STATUS_NOTADIRECTORY;
    }

    dirHandle = opendir( path );

    if( dirHandle != NULL )
    {
        while( ( ( dirEntry = readdir( dirHandle ) ) != NULL ) &&
               ( listIndex < listSize ) )
        {
            Any_memset( fileName, FILESYSTEM_NUL_CHARACTER, FILESYSTEM_FILENAME_LENGTH );
            skip    = false;
            nameLen = Any_strlen( dirEntry->d_name );
            nullPos = ( nameLen < FILESYSTEM_FILENAME_LENGTH ) ?
                      nameLen : FILESYSTEM_FILENAME_LENGTH;

            ANY_REQUIRE( nullPos < FILESYSTEM_FILENAME_LENGTH );

            Any_strncpy( fileName, dirEntry->d_name, nullPos );
            fileName[ nullPos ] = FILESYSTEM_NUL_CHARACTER;

            if( ( Any_strcmp( ".", fileName ) == 0 ) ||
                ( Any_strcmp( "..", fileName ) == 0 ) )
            {
                skip = true;
            }

            if( pattern != NULL )
            {
                if( fnmatch( pattern, fileName, 0 ) != 0 )
                {
                    skip = true;
                }
            }

            if( ( mode != FILESYSTEM_READDIR_ALL ) && ( !skip ) )
            {
                pathLen = Any_strlen( path );

                if( pathLen < FILESYSTEM_PATH_LENGTH )
                {
                    Any_strncpy( fullPath, path, FILESYSTEM_PATH_LENGTH - 1 );
                }
                else
                {
                    return FILESYSTEM_STATUS_PATHSIZETOOBIG;
                }

                impRetVal = FileSystem_implode( fullPath,
                                                FILESYSTEM_PATH_LENGTH,
                                                fileName );
                if( impRetVal != 0 )
                {
                    ANY_LOG( FILESYSTEM_LOGLEVEL_DEFAULT,
                             "Unable to extend path >%s< with >%s<",
                             ANY_LOG_WARNING,
                             fullPath,
                             fileName );
                    continue;
                }

                isFile = FileSystem_isRegularFile( fullPath );
                if( ( mode == FILESYSTEM_READDIR_FILES ) && ( !isFile ) )
                {
                    skip = true;
                }

                isDir = FileSystem_isDirectory( fullPath );
                if( ( mode == FILESYSTEM_READDIR_DIRS ) && ( !isDir ) )
                {
                    skip = true;
                }

                Any_memset( fullPath, FILESYSTEM_NUL_CHARACTER, FILESYSTEM_PATH_LENGTH );
            }

            if( !skip )
            {
                nameLen = Any_strlen( fileName );
                nullPos = ( nameLen < listElementSize ) ? nameLen : listElementSize - 1;
                tmp     = list + ( listIndex * listElementSize * sizeof( char ) );
                Any_memset( tmp, '\0', listElementSize );
                Any_strncpy( tmp, fileName, listElementSize - 1 );
                listIndex++;
            }
        }

        /* If no error occurred, return the number of elements in the list. */
        if( retVal == FILESYSTEM_STATUS_SUCCESS )
        {
            retVal = listIndex;
        }

        if( dirHandle != NULL )
        {
            closedir( dirHandle );
        }
    }
    else
    {
        ANY_LOG( FILESYSTEM_LOGLEVEL_CRITICAL, "Unable to open directory >%s<",
                 ANY_LOG_ERROR, path );
    }

    return retVal;
}


#endif


BaseI32 FileSystem_getCWD( char *path,
                           BaseUI32 size )
{
    BaseI32 retVal = FILESYSTEM_STATUS_SUCCESS;
    char    *tmp   = (char *)NULL;

    ANY_REQUIRE( path );

#if defined(__windows__)
    tmp = _getcwd( path, size );
#else
    tmp = getcwd( path, size );
#endif

    if( ( tmp == NULL ) && ( errno = ERANGE ) )
    {
        retVal = FILESYSTEM_STATUS_INSUFFICIENTSIZE;
    }
    else if( tmp == NULL )
    {
        retVal = FILESYSTEM_STATUS_UNABLETOGETCWD;
    }

    return retVal;
}


BaseI32 FileSystem_changeDirectory( const char *path )
{
    BaseI32 retVal = FILESYSTEM_STATUS_SUCCESS;

    ANY_REQUIRE( path );

    if( chdir( path ) != 0 )
    {
        retVal = FILESYSTEM_STATUS_GENERICERROR;
    }

    return retVal;
}


/*
 * FileSystem_makeDirectory
 *
 * S_IRWXU : constant := 448;
 * S_IRUSR : constant := 256;
 * S_IWUSR : constant := 128;
 * S_IXUSR : constant := 64;
 *
 * S_IRWXG : constant := 56;
 * S_IRGRP : constant := 32;
 * S_IWGRP : constant := 16;
 * S_IXGRP : constant := 8;
 *
 * S_IRWXO : constant := 7;
 * S_IROTH : constant := 4;
 * S_IWOTH : constant := 2;
 * S_IXOTH : constant := 1;
 *
 */
BaseI32 FileSystem_makeDirectory( const char *path )
{
    BaseI32 retVal = FILESYSTEM_STATUS_SUCCESS;

    ANY_REQUIRE( path );

#if defined(__windows__)
    retVal = mkdir( path );
#else
    /* under *nix, give user read/write/execute permissions to the new directory. */
    retVal = mkdir( path, S_IRWXU | S_IRWXG | S_IRWXO );
#endif

    if( retVal != 0 )
    {
        retVal = FILESYSTEM_STATUS_GENERICERROR;
    }

    return retVal;
}


BaseBool FileSystem_makeDirectories( const char *path )
{
    char   tmp[FILESYSTEM_PATH_LENGTH];
    char   *p = NULL;
    size_t len;

    ANY_REQUIRE( path );

    Any_snprintf( tmp, FILESYSTEM_PATH_LENGTH - 1, "%s", path );
    len = Any_strlen( tmp );
    ANY_REQUIRE_MSG( len > 0, "an empty path-argument is not allowed" );

    /* create directories until final path leaf */

    ANY_LOG( FILESYSTEM_LOGLEVEL_DEBUG, "mkdir -p %s", ANY_LOG_INFO, path );

    if( tmp[ len - 1 ] == '/' )
    {
        tmp[ len - 1 ] = 0;
    }

    for( p = tmp + 1; *p; p++ )
    {
        if( *p == '/' )
        {
            *p = 0;

            if( !FileSystem_isDirectory( tmp ) )
            {
                FileSystem_makeDirectory( tmp );
            }

            *p = '/';
        }
    }

    /* create final path */
    FileSystem_makeDirectory( tmp );

    return FileSystem_isDirectory( path );
}


BaseI32 FileSystem_strerror( FileSystemReturnStatus errorCode,
                             char *prefix,
                             char *stringBuffer,
                             BaseUI32 bufferSize )
{
    BaseI32  retVal                                   = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 prefixLen                                = 0;
    BaseUI32 internalLen                              = 0;
    char     internalBuffer[FILESYSTEM_BUFFER_LENGTH] = "";

    ANY_OPTIONAL( prefix );
    ANY_REQUIRE( stringBuffer );
    ANY_REQUIRE( bufferSize > 0 );

    if( prefix != NULL )
    {
        prefixLen = Any_strlen( prefix );

        if( prefixLen < FILESYSTEM_BUFFER_LENGTH )
        {
            Any_strncat( internalBuffer, prefix, FILESYSTEM_BUFFER_LENGTH - 1 );
            internalBuffer[ prefixLen ] = '\0';
        }
    }

    switch( errorCode )
    {
        case FILESYSTEM_STATUS_SUCCESS:
        {
            Any_strncpy( internalBuffer,
                         "The call was successful",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_GENERICERROR:
        {
            Any_strncpy( internalBuffer,
                         "Generic error",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_NOTWRITEABLE:
        {
            Any_strncpy( internalBuffer,
                         "File/directory is not writeable",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_NOTAREGULARFILE:
        {
            Any_strncpy( internalBuffer,
                         "File is not a regular file",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETOOPEN:
        {
            Any_strncpy( internalBuffer,
                         "Unable to open file",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETOGETSTATS:
        {
            Any_strncpy( internalBuffer,
                         "Unable to retrieve file stats",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_NOTEXECUTABLE:
        {
            Any_strncpy( internalBuffer,
                         "File is not executable",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETOGETLOCALTIME:
        {
            Any_strncpy( internalBuffer,
                         "Unable to retireve local time",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_INSUFFICIENTSIZE:
        {
            Any_strncpy( internalBuffer,
                         "Buffer size is not big enough",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETOGETCWD:
        {
            Any_strncpy( internalBuffer,
                         "Unable to retrieve current working directory",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_NOTADIRECTORY:
        {
            Any_strncpy( internalBuffer,
                         "Parameter is not a directory",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_SUFFIXTOOSMALL:
        {
            Any_strncpy( internalBuffer,
                         "Suffix size is too small",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_PATHSIZETOOBIG:
        {
            Any_strncpy( internalBuffer,
                         "Path size is too big",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_OUTPATHTOOSMALL:
        {
            Any_strncpy( internalBuffer,
                         "Output path is too small",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_FILEERROR:
        {
            Any_strncpy( internalBuffer,
                         "Error while reading file",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_INCOMPATIBLETYPES:
        {
            Any_strncpy( internalBuffer,
                         "Incompatible files types",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_DIRSAREDIFFERENT:
        {
            Any_strncpy( internalBuffer,
                         "Directories are different",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETOGETDIRENTRY:
        {
            Any_strncpy( internalBuffer,
                         "Unable to get directory entry",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_FILESAREDIFFERENT:
        {
            Any_strncpy( internalBuffer,
                         "Files are different",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETOMOVE:
        {
            Any_strncpy( internalBuffer,
                         "Unable to move file",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETORETRIEVEBUFFER:
        {
            Any_strncpy( internalBuffer,
                         "Unable to retrieve buffer",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_UNABLETOSEEK:
        {
            Any_strncpy( internalBuffer,
                         "Unable to seek to required offset",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_WRITEERROR:
        {
            Any_strncpy( internalBuffer,
                         "Error while writing file",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_TARGETFILEISADIRECTORY:
        {
            Any_strncpy( internalBuffer,
                         "Target file is a directory",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        case FILESYSTEM_STATUS_FILETOAPPENDISADIRECTORY:
        {
            Any_strncpy( internalBuffer,
                         "File to append is a directory",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
            break;
        }
        default:
        {
            Any_strncpy( internalBuffer,
                         "Unspecified error",
                         FILESYSTEM_BUFFER_LENGTH - 1 );
        }
    }

    internalLen = Any_strlen( internalBuffer );
    if( bufferSize > internalLen + prefixLen )
    {
        Any_strncpy( stringBuffer, internalBuffer, bufferSize - 1 );
        stringBuffer[ internalLen + prefixLen ] = FILESYSTEM_NUL_CHARACTER;
    }
    else
    {
        retVal = FILESYSTEM_STATUS_INSUFFICIENTSIZE;
    }

    return retVal;
}


BaseI32 FileSystem_implodeIfRelative( char *path,
                                      BaseUI32 pathSize )
{
    BaseI32  retVal                             = FILESYSTEM_STATUS_SUCCESS;
    BaseI32  cwdRetVal                          = FILESYSTEM_STATUS_SUCCESS;
    BaseI32  impRetVal                          = FILESYSTEM_STATUS_SUCCESS;
    BaseUI32 fullLen                            = 0;
    char     currentDir[FILESYSTEM_PATH_LENGTH] = "";
    char     fullPath[FILESYSTEM_PATH_LENGTH]   = "";

    ANY_REQUIRE( path );
    if( path[ 0 ] != FILESYSTEM_DIR_DELIMITER )
    {
        cwdRetVal = FileSystem_getCWD( currentDir,
                                       sizeof( currentDir ) );

        if( cwdRetVal == 0 )
        {
            Any_strncpy( fullPath, currentDir, FILESYSTEM_PATH_LENGTH - 1 );
            fullPath[ Any_strlen( currentDir ) ] = FILESYSTEM_NUL_CHARACTER;
            impRetVal = FileSystem_implode( fullPath, pathSize, path );

            if( impRetVal == 0 )
            {
                fullLen = Any_strlen( fullPath );

                if( fullLen < pathSize )
                {
                    Any_strncpy( path, fullPath, pathSize - 1 );
                    path[ fullLen ] = FILESYSTEM_NUL_CHARACTER;
                }
                else
                {
                    retVal = FILESYSTEM_STATUS_INSUFFICIENTSIZE;
                }
            }
            else
            {
                retVal = impRetVal;
            }
        }
        else
        {
            retVal = cwdRetVal;
        }
    }

    return retVal;
}


#if defined( __windows__ )
BaseI32 FileSystem_addTrailingStar( char *adaptedPath,
                                    const char *path,
                                    const BaseUI32 adaptedPathSize )
{
    BaseI32  retVal         = FILESYSTEM_STATUS_SUCCESS;
    char     delimiter[3]   = "";
    BaseUI32 adaptedPathLen = 0;

    ANY_REQUIRE( path );
    ANY_REQUIRE( adaptedPath );

    Any_strncpy( adaptedPath, path, adaptedPathSize );
    adaptedPathLen = Any_strlen( adaptedPath );

    /* Add a trailing "\*" to explore the contents of the directory. */
    delimiter[ 0 ] = FILESYSTEM_DIR_DELIMITER;
    delimiter[ 1 ] = '*';
    delimiter[ 2 ] = '\0';

    if( ( adaptedPathSize - adaptedPathLen ) >= sizeof( delimiter ) )
    {
        Any_strncat( adaptedPath, delimiter, adaptedPathSize - adaptedPathLen );
    }
    else
    {
        retVal = FILESYSTEM_STATUS_INSUFFICIENTSIZE;
    }

    return retVal;
}
#endif


/*EOF */
