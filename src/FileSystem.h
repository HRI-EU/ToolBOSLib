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


#ifndef FILESYSTEM_H
#define FILESYSTEM_H


/*---------------------------------------------*/
/* Main documentation                          */
/*---------------------------------------------*/

/*!
 * \page FileSystem_About Filesystem access
 *
 * The FileSystem (FileSystem.h) library provides utility functions for
 * performing platform-independent operations on files and directories:
 *
 * \li get meta-information (type, modification times, size,...)
 * \li compare two files or directories
 * \li create / move / delete
 * \li concatenate files
 * \li tokenize paths names
 * \li ...
 *
 * <h3>Example:</h3>
 * The following sample usage shows how to read a directory into a list:
 *
 * \code
 * BaseI32 retVal                            = FILESYSTEM_STATUS_SUCCESS;
 * char *dirListing                          = ( char* )NULL;
 * char currentDir[ FILESYSTEM_PATH_LENGTH ] = "";
 *
 * // Get current directory.
 * retVal = FileSystem_getCWD( currentDir, FILESYSTEM_PATH_LENGTH );
 *
 * if( retVal == FILESYSTEM_STATUS_SUCCESS )
 * {
 *   // Allocate memory for the list.
 *   dirListing = ANY_MALLOC( listEntriesCount, sizeof( char* ) );
 *
 *   // Load the list with entries from the directory.
 *   retVal = FileSystem_readDirectory( currentDir, FILESYSTEM_READDIR_ALL,
                                        NULL, dirListing, 40,
                                        FILESYSTEM_FILENAME_LENGTH );
 *
 *   ANY_LOG( FILESYSTEM_LOGLEVEL_DEFAULT, "_readDirectory() returned >%d<", ANY_LOG_INFO, retVal );
 *
 *   ANY_FREE( dirListing );
 * }
 * else
 * {
 *   ANY_LOG( FILESYSTEM_LOGLEVEL_DEFAULT, "Unable to retrieve current directory.", ANY_LOG_ERROR );
 * }
 * \endcode
 */


/*---------------------------------------------*/
/* Include files                               */
/*---------------------------------------------*/

#include <time.h>
#include <sys/stat.h>

#include <Base.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------*/
/* Defines and constants                       */
/*---------------------------------------------*/


#if defined( __msvc__ ) || defined( __windows__ )

/*! \brief Delimiter between directory names. */
#define FILESYSTEM_DIR_DELIMITER '\\'
//#define FILESYSTEM_DIR_DELIMITER '/'

/*! \brief Delimiter between paths. */
#define FILESYSTEM_PATH_DELIMITER ';'

/*! \brief End of the line delimiter. */
#define FILESYSTEM_LINE_DELIMITER "\r\n"

#else

/*! \brief Delimiter between directory names. */
#define FILESYSTEM_DIR_DELIMITER '/'

/*! \brief Delimiter between paths. */
#define FILESYSTEM_PATH_DELIMITER ':'

/*! \brief End of the line delimiter. */
#define FILESYSTEM_LINE_DELIMITER "\n"

#endif


/*! \brief Maximum length for the name of a file. */
#define FILESYSTEM_FILENAME_LENGTH 512

/*! \brief Maximum length for a path. */
#define FILESYSTEM_PATH_LENGTH 4096

/*! \brief Length of the time-string returned by some functions. */
#define FILESYSTEM_TIMESTRING_LENGTH 256

/*! \brief Generic length of a buffer. */
#define FILESYSTEM_BUFFER_LENGTH 1024


/*! \brief Log level constants. */
#define FILESYSTEM_LOGLEVEL_CRITICAL 0
#define FILESYSTEM_LOGLEVEL_DEFAULT  3
#define FILESYSTEM_LOGLEVEL_DEBUG    5
#define FILESYSTEM_LOGLEVEL_VERBOSE  8


/*---------------------------------------------*/
/* Data types                                  */
/*---------------------------------------------*/

/*! \brief List of modes for FileSystem_readDirectory(). */
typedef enum FileSystemReadDirMode
{
    FILESYSTEM_READDIR_ALL, /*!< Read everything. */
            FILESYSTEM_READDIR_FILES, /*!< Read only files. */
            FILESYSTEM_READDIR_DIRS  /*!< Read only directories. */
}
        FileSystemReadDirMode;


/*! \brief List of types returned by the FileSystem_getFileType(). */
typedef enum FileSystemFileType
{
    FILESYSTEM_FILETYPE_UNKNOWN = -1,
    FILESYSTEM_FILETYPE_DIRECTORY = 0,
    FILESYSTEM_FILETYPE_REGULARFILE = 1,
    FILESYSTEM_FILETYPE_BLOCKSPECIALFILE = 2,
    FILESYSTEM_FILETYPE_CHARSPECIALFILE = 3,
    FILESYSTEM_FILETYPE_SYMLINK = 4,
    FILESYSTEM_FILETYPE_NAMEDPIPE = 5,
    FILESYSTEM_FILETYPE_SOCKET = 6
}
        FileSystemFileType;


