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


#include <AnyLog.h>
#include <AnyMem.h>


#if !defined(__windows__)


void *Any_memAllocAlign( size_t size, size_t alignment )
{
    void *pointer;
    int retVal;

    retVal = posix_memalign( &pointer, alignment, size );

    if( retVal == 0 )
    {
        return pointer;
    }
    else
    {
        if( retVal == EINVAL )
        {
#if defined(__64BIT__)
            ANY_LOG( 0, "Alignment %ld is too small or not a power of 2",
                     ANY_LOG_FATAL, alignment );
#else
            ANY_LOG( 0, "Alignment %d is too small or not a power of 2",
                     ANY_LOG_FATAL, alignment );
#endif
        }

        return NULL;
    }
}


#endif   /* __windows__ */


/* EOF */
