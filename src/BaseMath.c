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


#include <math.h>

#include <Any.h>
#include <BaseMath.h>


#ifndef __USE_MISC
#define __USE_MISC
#endif

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#ifndef __USE_GNU
#define __USE_GNU
#endif


/*-------------------------------------------------------------------------*/
/* Geometric operations                                                    */
/*-------------------------------------------------------------------------*/


BaseBool Base2DSize_isPointInside( const Base2DSize *self, const Base2DPoint *point )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( point );

    return ( point->x >= 0 ) &&
           ( point->y >= 0 ) &&
           ( point->x < self->width ) &&
           ( point->y < self->height );
}


BaseBool Base2DRect_isPointInside( const Base2DRect *self, const Base2DPoint *point )
{
    Base2DPoint np;

    ANY_REQUIRE( self );
    ANY_REQUIRE( point );

    np.x = point->x - self->upperLeft.x;
    np.y = point->y - self->upperLeft.y;

    return Base2DSize_isPointInside( &self->size, &np );
}


Base2DPoint *Base2DPoint_clipToSize( Base2DPoint *self, const Base2DPoint *src,
                                     const Base2DSize *size )
{
    ANY_REQUIRE( Base2DSize_isValid( size ));

    self->x = BASE_MATH_MAX( src->x, 0 );
    self->x = BASE_MATH_MIN( self->x, size->width - 1 );
    self->y = BASE_MATH_MAX( src->y, 0 );
    self->y = BASE_MATH_MIN( self->y, size->height - 1 );

    return self;
}


Base2DPoint *Base2DPoint_clipToSize_I( Base2DPoint *self,
                                       const Base2DSize *size )
{
    ANY_REQUIRE( Base2DSize_isValid( size ));

    self->x = BASE_MATH_MAX( self->x, 0 );
    self->x = BASE_MATH_MIN( self->x, size->width - 1 );
    self->y = BASE_MATH_MAX( self->y, 0 );
    self->y = BASE_MATH_MIN( self->y, size->height - 1 );

    return self;
}


Base2DPoint *Base2DPoint_clipToRect( Base2DPoint *self, const Base2DPoint *src,
                                     const Base2DRect *rect )
{
    ANY_REQUIRE( Base2DRect_isValid( rect ));

    self->x = BASE_MATH_MAX( src->x, rect->upperLeft.x );
    self->x = BASE_MATH_MIN( self->x, rect->upperLeft.x + rect->size.width - 1 );
    self->y = BASE_MATH_MAX( src->y, rect->upperLeft.y );
    self->y = BASE_MATH_MIN( self->y, rect->upperLeft.y + rect->size.height - 1 );

    return self;
}


Base2DPoint *Base2DPoint_clipToRect_I( Base2DPoint *self,
                                       const Base2DRect *rect )
{
    ANY_REQUIRE( Base2DRect_isValid( rect ));

    self->x = BASE_MATH_MAX( self->x, rect->upperLeft.x );
    self->x = BASE_MATH_MIN( self->x, rect->upperLeft.x + rect->size.width - 1 );
    self->y = BASE_MATH_MAX( self->y, rect->upperLeft.y );
    self->y = BASE_MATH_MIN( self->y, rect->upperLeft.y + rect->size.height - 1 );

    return self;
}


BaseBool Base2DSize_isValid( const Base2DSize *self )
{
    return ( self->width > 0 && self->height > 0 );
}


BaseBool Base2DSize_isSizeInside( const Base2DSize *self, const Base2DSize *size )
{
    return ( size->width <= self->width && size->height <= self->height );
}


BaseBool Base2DSize_isRectInside( const Base2DSize *self, const Base2DRect *rect )
{
    return ( rect->upperLeft.x >= 0 && rect->upperLeft.y >= 0 &&
             rect->upperLeft.x + rect->size.width <= self->width &&
             rect->upperLeft.y + rect->size.height <= self->height );
}


Base2DSize *Base2DSize_setInvalid( Base2DSize *self )
{
    self->width = self->height = 0;

    return self;
}


Base2DSize *Base2DSize_or( Base2DSize *self, const Base2DSize *src,
                           const Base2DSize *src2 )
{
    self->width = BASE_MATH_MAX( src->width, src2->width );
    self->height = BASE_MATH_MAX( src->height, src2->height );

    return self;
}