/*! \brief Status codes. */
typedef enum _FileSystemReturnStatus
{
    FILESYSTEM_STATUS_SUCCESS = 0,
    FILESYSTEM_STATUS_GENERICERROR = -1,
    FILESYSTEM_STATUS_NOTWRITEABLE = -2,
    FILESYSTEM_STATUS_NOTAREGULARFILE = -3,
    FILESYSTEM_STATUS_UNABLETOOPEN = -4,
    FILESYSTEM_STATUS_UNABLETOGETSTATS = -5,
    FILESYSTEM_STATUS_NOTEXECUTABLE = -6,
    FILESYSTEM_STATUS_UNABLETOGETLOCALTIME = -7,
    FILESYSTEM_STATUS_INSUFFICIENTSIZE = -8,
    FILESYSTEM_STATUS_UNABLETOGETCWD = -9,
    FILESYSTEM_STATUS_NOTADIRECTORY = -10,
    FILESYSTEM_STATUS_SUFFIXTOOSMALL = -11,
    FILESYSTEM_STATUS_PATHSIZETOOBIG = -12,
    FILESYSTEM_STATUS_OUTPATHTOOSMALL = -13,
    FILESYSTEM_STATUS_FILEERROR = -14,
    FILESYSTEM_STATUS_INCOMPATIBLETYPES = -15,
    FILESYSTEM_STATUS_DIRSAREDIFFERENT = -16,
    FILESYSTEM_STATUS_UNABLETOGETDIRENTRY = -17,
    FILESYSTEM_STATUS_FILESAREDIFFERENT = -18,
    FILESYSTEM_STATUS_UNABLETOMOVE = -19,
    FILESYSTEM_STATUS_UNABLETORETRIEVEBUFFER = -20,
    FILESYSTEM_STATUS_UNABLETOSEEK = -21,
    FILESYSTEM_STATUS_WRITEERROR = -22,
    FILESYSTEM_STATUS_TARGETFILEISADIRECTORY = -23,
    FILESYSTEM_STATUS_FILETOAPPENDISADIRECTORY = -24
}
        FileSystemReturnStatus;


/*---------------------------------------------*/
/* Functions                                   */
/*---------------------------------------------*/

/*!
 * \brief Return the size of a file/directory.
 *
 * \param path Path to the file/directory.
 *
 * \return  Size of the file/directory in case of success;
 *          FILESYSTEM_STATUS_UNABLETOGETSTATS if it's not possible to access this file/directory stats.
 *
 * \code
 *
 *   BaseI32 retVal                                = 0;
 *   BaseI32 cwdRetVal                             = 0;
 *   char currentDir[ FILESYSTEM_PATH_LENGTH ]     = "";
 *   char filePath[ FILESYSTEM_PATH_LENGTH ]       = "";
 *   char fileName[ FILESYSTEM_FILENAME_LENGTH ]   = "myFile.txt";
 *
 *   cwdRetVal = FileSystem_getCWD( currentDir,
 *                                  sizeof( currentDir ) );
 *
 *   if( cwdRetVal == 0 )
 *   {
 *     strncpy( filePath, currentDir, sizeof( filePath ) );
 *     retVal = FileSystem_implode( filePath, sizeof( filePath ), fileName );
 *
 *     if( retVal == 0 )
 *     {
 *       retVal = FileSystem_getSize( filePath );
 *     }
 *   }
 *
 * \endcode
 */
BaseI32 FileSystem_getSize( const char *path );

/*!
 * \brief Compare two directories.
 *
 * \param first First directory to be compared;
 * \param second Second directory to be compared.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_DIRSAREDIFFERENT in case the two directories are different;
 *         FILESYSTEM_STATUS_UNABLETOGETDIRENTRY if it's not possible to access the stats of one of the two directories;
 *         FILESYSTEM_STATUS_NOTADIRECTORY if at least one of the two parameters doesn't point to a directory.
 *
 * \code
 *
 *  char fullPathFirst[ FILESYSTEM_PATH_LENGTH ]  = "";
 *  char fullPathSecond[ FILESYSTEM_PATH_LENGTH ] = "";
 *
 *  ...
 *  if( FileSystem_isDirectory( fullPathFirst ) &&
 *      FileSystem_isDirectory( fullPathSecond ) )
 *  {
 *    retVal = FileSystem_compareDirectories( fullPathFirst, fullPathSecond );
 *  }
 *  else
 *  {
 *    retVal = FILESYSTEM_STATUS_INCOMPATIBLETYPES;
 *  }
 *
 * \endcode
 */
BaseI32 FileSystem_compareDirectories( const char *first,
                                       const char *second );

