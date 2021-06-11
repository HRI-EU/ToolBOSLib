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


#include <ctype.h>
#include <string.h>

#if !defined(__windows__)

#include <strings.h>

#endif

#include <UString.h>


#define USTRING_BUFFER_SIZE 4096


int UString_explode( const char *string,
                     char *list,
                     const unsigned int listSize,
                     const unsigned int listElementSize,
                     const char token )
{
    int retVal = 0;
    int pos = 0;
    int i = 0;
    int start = 0;
    bool found = false;
    unsigned int listIndex = 0;
    int stringLen = 0;
    int charCount = 0;
    char buf[USTRING_BUFFER_SIZE] = "";
    char *tmp = NULL;

    ANY_REQUIRE( string );
    ANY_REQUIRE( list );
    ANY_REQUIRE( listSize > 0 );
    ANY_REQUIRE( listElementSize > 0 );

    pos = -1;
    stringLen = (int)Any_strlen( string );

    while(( pos < (int)stringLen ) &&
          ( stringLen > 0 ))
    {
        start = pos + 1;
        i = start;
        found = false;
        while( i < (int)stringLen )
        {
            if( string[ i++ ] == token )
            {
                pos = i - 1;
                found = true;
                break;
            }
        }

        if( found == false )  /* The while above terminated because 'i' reached  */
        {                     /* the same value of 'stringLen'. */
            pos = i;
        }

        charCount = pos - start;
        Any_strncpy( buf, string + start, sizeof( buf ));
        buf[ charCount ] = '\0';

        tmp = list + ( listIndex * listElementSize * sizeof( char ));

        if( Any_strlen( buf ) > 0 )
        {
            if( Any_strlen( buf ) < listElementSize )
            {
                Any_strncpy( tmp, buf, listElementSize );
            }
            else
            {
                Any_strncpy( tmp, buf, listElementSize - 1 );
                tmp[ listElementSize - 1 ] = '\0';
            }
        }
        else
        { /* The buffer is empty, let's fill up an empty list element. */
            Any_strncpy( tmp, "\0", listElementSize );
        }

        listIndex++;

        if(( listIndex >= listSize ) ||
           ( pos == ((int)stringLen - 1 )))
        {
            break;
        }
    }  /* End while( pos < ... ). */

    if( retVal >= 0 )
    {
        retVal = listIndex;
    }

    return retVal;
}


int UString_replaceChrWithStr( char *destString,
                               int destStringLen,
                               char *srcString,
                               char chr,
                               char *str )
{
    char srcArr[] = { chr, '\0' };
    char *replArr[] = { str, (char *)NULL };

    ANY_REQUIRE( destString );
    ANY_REQUIRE( destStringLen > 0 );
    ANY_REQUIRE( srcString );
    ANY_REQUIRE( str );

    return UString_replaceChrArrayWithStrArray( destString, destStringLen, srcString, srcArr, replArr );
}