Base2DSize *Base2DSize_or_I( Base2DSize *self, const Base2DSize *src )
{
    self->width = BASE_MATH_MAX( self->width, src->width );
    self->height = BASE_MATH_MAX( self->height, src->height );

    return self;
}


Base2DSize *Base2DSize_and( Base2DSize *self, const Base2DSize *src,
                            const Base2DSize *src2 )
{
    self->width = BASE_MATH_MIN( src->width, src2->width );
    self->height = BASE_MATH_MIN( src->height, src2->height );

    return self;
}


Base2DSize *Base2DSize_and_I( Base2DSize *self, const Base2DSize *src )
{
    self->width = BASE_MATH_MIN( self->width, src->width );
    self->height = BASE_MATH_MIN( self->height, src->height );

    return self;
}


Base2DSize *Base2DSize_scale( Base2DSize *self, const Base2DSize *src,
                              const BaseF64 scale )
{
    if( Base2DSize_isValid( src ))
    {
        self->width = (BaseI32)BASE_MATH_ROUND( src->width * scale );
        self->height = (BaseI32)BASE_MATH_ROUND( src->height * scale );
    }
    else
    {
        *self = *src;
    }

    return self;
}


Base2DSize *Base2DSize_scale_I( Base2DSize *self, const BaseF64 scale )
{
    if( Base2DSize_isValid( self ))
    {
        self->width = (BaseI32)BASE_MATH_ROUND( self->width * scale );
        self->height = (BaseI32)BASE_MATH_ROUND( self->height * scale );
    }

    return self;
}


BaseBool Base2DRect_isValid( const Base2DRect *self )
{
    return Base2DSize_isValid( &self->size );
}


BaseBool Base2DRect_isRectInside( const Base2DRect *self, const Base2DRect *rect )
{
    return ( rect->upperLeft.x >= self->upperLeft.x &&
             rect->upperLeft.y >= self->upperLeft.y &&
             rect->upperLeft.x + rect->size.width <=
             self->upperLeft.x + self->size.width &&
             rect->upperLeft.y + rect->size.height <=
             self->upperLeft.y + self->size.height );
}


Base2DRect *Base2DRect_setInvalid( Base2DRect *self )
{
    Base2DSize_setInvalid( &self->size );

    return self;
}


Base2DPoint *Base2DRect_getCenterPoint( const Base2DRect *self,
                                        Base2DPoint *point )
{
    ANY_REQUIRE( Base2DRect_isValid( self ));

    point->x = self->upperLeft.x + BASE_MATH_CENTER( self->size.width );
    point->y = self->upperLeft.y + BASE_MATH_CENTER( self->size.height );

    return point;
}


Base2DRect *Base2DRect_or( Base2DRect *self, const Base2DRect *src,
                           const Base2DRect *src2 )
{
    if( Base2DRect_isValid( src ))
    {
        if( Base2DRect_isValid( src2 ))
        {
            self->upperLeft.x = BASE_MATH_MIN( src->upperLeft.x, src2->upperLeft.x );
            self->upperLeft.y = BASE_MATH_MIN( src->upperLeft.y, src2->upperLeft.y );
            self->size.width =
                    BASE_MATH_MAX( src->upperLeft.x + src->size.width,
                                   src2->upperLeft.x + src2->size.width ) - self->upperLeft.x;
            self->size.height =
                    BASE_MATH_MAX( src->upperLeft.y + src->size.height,
                                   src2->upperLeft.y + src2->size.height ) - self->upperLeft.y;
        }
        else
        {
            *self = *src;
        }
    }
    else
    {
        *self = *src2;
    }

    return self;
}


Base2DRect *Base2DRect_or_I( Base2DRect *self, const Base2DRect *src )
{
    if( Base2DRect_isValid( src ))
    {
        if( Base2DRect_isValid( self ))
        {
            Base2DPoint origUpperLeft = self->upperLeft;
            self->upperLeft.x = BASE_MATH_MIN( self->upperLeft.x, src->upperLeft.x );
            self->upperLeft.y = BASE_MATH_MIN( self->upperLeft.y, src->upperLeft.y );
            self->size.width =
                    BASE_MATH_MAX( origUpperLeft.x + self->size.width,
                                   src->upperLeft.x + src->size.width ) - self->upperLeft.x;
            self->size.height =
                    BASE_MATH_MAX( origUpperLeft.y + self->size.height,
                                   src->upperLeft.y + src->size.height ) - self->upperLeft.y;
        }
        else
        {
            *self = *src;
        }
    }

    return self;
}