/*!
 * \brief Compare two files.
 *
 * \param first First file to be compared;
 * \param second Second file to be compared.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case the two files hold the same content;
 *         FILESYSTEM_STATUS_FILESAREDIFFERENT if the two files are different;
 *         FILESYSTEM_STATUS_UNABLETOOPEN if it's not possible to open at least one of the two files;
 *         FILESYSTEM_STATUS_NOTAFILE if at least one of the two parameters is not a file.
 *
 * \code
 *
 *  char fullPathFirst[ FILESYSTEM_PATH_LENGTH ]  = "";
 *  char fullPathSecond[ FILESYSTEM_PATH_LENGTH ] = "";
 *
 *  ...
 *  if( FileSystem_isRegularFile( fullPathFirst ) &&
 *      FileSystem_isRegularFile( fullPathSecond ) )
 *  {
 *    retVal = FileSystem_compareFiles( fullPathFirst, fullPathSecond );
 *  }
 *  else
 *  {
 *    retVal = FILESYSTEM_STATUS_INCOMPATIBLETYPES;
 *  }
 *
 * \endcode
 */
BaseI32 FileSystem_compareFiles( const char *first,
                                 const char *second );

/*!
 * \brief Compare two files or two directories.
 *
 * \param first First item to be compared;
 * \param second Second item to be compared.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_INCOMPATIBLETYPES if the two items passed as parameters are of different type;
 *         FILESYSTEM_STATUS_FILESAREDIFFERENT if the two items passed as parameters are two files and they are different;
 *         FILESYSTEM_STATUS_UNABLETOOPEN if the two items passed as parameters are two files and it's not possible to open at least one of them.
 *         FILESYSTEM_STATUS_DIRSAREDIFFERENT if the two items passed as parameters are two directories and they are different;
 *         FILESYSTEM_STATUS_UNABLETOGETDIRENTRY  if the two items passed as parameters are two directories and it's not possible to access the stats of one of them;
 *
 * \code
 *
 * void Test_Compare( const char* first, const char* second )
 * {
 *   BaseI32 retVal = 0;
 *
 *   retVal = FileSystem_compare( first, second );
 *   ...
 *   return retVal;
 * }
 *
 * \endcode
 *
 */
BaseI32 FileSystem_compare( const char *first,
                            const char *second );

/*!
 * \brief Copy one file to another.
 *
 * \param fileToCopy Name of the file to be copied;
 * \param targetFile Name of the new file
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_UNABLETOOPEN if it's not possible to open one of the two files passed as parameters;
 *         FILESYSTEM_STATUS_NOTWRITABLE if the target is not writeable;
 *         FILESYSTEM_STATUS_FILEERROR if an error occurred while accessing one of the two files.
 *
 * \code
 *
 *  BaseI32 retVal = 0;
 *  char src[]     = "mainFile.txt";
 *  char dest[]    = "mainFileCopy.txt";
 *
 *  retVal = FileSystem_copyFile( src, dest );
 *  ...
 *
 * \endcode
 */
BaseI32 FileSystem_copyFile( const char *fileToCopy,
                             const char *targetFile );

/*!
 * \brief Delete the content of a directory.
 *
 * \param path Directory to be deleted.
 *
 * \return  FILESYSTEM_STATUS_SUCCESS in case of success;
 *          FILESYSTEM_STATUS_NOTADIRECTORY if \a path is not a directory;
 *
 *
 * \code
 *
 *  isDir = FileSystem_isDirectory( fullPath );
 *  if( isDir == true )
 *  {
 *    ANY_LOG( FILESYSTEM_DEBUG_LOGLEVEL,
 *             "Deleting content of directory >%s<",
 *             ANY_LOG_INFO,
 *             fullPath );
 *    FileSystem_deleteDirContent( fullPath );
 *  }
 *
 * \endcode
 */
BaseI32 FileSystem_deleteDirContent( const char *path );

/*!
 * \brief Append a file to another.
 *
 * \param targetFile File whose content will include the one coming
 *        from the \a fileToAppend one.
 * \param fileToAppend File to be appended.
 *
 * \return  FILESYSTEM_STATUS_SUCCESS in case of success;
 *          FILESYSTEM_STATUS_TARGETFILEISADIRECTORY if the \a targetFile is a directory;
 *          FILESYSTEM_STATUS_FILETOAPPENDISADIRECTORY if the \a fileToAppend is a directory;
 *          FILESYSTEM_STATUS_UNABLETOOPEN if it's not possible to open one of the two files passed as parameters;
 *          FILESYSTEM_STATUS_WRITEERROR if the number of bytes read is different from the number of bytes written;
 *          FILESYSTEM_STATUS_FILEERROR if an error occurred while reading/writing one of the two files.
 *
 * \code
 *
 * void Test_ConcatenateFile( const char* targetFile, const char* fileToAppend )
 * {
 *   BaseI32 retVal      = 0;
 *
 *   ...
 *   retVal = FileSystem_concatenate( targetFile, fileToAppend );
 *   ...
 * }
 *
 * \endcode
 */
BaseI32 FileSystem_concatenate( const char *targetFile,
                                const char *fileToAppend );

