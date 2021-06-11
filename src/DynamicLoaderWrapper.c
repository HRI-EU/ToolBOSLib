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


/*
 * The functions below become real wrappers of the -ldl library once
 * the -Wl,--wrap,<symbol> is defined. Once we do so, then the gnu linker
 * will automatically maps the <symbol> into __wrap_<symbol> giving us
 * the possibility to intercept globally any reference to the given <symbol>.
 *
 * Neverthless, the gnu linker give us the possibility to reference the
 * real <symbol> by calling the function __real_<symbol> instead. Such
 * __real_<symbol> will be translated to <symbol>, so the real wrapped
 * function
 */

#include <Any.h>


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

void *__wrap_dlopen( const char *filename, int flag )
{
    ANY_LOG( 0, "A call to dlopen('%s') has been detected, please check your static code", ANY_LOG_WARNING, filename );
    return NULL;  /* always unable to open the library */
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


char *__wrap_dlerror( void )
{
    ANY_LOG( 0, "A call to dlerror() has been detected, please check your static code", ANY_LOG_WARNING );
    return NULL;  /* always returns no errors */
}


void *__wrap_dlsym( void *handle, const char *symbol )
{
    ANY_LOG( 0, "A call to dlsym(%p, '%s') has been detected, please check your static code", ANY_LOG_WARNING,
             handle, symbol );
    return NULL;  /* always returns no symbol found */
}


int __wrap_dlclose( void *handle )
{
    ANY_LOG( 0, "A call to dlclose(%p) has been detected, please check your static code", ANY_LOG_WARNING, handle );
    return 0;  /* always success */
}


/* EOF */