Base2DRect *Base2DRect_and( Base2DRect *self, const Base2DRect *src,
                            const Base2DRect *src2 )
{
    if( Base2DRect_isValid( src ) && Base2DRect_isValid( src2 ))
    {
        self->upperLeft.x = BASE_MATH_MAX( src->upperLeft.x, src2->upperLeft.x );
        self->upperLeft.y = BASE_MATH_MAX( src->upperLeft.y, src2->upperLeft.y );
        self->size.width =
                BASE_MATH_MIN( src->upperLeft.x + src->size.width,
                               src2->upperLeft.x + src2->size.width ) - self->upperLeft.x;
        self->size.height =
                BASE_MATH_MIN( src->upperLeft.y + src->size.height,
                               src2->upperLeft.y + src2->size.height ) - self->upperLeft.y;
    }
    else
    {
        Base2DRect_setInvalid( self );
    }

    return self;
}


Base2DRect *Base2DRect_and_I( Base2DRect *self, const Base2DRect *src )
{
    if( Base2DRect_isValid( self ))
    {
        if( Base2DRect_isValid( src ))
        {
            Base2DPoint origUpperLeft = self->upperLeft;
            self->upperLeft.x = BASE_MATH_MAX( self->upperLeft.x, src->upperLeft.x );
            self->upperLeft.y = BASE_MATH_MAX( self->upperLeft.y, src->upperLeft.y );
            self->size.width =
                    BASE_MATH_MIN( origUpperLeft.x + self->size.width,
                                   src->upperLeft.x + src->size.width ) - self->upperLeft.x;
            self->size.height =
                    BASE_MATH_MIN( origUpperLeft.y + self->size.height,
                                   src->upperLeft.y + src->size.height ) - self->upperLeft.y;
        }
        else
        {
            *self = *src;
        }
    }

    return self;
}


Base2DRect *Base2DRect_scale( Base2DRect *self, const Base2DRect *src,
                              const BaseF64 scale )
{
    if( Base2DRect_isValid( src ))
    {
        self->upperLeft.x = (BaseI32)BASE_MATH_ROUND( src->upperLeft.x * scale );
        self->upperLeft.y = (BaseI32)BASE_MATH_ROUND( src->upperLeft.y * scale );
        self->size.width = (BaseI32)BASE_MATH_ROUND( src->size.width * scale );
        self->size.height = (BaseI32)BASE_MATH_ROUND( src->size.height * scale );
    }
    else
    {
        *self = *src;
    }

    return self;
}


Base2DRect *Base2DRect_scale_I( Base2DRect *self, const BaseF64 scale )
{
    if( Base2DRect_isValid( self ))
    {
        self->upperLeft.x = (BaseI32)BASE_MATH_ROUND( self->upperLeft.x * scale );
        self->upperLeft.y = (BaseI32)BASE_MATH_ROUND( self->upperLeft.y * scale );
        self->size.width = (BaseI32)BASE_MATH_ROUND( self->size.width * scale );
        self->size.height = (BaseI32)BASE_MATH_ROUND( self->size.height * scale );
    }

    return self;
}


Base2DRect *Base2DRect_scaleCenterWidth(
        Base2DRect *self, const Base2DRect *src, const BaseF64 scale )
{
    if( Base2DRect_isValid( src ))
    {
        self->size.width = (BaseI32)BASE_MATH_ROUND( src->size.width * scale );
        self->upperLeft.x = src->upperLeft.x + BASE_MATH_CENTER( src->size.width - self->size.width );
        self->upperLeft.y = src->upperLeft.y;
        self->size.height = src->size.height;
    }
    else
    {
        *self = *src;
    }

    return self;
}


Base2DRect *Base2DRect_scaleCenterWidth_I( Base2DRect *self,
                                           const BaseF64 scale )
{
    if( Base2DRect_isValid( self ))
    {
        BaseI32 orgWidth = self->size.width;
        self->size.width = (BaseI32)BASE_MATH_ROUND( self->size.width * scale );
        self->upperLeft.x += BASE_MATH_CENTER( orgWidth - self->size.width );
    }

    return self;
}