/*!
 * \brief Find an executable in a list of paths.
 *
 * \param path String containing the list of paths where to search for the executable;
 * \param fileName Name of the executable file;
 * \param outPath Buffer receiving the path of the executable if found;
 * \param outPathSize Size of the buffer.
 *
 * \return  FILESYSTEM_STATUS_SUCCESS in case of success;
 *          FILESYSTEM_STATUS_PATHSIZETOOBIG if the size of a path obtained by concatenating one of the elements of the \a pathList and the \a fileName
 *                                           is too big;
 *          FILESYSTEM_STATUS_OUTPATHTOOSMALL if the size of the \a outPath is not big enough to accept the resulting path.
 *
 * \code
 *
 *  void Test_Which( const char* path,
 *                   const char* fileName )
 *  {
 *    ...
 *    const BaseUI32 listSize                = EXAMPLE_LIST_ENTRIES_COUNT;
 *    const BaseUI32 listElementSize         = FILESYSTEM_FILENAME_LENGTH;
 *    char outPath[ FILESYSTEM_PATH_LENGTH ] = "";
 *    char currentDir[ FILESYSTEM_PATH_LENGTH ];
 *
 *    cwdRetVal = FileSystem_getCWD( currentDir,
 *                                  FILESYSTEM_PATH_LENGTH );
 *
 *    // Use the list to feed the call.
 *    retVal = FileSystem_which( path,
 *                               fileName,
 *                               outPath,
 *                               FILESYSTEM_PATH_LENGTH );
 *      ...
 *    }
 *    ...
 *  }
 *
 * \endcode
 */
BaseI32 FileSystem_which( const char *path,
                          const char *fileName,
                          char *outPath,
                          const BaseUI32 outPathSize );

/*!
 * \brief Spilt a \a path from its suffix by removing the
 *        \a suffix from the fileName and returning it
 *        in the \a outSuffix parameter.
 *
 * \param fileName Name to be split;
 * \param outSuffix Buffer receiving the suffix of the file;
 * \param suffixSize Size of the \a buffer for the suffix.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_SUFFIXTOOSMALL if the size of the suffix buffer is not big enough.
 *
 * \code
 *
 *  char localFileName[ FILESYSTEM_FILENAME_LENGTH ] = "myFile.txt";
 *  char suffix[ EXAMPLE_SHORT_BUFFERSIZE ]          = "";
 *
 *  retVal = FileSystem_removeSuffixFromFileName( localFileName,
 *                                                suffix,
 *                                                sizeof( suffix ) );
 *  // Now localFileName = "myFile" and suffix = "txt".
 *
 * \endcode
 *
 */
BaseI32 FileSystem_removeSuffixFromFileName( char *fileName,
                                             char *outSuffix,
                                             BaseUI32 suffixSize );

/*!
 * \brief Spilt a \a path into a list of directories.
 *
 * \param path Path to be split;
 * \param list List receiving the parts of the path to be split;
 * \param listSize Number of elements in the list;
 * \param listElementSize Size of each list element.
 *
 * \return The size of the list in case of success;
 *         A negative error code otherwise.
 *
 * \code
 *
 *  char path[] = "usr/lib:"
 *                "../examples/precise64:"
 *                "/home/Projects/Libraries/FileSystem/2.0/";
 *  BaseUI32 listSize = 256;
 *  BaseUI32 listElementSize = 4096;
 *  BaseI32 retVal       = 0;
 *  BaseUI32 i           = 0;
 *  char** directoryList = ( char** )NULL;
 *
 *  directoryList = ANY_MALLOC( listSize, sizeof( char* ) );
 *
 *  // Allocate the basic structure of the list.
 *  if( directoryList == NULL )
 *  {
 *    ANY_LOG( FILESYSTEM_DEFAULT_LOGLEVEL,
 *             "Unable to allocate directory list",
 *             ANY_LOG_INFO );
 *    return;
 *  }
 *
 *  // Allocate each of the elements of the list.
 *  for( i = 0; i < listSize; i++ )
 *  {
 *    directoryList[ i ] = ANY_MALLOC( listElementSize, sizeof( char ) );
 *    if( directoryList == NULL )
 *    {
 *      ANY_LOG( FILESYSTEM_DEFAULT_LOGLEVEL,
 *               "Unable to allocate directory list element no.%d",
 *                ANY_LOG_INFO,
 *                i );
 *      return;
 *    }
 *  }
 *
 *  retVal = FileSystem_explodePath( path,
 *                                   directoryList,
 *                                   listSize,
 *                                   listElementSize );
 *
 *  // Now directoryList[ 0 ] = "usr/lib",
 *  //     directoryList[ 1 ] = "../examples/precise64",
 *  //     directoryList[ 2 ] = "/home/Projects/Libraries/FileSystem/2.0/".
 *
 * \endcode
 */
BaseI32 FileSystem_explodePath( const char *path,
                                char *list,
                                const BaseUI32 listSize,
                                const BaseUI32 listElementSize );

