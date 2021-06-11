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


#include <Any.h>

#if defined( __windows__ ) || defined(__macos__)

void *Any_memmem( const void *buf, int buflen, const void *pattern, int len )
{
  char *bf = (char *)buf, *pt = (char *)pattern, *p = bf;

  while ( len <= ( buflen - (p - bf) ) )
  {
    if ( (p = Any_memchr( p, (int)(*pt), buflen - (p - bf))) != NULL )
    {
      if ( Any_memcmp( p, pattern, len ) == 0 )
      {
        return p;
      }
      else
      {
        p++;
      }
    }
    else
    {
      break;
    }
  }

  return NULL;
}

#endif

#if defined( __windows__ )

char *Any_strsep( char **string, const char *delimiters )
{
  char *retVal = NULL;
  char *p = NULL;

  if ( string )
  {
    retVal = *string;

    if( !retVal )
    {
      return( retVal );
    }

    p = strpbrk( retVal, delimiters );

    if( !p )
    {
      *string = NULL;
      return( retVal );
    }

    *p++ = '\0';

    *string = p;
 }

 return( retVal );
}

int Any_vsscanf( const char *str, const char *format, va_list argList )
{
  return ( Any_sscanf( str, format, *(void**)argList ) );
}

char *Any_strdup( char *str )
{
  int len = 0;
  char *retVal = NULL;

  ANY_REQUIRE_MSG( str, "The string to duplicate cannot be NULL" );

  len = Any_strlen( str ) + 1;

  retVal = ANY_BALLOC( len );

  if ( retVal )
  {
    Any_memcpy( retVal, str, len );
  }

  return retVal;
}

#endif /* __windows__ */