int UString_replaceChrArrayWithStrArray( char *destString,
                                         int destStringLen,
                                         char *srcString,
                                         char findChr[],
                                         char *replaceString[] )
{
    int prevSrcPos = 0;
    int currSrcPos = 0;
    int currDestPos = 0;
    int srcStringLen = 0;
    int replStrLen = 0;
    int numEnries = 0;
    int entryPos = 0;

    ANY_REQUIRE( destString );
    ANY_REQUIRE( destStringLen > 0 );
    ANY_REQUIRE( srcString );
    ANY_REQUIRE( findChr );
    ANY_REQUIRE( replaceString );

    srcStringLen = Any_strlen( srcString );
    ANY_REQUIRE( srcStringLen < destStringLen );

    numEnries = Any_strlen( findChr );
    entryPos = 0;
    while( replaceString[ entryPos ] != (char *)NULL && entryPos < numEnries )
    {
        entryPos++;
    }

    ANY_REQUIRE_MSG( entryPos == numEnries && replaceString[ entryPos ] == (char *)NULL,
                     "findChr and replaceString do not have the same length!" );

    *destString = '\0';

    for( currDestPos = prevSrcPos = currSrcPos = 0; currSrcPos < srcStringLen; ++currSrcPos )
    {
        for( entryPos = 0; entryPos < numEnries; entryPos++ )
        {
            if( srcString[ currSrcPos ] == findChr[ entryPos ] )
            {
                if( currDestPos + currSrcPos - prevSrcPos >= destStringLen )
                {
                    return false;
                }

                Any_memcpy( &( destString[ currDestPos ] ),
                            &srcString[ prevSrcPos ],
                            currSrcPos - prevSrcPos );

                currDestPos += currSrcPos - prevSrcPos;

                replStrLen = Any_strlen( replaceString[ entryPos ] );

                if( currDestPos + replStrLen >= destStringLen )
                {
                    return 1;
                }

                Any_strcpy( &( destString[ currDestPos ] ), replaceString[ entryPos ] );

                currDestPos += replStrLen;
                prevSrcPos = currSrcPos + 1;
            }
        }
    }

    if( prevSrcPos != currSrcPos )
    {
        if( currDestPos + currSrcPos - prevSrcPos >= destStringLen )
        {
            return 1;
        }

        Any_memcpy( &( destString[ currDestPos ] ),
                    &srcString[ prevSrcPos ],
                    currSrcPos - prevSrcPos + 1 );
    }

    return 0;
}


int UString_startsWithChr( const char *string, const char ch )
{
    int length = 0;

    ANY_REQUIRE( string );

    length = Any_strlen( string );

    if( length < 1 )
    {
        return 0;
    }

    return string[ 0 ] == ch;
}


int UString_endsWithChr( const char *string, const char ch )
{
    int length = 0;

    ANY_REQUIRE( string );

    length = strlen( string );

    if( length < 1 )
    {
        return 0;
    }

    return string[ length - 1 ] == ch;
}


int UString_startsWith( const char *string, const char *startStr )
{
    int result = 0;
    int index = 0;

    ANY_REQUIRE( string );
    ANY_REQUIRE( startStr );

    if( string )
    {
        if( startStr )
        {
            while(( string[ index ] == startStr[ index ] ) &&
                  ( string[ index ] ) && ( startStr[ index ] ))
            {
                ++index;
            }

            if( startStr[ index ] )
            {
                result = 1;
            }
        }
    }
    else if( startStr )
    {
        result = 1;
    }

    return result;
}


int UString_endsWith( const char *string, const char *endStr )
{
    int result = 0;
    int index = 0;
    int stringLen = 0;
    int tmp = 0;

    ANY_REQUIRE( string );
    ANY_REQUIRE( endStr );

    if( string )
    {
        if( endStr )
        {
            stringLen = Any_strlen( string );
            tmp = Any_strlen( endStr );

            if( stringLen < tmp )
            {
                result = 1;
            }
            else
            {
                index = stringLen - tmp;
                tmp = index;

                while(( string[ index ] == endStr[ index - tmp ] ) &&
                      ( string[ index ] ) && ( endStr[ index - tmp ] ))
                {
                    ++index;
                }

                if( endStr[ index - tmp ] )
                {
                    result = 1;
                }
            }
        }
    }
    else if( endStr )
    {
        result = 1;
    }

    return result;
}


int UString_getFirstPos( const char *string, const char sign )
{
    return UString_getFirstPosChr( string, sign );
}


int UString_getFirstPosChr( const char *string, const char sign )
{
    int i = 0;
    int length = 0;
    char *result = (char *)NULL;

    ANY_REQUIRE( string );

    result = (char *)string;

    length = Any_strlen( result );

    for( i = 0; i < length; i++ )
    {
        if( result[ i ] == sign )
        {
            return i;
        }
    }

    return -1;
}


int UString_getLastPos( const char *string, const char sign )
{
    return UString_getLastPosChr( string, sign );
}