/*!
 * \brief Decomposes a \a path into two parts, a \a branch and
 *        \a leaf.
 *
 * \param path Path to be decomposed;
 * \param branch First part of the \a path;
 * \param branchSize Size of the \a branch buffer;
 * \param leaf Final file/directory of the \a path;
 * \param leafSize Size of the \a leaf buffer.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_INSUFFICIENTSIZE in case the size of the leaf buffer or the one of the branch buffer is not big enough.
 *
 * \code
 *
 * char path[]                             = "../examples/precise64";
 * BaseI32 retVal                          = 0;
 * char branch[ FILESYSTEM_PATH_LENGTH ]   = "";
 * char leaf[ FILESYSTEM_FILENAME_LENGTH ] = "";
 *
 * Any_memset( branch, '\0', FILESYSTEM_PATH_LENGTH );
 * Any_memset( leaf, '\0', FILESYSTEM_FILENAME_LENGTH );
 *
 * retVal = FileSystem_decomposePath( path,
 *                                    branch,
 *                                    sizeof( branch ),
 *                                    leaf,
 *                                    sizeof( leaf ) );
 *
 * // Now branch holds the value "../examples" and
 * // leaf holds "precise64".
 *
 * \endcode
 */
BaseI32 FileSystem_decomposePath( const char *path,
                                  char *branch,
                                  BaseUI32 branchSize,
                                  char *leaf,
                                  BaseUI32 leafSize );

/*!
 * \brief Retrieves modification data about the file pointed by
 *        \a path.
 *
 * \param path Path of the file whose modification data has to
 *             be retrieved;
 * \param modTime Time structure receiving the data about the file.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_UNABLETOGETSTATS in case it's not possible to extract statistics for the file pointed by \a path;
 *         FILESYSTEM_STATUS_UNABLETOGETLOCALTIME if it's not possible to get the modification time of the file pointed by \a path.
 *
 * \code
 *
 *  ...
 *  char filePath[ FILESYSTEM_PATH_LENGTH ] = "/home/Projects/Libraries/"
 *                                            "FileSystem/2.0/examples/"
 *                                            "precise64/Examples";
 *  struct tm modTime;
 *
 *  retVal = FileSystem_getModificationTime( filePath,
 *                                           &modTime );
 *  ...
 *
 * \endcode
 */
BaseI32 FileSystem_getModificationTime( const char *path,
                                        struct tm *modTime );

/*!
 * \brief Checks if \a path is an executable.
 *
 * \param path Path of the file to check;
 *
 * \attention This function is currently not supported under Windows
 *            and will return "0" (decimal) if the file exists,
 *            or \c FILESYSTEM_STATUS_UNABLETOGETSTATS else.
 *
 * \return true if \a path is an executable;
 *         false otherwise.
 *
 * \code
 *
 *  BaseBool retVal = false;
 *  char filePath[ FILESYSTEM_PATH_LENGTH ] = "/home/Projects/Libraries/"
 *                                            "FileSystem/2.0/examples/"
 *                                            "precise64/Examples";
 *  retVal = FileSystem_isExecutable( filePath );
 *  ...
 *
 * \endcode
 */
BaseBool FileSystem_isExecutable( const char *path );

/*!
 * \brief Checks if \a path is writeable.
 *
 * \param path Path of the file to check;
 *
 * \return true if \a path is an writeable;
 *         false otherwise.
 *
 * \code
 *
 *  BaseBool retVal = false;
 *  char filePath[ FILESYSTEM_PATH_LENGTH ] = "/home/Projects/Libraries/"
 *                                            "FileSystem/2.0/examples/"
 *                                            "Examples.c";
 *  retVal = FileSystem_isWriteable( filePath );
 *  ...
 *
 * \endcode
 */
BaseBool FileSystem_isWriteable( const char *path );

/*!
 * \brief Read the file pointed by \a path into a memory \a block.
 *
 * \param path Path of the file to read;
 * \param block Block receiving the content fo the file;
 * \param offset Position where to start writing on the block.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_UNABLETOOPEN if it's not possible to open the file pointed by \a filePath;
 *         FILESYSTEM_STATUS_UNABLETORETRIEVEBUFFER if it's not possible to retrieve the buffer from \a block;
 *         FILESYSTEM_STATUS_UNABLETOSEEK if it's not possible to seek to position \a offset;
 *         FILESYSTEM_STATUS_NOTAREGULARFILE the file pointed by \a filePath is not a regular file.
 *
 * \code
 *
 *  char filePath[ FILESYSTEM_PATH_LENGTH ]   = "/home/Projects/Libraries/"
 *                                              "FileSystem/2.0/examples/"
 *                                              "works/fileToRead.txt";
 *  MemI8 *block                              = ( MemI8* )NULL;
 *  BaseI32 offset                            = 0;
 *
 *  block = MemI8_new();
 *
 *  if( block == NULL )
 *  {
 *    return;
 *  }
 *
 *  if( MemI8_init( block, 4096 ) != 0 )
 *  {
 *    return;
 *  }
 *
 *  retVal = FileSystem_readFile( filePath,
 *                                block,
 *                                offset );
 *
 * \endcode
 *
 */