Base2DRect *Base2DRect_scaleCenterHeight(
        Base2DRect *self, const Base2DRect *src, const BaseF64 scale )
{
    if( Base2DRect_isValid( src ))
    {
        self->size.height = (BaseI32)BASE_MATH_ROUND( src->size.height * scale );
        self->upperLeft.y = src->upperLeft.y + BASE_MATH_CENTER( src->size.height - self->size.height );
        self->upperLeft.x = src->upperLeft.x;
        self->size.width = src->size.width;
    }
    else
    {
        *self = *src;
    }

    return self;
}


Base2DRect *Base2DRect_scaleCenterHeight_I( Base2DRect *self,
                                            const BaseF64 scale )
{
    if( Base2DRect_isValid( self ))
    {
        BaseI32 orgHeight = self->size.height;
        self->size.height = (BaseI32)BASE_MATH_ROUND( self->size.height * scale );
        self->upperLeft.y += BASE_MATH_CENTER( orgHeight - self->size.height );
    }

    return self;
}


Base2DRect *Base2DRect_scaleCenter( Base2DRect *self, const Base2DRect *src,
                                    const BaseF64 scale )
{
    if( Base2DRect_isValid( src ))
    {
        self->size.width = (BaseI32)BASE_MATH_ROUND( src->size.width * scale );
        self->size.height = (BaseI32)BASE_MATH_ROUND( src->size.height * scale );
        self->upperLeft.x = src->upperLeft.x + BASE_MATH_CENTER( src->size.width - self->size.width );
        self->upperLeft.y = src->upperLeft.y + BASE_MATH_CENTER( src->size.height - self->size.height );
    }
    else
    {
        *self = *src;
    }

    return self;
}


Base2DRect *Base2DRect_scaleCenter_I( Base2DRect *self, const BaseF64 scale )
{
    if( Base2DRect_isValid( self ))
    {
        Base2DSize orgSize = self->size;
        self->size.width = (BaseI32)BASE_MATH_ROUND( self->size.width * scale );
        self->size.height = (BaseI32)BASE_MATH_ROUND( self->size.height * scale );
        self->upperLeft.x += BASE_MATH_CENTER( orgSize.width - self->size.width );
        self->upperLeft.y += BASE_MATH_CENTER( orgSize.height - self->size.height );
    }

    return self;
}


Base2DRect *Base2DRect_clipToSize( Base2DRect *self, const Base2DRect *src,
                                   const Base2DSize *size )
{
    if( Base2DRect_isValid( src ) && Base2DSize_isValid( size ))
    {
        self->upperLeft.x = BASE_MATH_MAX( 0, src->upperLeft.x );
        self->upperLeft.y = BASE_MATH_MAX( 0, src->upperLeft.y );
        self->size.width = BASE_MATH_MIN( src->upperLeft.x + src->size.width,
                                          size->width ) - self->upperLeft.x;
        self->size.height = BASE_MATH_MIN( src->upperLeft.y + src->size.height,
                                           size->height ) - self->upperLeft.y;
    }
    else
    {
        Base2DRect_setInvalid( self );
    }

    return self;
}


Base2DRect *Base2DRect_clipToSize_I( Base2DRect *self, const Base2DSize *size )
{
    if( Base2DRect_isValid( self ) && Base2DSize_isValid( size ))
    {
        Base2DPoint orgUpperLeft = self->upperLeft;
        self->upperLeft.x = BASE_MATH_MAX( self->upperLeft.x, 0 );
        self->upperLeft.y = BASE_MATH_MAX( self->upperLeft.y, 0 );
        self->size.width = BASE_MATH_MIN( orgUpperLeft.x + self->size.width,
                                          size->width ) - self->upperLeft.x;
        self->size.height = BASE_MATH_MIN( orgUpperLeft.y + self->size.height,
                                           size->height ) - self->upperLeft.y;
    }
    else
    {
        Base2DRect_setInvalid( self );
    }

    return self;
}


/*-------------------------------------------------------------------------*/
/* Byte-order conversion                                                   */
/*-------------------------------------------------------------------------*/

BaseF32 BaseF32_flipEndian( BaseF32 a )
{
    union
    {
        BaseI32 i;
        BaseF32 f;
    } m;

    m.f = a;
    m.i = BASEI32_FLIPENDIAN( m.i );

    return m.f;
}


BaseF64 BaseF64_flipEndian( BaseF64 a )
{
    union
    {
        BaseI64 i;
        BaseF64 f;
    } m;

    m.f = a;
    m.i = BASEI64_FLIPENDIAN( m.i );

    return m.f;
}


/* EOF */