int UString_getLastPosChr( const char *string, const char sign )
{
    int i = 0;
    char *result = (char *)NULL;

    ANY_REQUIRE( string );

    result = (char *)string;

    for( i = Any_strlen( result ); i > 0; i-- )
    {
        if( result[ i ] == sign )
        {
            return i;
        }
    }
    return -1;
}


int UString_findNextOccurrenceChr( const char *string, char searchedChar, int start )
{
    int stringLength = 0;
    int result;

    ANY_REQUIRE( string );
    ANY_REQUIRE( searchedChar );

    stringLength = Any_strlen( string );

    if( start < 0 || start > stringLength )
    {
        start = 0;
    }

    result = start;

    while( result < stringLength )
    {
        if( string[ result ] == searchedChar )
        {
            break;
        }
        ++result;
    }

    if( result == ( stringLength - 1 ))
    {
        result = -1;
    }
    return result;
}


int UString_findPreviousOccurrenceChr( const char *string, char searchedChar, int start )
{
    int result = 0;
    int stringLength = 0;

    ANY_REQUIRE( string );
    ANY_REQUIRE( searchedChar );

    stringLength = Any_strlen( string );

    if( start < 0 || start > stringLength )
    {
        start = stringLength;
    }

    result = start;

    while( result > -1 )
    {
        if( string[ result ] == searchedChar )
        {
            break;
        }

        result--;
    }

    return result;
}


int UString_findNextOccurrence( const char *string, const char *searchedStr, int start )
{
    int stringLength = 0;
    int searchedLength = 0;
    int count = 0;
    int result = -1;
    int i = 0;

    ANY_REQUIRE( string );
    ANY_REQUIRE( searchedStr );

    stringLength = Any_strlen( string );
    searchedLength = Any_strlen( searchedStr );

    if( start < 0 || start > stringLength )
    {
        start = 0;
    }

    for( i = start; i < stringLength; i++ )
    {
        if( string[ i ] == searchedStr[ 0 ] )
        {
            count = 1;
            continue;
        }

        if( count > 0 )
        {
            if( string[ i ] == searchedStr[ count ] )
            {
                ++count;
            }
            else
            {
                count = 0;
            }
        }

        if( count == searchedLength )
        {
            result = i - count + 1;
            break;
        }
    }

    return result;
}


int UString_findPreviousOccurrence( const char *string,
                                    const char *searchedStr,
                                    int start )
{
    int stringLength = 0;
    int searchedLength = 0;
    int count = 1;
    int result = -1;
    int i = 0;

    ANY_REQUIRE( string );
    ANY_REQUIRE( searchedStr );

    stringLength = Any_strlen( string );
    searchedLength = Any_strlen( searchedStr );

    if( start < 0 || start > stringLength )
    {
        start = stringLength;
    }

    for( i = start; i >= 0; i-- )
    {
        if( string[ i ] == searchedStr[ searchedLength - 1 ] )
        {
            count = searchedLength - 1;
            continue;
        }

        if( count > 1 )
        {
            if( string[ i ] == searchedStr[ searchedLength - count ] )
            {
                ++count;
            }
            else
            {
                count = 1;
            }
        }

        if( count == searchedLength )
        {
            result = i - 1;
            break;
        }
    }

    return result;

}


int UString_getOccurrences( const char *string, const char sign )
{
    return UString_getNumberOccurrencesChr( string, sign );
}


int UString_getNumberOccurrencesChr( const char *string, const char sign )
{
    int i = 0;
    int result = 0;
    int length = 0;

    ANY_REQUIRE( string );

    length = Any_strlen( string );

    for( i = 0; i < length; i++ )
    {
        if( string[ i ] == sign )
        {
            result++;
        }
    }

    return result;
}


int UString_getNumberOccurrences( const char *string, const char *searchedStr )
{
    int stringLength = 0;
    int searchedLength = 0;
    int count = 0;
    int result = 0;
    int i = 0;

    ANY_REQUIRE( string );
    ANY_REQUIRE( searchedStr );

    stringLength = Any_strlen( string );
    searchedLength = Any_strlen( searchedStr );

    for( i = 0; i < stringLength; i++ )
    {
        if( string[ i ] == searchedStr[ 0 ] )
        {
            count = 1;
            continue;
        }

        if( count > 0 )
        {
            if( string[ i ] == searchedStr[ count ] )
            {
                ++count;
            }
            else
            {
                count = 0;
            }
        }

        if( count == searchedLength )
        {
            ++result;
            count = 0;
        }
    }

    return result;
}