BaseI32 FileSystem_readFile( const char *path,
                             MemI8 *block,
                             const BaseUI32 offset );

/*!
 * \brief Moves a file.
 *
 * \param source Path of the file to be moved;
 * \param dest Path of the destination file.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_PATHSIZETOOBIG if the size of the \a dest path is too big;
 *         FILESYSTEM_STATUS_NOTAREGULARFILE if \a source is not a regular file;
 *         FILESYSTEM_STATUS_UNABLETOMOVE if it's not possible to complete the move;
 *         FILESYSTEM_STATUS_NOTWRITABLE if \a dest is not writeable.
 *
 * This method moves the file from \a source to \a dest.
 *
 * \code
 *
 *   BaseI32 retVal                            = 0;
 *   char sourcePath[ FILESYSTEM_PATH_LENGTH ] = "/home/Projects/Libraries/"
 *                                               "FileSystem/2.0/examples/"
 *                                               "works/fileToMove.txt";
 *   char destPath[ FILESYSTEM_PATH_LENGTH ]   = "/home/Projects/Libraries/"
 *                                               "FileSystem/2.0/examples/"
 *                                               "works/fileMoved.txt";
 *
 *   retVal = FileSystem_move( sourcePath, destPath );
 *   ...
 *
 * \endcode
 *
 */
BaseI32 FileSystem_move( const char *source,
                         const char *dest );

/*!
 * \brief Deletes a name from the filesystem.
 *
 * \param path Path of the filesystem object to be removed
 *             (file, folder, symbolic link, socket).
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_GENERICERROR in case of error.
 *
 * This method deletes the filesystem object pointed by \a path.
 *
 * If the filesystem object is a directory, it needs to
 * be empty.
 *
 * If the path was the last link to a file and no processes
 * have the file open the file is deleted and the space it was using
 * is made available for reuse.
 *
 * If the path was the last link to a file but any processes still
 * have the file open the file will remain in existence until the
 * last file descriptor referring to it is closed.
 *
 * If the path referred to a symbolic link, the link is removed and
 * the linked object remain untouched.
 *
 * If the path referred to a socket, fifo or device, the name for it
 * is removed but processes which have the object open may continue
 * to use it.
 *
 *
 * \code
 *
 *   char fullPath[ FILESYSTEM_PATH_LENGTH ] = "/home/Projects/Libraries/"
 *                                             "FileSystem/2.0/examples/"
 *                                             "works/fileToDelete.txt";
 *   retVal = FileSystem_remove( fullPath );
 *   ...
 *
 * \endcode
 */
BaseI32 FileSystem_remove( const char *path );

/*!
 * \brief Extend a given path by appending a file or directory name.
 *
 * \param path Path to extend;
 * \param size Size of the \a path buffer;
 * \param toAdd File or directory name to add to \a path.
 *
 * \return FILESYSTEM_STATUS_SUCCESS in case of success;
 *         FILESYSTEM_STATUS_INSUFFICIENTSIZE if \a size is not enough to concatenate \a toAdd to \a path.
 *
 * This method sets \a path to \a path + "/" + \a toAdd.
 * The nice thing about this function is that the result is correct no matter
 * if the original path ends with the directory delimiter "/" or not.
 *
 * \see FileSystem_decomposePath
 *
 * \code
 *
 * char fileName[ FILESYSTEM_FILENAME_LENGTH ] = "fileToRead.txt";
 * char filePath[ FILESYSTEM_PATH_LENGTH ]     = "/home/Projects/Libraries/"
 *                                               "FileSystem/2.0/examples/"
 *                                               "works/";
 *
 * retVal = FileSystem_implode( filePath, sizeof( filePath ), fileName );
 *
 * // Now filePath = "/home/Projects/Libraries/FileSystem/2.0/"
 *                   "examples/works/fileToRead.txt".
 *
 * \endcode
 *
 */
BaseI32 FileSystem_implode( char *path,
                            const BaseI32 size,
                            const char *toAdd );

/*!
 * \brief Get current directory.
 *
 * \param path Buffer that will receive the current directory path;
 * \param size Size of the buffer.
 *
 * \return FILESYSTEM_STATUS_SUCCESS on success;
 *         FILESYSTEM_STATUS_INSUFFICIENTSIZE if the size of the buffer is not bit enough;
 *         FILESYSTEM_STATUS_UNABLETOGETCWD if an error occurred.
 *
 * \code
 *
 *   BaseI32 cwdRetVal                         = 0;
 *   char currentDir[ FILESYSTEM_PATH_LENGTH ] = "";
 *
 *   cwdRetVal = FileSystem_getCWD( currentDir,
 *                                  sizeof( currentDir ) );
 *
 * \endcode
 */
BaseI32 FileSystem_getCWD( char *path, BaseUI32 size );

/*!
 * \brief Change current directory.
 *
 * \param path Path to the directory to change to.
 *
 * \return  FILESYSTEM_STATUS_SUCCESS on success;
 *          FILESYSTEM_STATUS_GENERICERROR if an error occurs.
 *
 * \code
 *
 *  BaseI32 chdRetVal  = 0;
 *  char newDir[]      = "./precise64";
 *
 *  chdRetVal = FileSystem_changeDirectory( newDir );
 *
 * \endcode
 */
BaseI32 FileSystem_changeDirectory( const char *path );

/*!
 * \brief Create new directory and grant the owner read, write and
 *        execute authority.
 *
 * \param path Path to the directory to be created.
 *
 * \return FILESYSTEM_STATUS_SUCCESS on success;
 *         FILESYSTEM_STATUS_GENERICERROR if an error occurs.
 *
 * \code
 *
 * FileSystem_makeDirectory( "dirToEmpty" );
 *
 * \endcode
 */
BaseI32 FileSystem_makeDirectory( const char *path );


/*!
 * \brief Create a directory tree (like "mkdir -p")
 *
 * Creates the specified directory, and all the path in between
 * (if neccessary).
 *
 * \return true on success, false otherwise
 */
BaseBool FileSystem_makeDirectories( const char *path );


/*!
 * \brief Read the entries of a directory.
 *
 * \param path Path to the directory to read;
 * \param mode Mode for reading, i.e., all entries, only files, or only
 *             subdirectories;
 * \param pattern Optional pattern that all returned entries must match.
 *                A \c NULL pointer is equivalent to "*", i.e. match all;
 * \param list List receiving the entries of the directory pointed by \a path;
 * \param listSize Number of elements of the \a list;
 * \param listElementSize Size of each element of the \a list.
 *
 * \return  The number of elements in the list on success;
 *          FILESYSTEM_STATUS_NOTADIRECTORY if \a path is not a directory;
 *          FILESYSTEM_STATUS_PATHSIZETOOBIG if the size of the \a path buffer is too big.
 *
 * \attention The entries "." and ".." are never included in the returned
 *            list, they are always skipped no matter which \a mode or \a
 *            pattern is specified.
 *
 * \code
 *
 *   BaseI32 retVal                            = 0;
 *   BaseI32 i                                 = 0;
 *   char** dirListing                         = ( char** )NULL;
 *   char currentDir[ FILESYSTEM_PATH_LENGTH ] = "";
 *   BaseUI32 listElementSize                  = 5;
 *   FileSystemReadDirMode mode                = FILESYSTEM_READDIR_ALL;
 *
 *   dirListing = ANY_MALLOC( EXAMPLE_LIST_ENTRIES_COUNT, sizeof( char* ) );
 *
 *   for ( i = 0; i < EXAMPLE_LIST_ENTRIES_COUNT; i++ )
 *   {
 *     dirListing[ i ] = ANY_MALLOC( listElementSize, sizeof( char ) );
 *   }
 *
 *   retVal = FileSystem_readDirectory( currentDir,
 *                                      mode,
 *                                      NULL,
 *                                      dirListing,
 *                                      EXAMPLE_LIST_ENTRIES_COUNT,
 *                                      listElementSize );
 *   ...
 *
 * \endcode
 */
BaseI32 FileSystem_readDirectory( const char *path,
                                  const FileSystemReadDirMode mode,
                                  const char *pattern,
                                  char *list,
                                  const BaseUI32 listSize,
                                  const BaseUI32 listElementSize );

/*!
 * \brief Check whether the path passed as parameter points to a directory.
 *
 * \param path Path of the directory to check.
 *
 * \attention The "pattern" searching is currently not supported under
 *            Windows!
 *
 * \return true if the path passed as parameter points to a directory;
 *         false otherwise.
 *
 * \code
 *
 *   BaseBool retVal                           = false;
 *   BaseBool cwdRetVal                        = 0;
 *   char currentDir[ FILESYSTEM_PATH_LENGTH ] = "";
 *
 *   cwdRetVal = FileSystem_getCWD( currentDir,
 *                                  FILESYSTEM_PATH_LENGTH );
 *   if( cwdRetVal == 0 )
 *   {
 *     retVal = FileSystem_isDirectory( currentDir );
 *   }
 *
 * \endcode
 */
BaseBool FileSystem_isDirectory( const char *path );

/*!
 * \brief Check wether the path passed as parameter points to a file.
 *
 * \param path Path of the file to check.
 *
 * \return true if the path passed as parameter points to a file;
 *         false otherwise.
 *
 * \code
 *
 *  BaseBool retVal    = false;
 *  char fileToCheck[] = "./mainFile.txt";
 *
 *  retVal = FileSystem_isRegularFile( fileToCheck );
 *
 * \endcode
 */
BaseBool FileSystem_isRegularFile( const char *path );

/*!
 * \brief Check wether the path passed as parameter points to a file.
 *
 * \param path Path of the file to check.
 *
 * \return true if the path passed as parameter is a symbolic link;
 *         false otherwise.
 *
 * \code
 *
 *   BaseBool retVal    = false;
 *   char fileToCheck[] = "./lnkMainFile.txt";
 *
 *   retVal = FileSystem_isSymLink( fileToCheck );
 *
 * \endcode
 */