char *UString_trim( char *string )
{
    return UString_trimRight( UString_trimLeft( string ));
}


char *UString_trimLeft( char *string )
{
    int firstNonBlank = 0;
    int c = 0;

    ANY_REQUIRE( string );

    //search for the first non_blank character in the string
    while( string[ firstNonBlank ] && isspace( string[ firstNonBlank ] ))
    {
        firstNonBlank++;
    }

    while( string[ c ] != '\0' )
    {
        string[ c ] = string[ c + firstNonBlank ];
        c++;
    }

    return string;
}


char *UString_trimRight( char *string )
{
    size_t size = 0;
    char *end = (char *)NULL;

    ANY_REQUIRE( string );

    size = Any_strlen( string );

    if( size > 0 )
    {
        end = string + size - 1;

        while( end >= string && isspace( *end ))
        {
            end--;
        }

        ANY_REQUIRE( end );
        *( end + 1 ) = '\0';
    }

    return string;
}


char *UString_getSubStr( char *destString, int destStringSize, const char *sourceString, int start, int end )
{
    int length = 0;
    int resultLength = 0;

    ANY_REQUIRE( sourceString );
    ANY_REQUIRE( destString );

    length = Any_strlen( sourceString );

    if( start < 0 )
    {
        start = 0;
    }

    if( end > length )
    {
        end = length;
    }

    if( end - start > destStringSize )
    {
        resultLength = destStringSize;
    }
    else
    {
        resultLength = end - start;
    }

    Any_strncpy( destString, &( sourceString[ start ] ), resultLength );

    return destString;
}


int UString_compare( const char *string1, const char *string2 )
{
    ANY_REQUIRE( string1 );
    ANY_REQUIRE( string2 );

    return Any_strcmp( string1, string2 );
}


int UString_compareCase( const char *string1, const char *string2 )
{
    ANY_REQUIRE( string1 );
    ANY_REQUIRE( string2 );

    return Any_strcasecmp( string1, string2 );
}


int UString_compareCaseN( const char *string1, const char *string2, const int len )
{
    ANY_REQUIRE( string1 );
    ANY_REQUIRE( string2 );

    return Any_strncasecmp( string1, string2, len );
}


int UString_getLength( char *string )
{
    ANY_REQUIRE( string );

    return Any_strlen( string );
}


char *UString_append( char *destString, int destStringSize, char *string )
{
    int destStringLen = 0;
    int stringLen = 0;
    int strLenToCopy = 0;

    ANY_REQUIRE( destString );
    ANY_REQUIRE( string );
    ANY_REQUIRE( destStringSize >= 0 );

    destStringLen = Any_strlen( destString );
    stringLen = Any_strlen( string );

    if( stringLen < destStringSize - destStringLen )
    {
        strLenToCopy = stringLen;
    }
    else
    {
        strLenToCopy = destStringSize - destStringLen - 1;
    }

    Any_strncat( destString, string, strLenToCopy );

    return destString;
}


char *UString_appendChr( char *destString, char ch, int size )
{
    int length = 0;

    ANY_REQUIRE( destString );

    if( size < 0 )
    {
        size = 0;
    }

    length = Any_strlen( destString );

    if( size >= length + 2 )
    {
        destString[ length ] = ch;
        destString[ length + 1 ] = '\0';
    }

    return destString;
}


int UString_snprintf( char *str, unsigned int size, const char *format, ... )
{
    va_list varArgs;
    int result = 0;

    va_start( varArgs, format );

    result = Any_vsnprintf( str, size, format, varArgs );

    va_end( varArgs );

    return result;
}


/* EOF */