BaseBool FileSystem_isSymLink( const char *path );

/*!
 * \brief Check wether the stats struct passed as parameter points
 *        to a socket.
 *
 * \param path stat strcut of the descriptor to check.
 *
 * \return true if the path passed as parameter points to a socket file;
 *         false otherwise.
 *
 * \code
 *
 *   BaseBool retVal    = false;
 *   char fileToCheck[] = "";
 *
 *   retVal = FileSystem_isSocket( fileToCheck );
 *
 * \endcode
 */
BaseBool FileSystem_isSocket( const char *path );

/*!
 * \brief Check wether the path passed as parameter points
 *        to a named pipe.
 *
 * \param path Path to the file to check.
 *
 * \attention Under Windows this function always returns \c false.
 *
 * \return true if the path passed as parameter points to a named pipe file;
 *         false otherwise. Under Windows always returns \c false.
 *
 * \code
 *
 *   BaseBool retVal    = false;
 *   char fileToCheck[] = "";
 *
 *   retVal = FileSystem_isNamedPipe( fileToCheck );
 *
 * \endcode
 */
BaseBool FileSystem_isNamedPipe( const char *path );

/*!
 * \brief Check wether the path passed as parameter points
 *        to a device special file.
 *
 * \param path Path to the file to check.
 *
 * \return true if the path passed as parameter points to a block special file (e.g. a disk);
 *         false otherwise.
 *
 * \code
 *
 *   BaseBool retVal    = false;
 *   char fileToCheck[] = "";
 *
 *   retVal = FileSystem_isBlockSpecialFile( fileToCheck );
 *
 * \endcode
 */
BaseBool FileSystem_isBlockSpecialFile( const char *path );

/*!
 * \brief Check wether the path passed as parameter points
 *        to a character special file.
 *
 * \param path Path to the file to check.
 *
 * \return true if the path passed as parameter points to a character special file (e.g. a terminal);
 *         false otherwise;
 *
 * \code
 *
 *   BaseBool retVal    = false;
 *   char fileToCheck[] = "";
 *
 *   retVal = FileSystem_isCharacterSpecialFile( fileToCheck );
 *
 * \endcode
 */
BaseBool FileSystem_isCharacterSpecialFile( const char *path );

/*!
 * \brief Return the type of the type passed as parameter
 *
 * \param path Path to the file to check.
 *
 * \return enum stating the type of the file.
 *
 * \code
 *
 * FileSystemFileType fileType             = FILESYSTEM_FILETYPE_DIRECTORY;
 * char filePath[ FILESYSTEM_PATH_LENGTH ] = "/home/Projects/Libraries/"
 *                                           "FileSystem/2.0/examples/"
 *                                           "works/fileToRead.txt";
 *
 * fileType = FileSystem_getFileType( filePath );
 *
 * switch( fileType )
 * {
 *   case FILESYSTEM_FILETYPE_REGULARFILE:
 *   {
 *     strcpy( fileTypeString, "regular file" );
 *     break;
 *   }
 *   case FILESYSTEM_FILETYPE_DIRECTORY:
 *   {
 *     ...
 *   }
 * }
 *
 * \endcode
 */
FileSystemFileType FileSystem_getFileType( const char *path );

/*!
 * \brief Assigns a textual description to the \a stringBuffer
 *        passed as parameter according to the \a errorCode in input.
 *
 * \param errorCode Code of the error to be translated into a string;
 * \param prefix Prefix string to be inserted at the beginning of the
 *               error message copied into the \a stringBuffer;
 * \param stringBuffer Buffer that will receive the error description;
 * \param bufferSize Size of the \a stringBuffer.
 *
 * \return FILESYSTEM_STATUS_SUCCESS on success;
 *         FILESYSTEM_STATUS_UNSUFFICIENTSIZE if the \a bufferSize is
 *                                            too small.
 *
 * \code
 *
 *   BaseI32 retVal                                = FILESYSTEM_STATUS_SUCCESS;
 *   char *errorBuffer[ FILESYSTEM_BUFFER_LENGTH ] = ""
 *
 *   retVal = FileSystem_getCWD( currentDir,
 *                               sizeof( currentDir ) );
 *
 *   if( retVal != FILESYSTEM_STATUS_SUCCESS )
 *   {
 *     FileSystem_strerror( retVal, errorBuffer, sizeof( errorBuffer );
 *     ANY_LOG( 0, errorBuffer, ANY_LOG_INFO );
 *   }
 *
 *
 * \endcode
 */
BaseI32 FileSystem_strerror( FileSystemReturnStatus errorCode,
                             char *prefix,
                             char *stringBuffer,
                             BaseUI32 bufferSize );

#ifdef __cplusplus
} //extern C
#endif

#endif  /* FILESYSTEM_H */


/* EOF */
