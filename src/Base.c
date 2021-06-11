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


/*---------------------------------------------*/
/* Includes                                    */
/*---------------------------------------------*/

#include <Any.h>
#include <BaseTypes.h>


/*---------------------------------------------*/
/* Public Functions                            */
/*---------------------------------------------*/

BaseBool *BaseBool_new( void )
{
    BaseBool *self = (BaseBool *)NULL;

    self = ANY_TALLOC( BaseBool );
    ANY_REQUIRE_MSG( self, "BaseBool_new(): memory allocation failed" );

    return self;
}


void BaseBool_init( BaseBool *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseBool_clear( BaseBool *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseBool_delete( BaseBool *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseI8 *BaseI8_new( void )
{
    BaseI8 *self = (BaseI8 *)NULL;

    self = ANY_TALLOC( BaseI8 );
    ANY_REQUIRE_MSG( self, "BaseI8_new(): memory allocation failed" );

    return self;
}


void BaseI8_init( BaseI8 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI8_clear( BaseI8 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI8_delete( BaseI8 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseUI8 *BaseUI8_new( void )
{
    BaseUI8 *self = (BaseUI8 *)NULL;

    self = ANY_TALLOC( BaseUI8 );
    ANY_REQUIRE_MSG( self, "BaseUI8_new(): memory allocation failed" );

    return self;
}


void BaseUI8_init( BaseUI8 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI8_clear( BaseUI8 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI8_delete( BaseUI8 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseI16 *BaseI16_new( void )
{
    BaseI16 *self = (BaseI16 *)NULL;

    self = ANY_TALLOC( BaseI16 );
    ANY_REQUIRE_MSG( self, "BaseI16_new(): memory allocation failed" );

    return self;
}


void BaseI16_init( BaseI16 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI16_clear( BaseI16 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI16_delete( BaseI16 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseUI16 *BaseUI16_new( void )
{
    BaseUI16 *self = (BaseUI16 *)NULL;

    self = ANY_TALLOC( BaseUI16 );
    ANY_REQUIRE_MSG( self, "BaseUI16_new(): memory allocation failed" );

    return self;
}


void BaseUI16_init( BaseUI16 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI16_clear( BaseUI16 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI16_delete( BaseUI16 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseI32 *BaseI32_new( void )
{
    BaseI32 *self = (BaseI32 *)NULL;

    self = ANY_TALLOC( BaseI32 );
    ANY_REQUIRE_MSG( self, "BaseI32_new(): memory allocation failed" );

    return self;
}


void BaseI32_init( BaseI32 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI32_clear( BaseI32 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI32_delete( BaseI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseUI32 *BaseUI32_new( void )
{
    BaseUI32 *self = (BaseUI32 *)NULL;

    self = ANY_TALLOC( BaseUI32 );
    ANY_REQUIRE_MSG( self, "BaseUI32_new(): memory allocation failed" );

    return self;
}


void BaseUI32_init( BaseUI32 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI32_clear( BaseUI32 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI32_delete( BaseUI32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseF32 *BaseF32_new( void )
{
    BaseF32 *self = (BaseF32 *)NULL;

    self = ANY_TALLOC( BaseF32 );
    ANY_REQUIRE_MSG( self, "BaseF32_new(): memory allocation failed" );

    return self;
}


void BaseF32_init( BaseF32 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseF32_clear( BaseF32 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseF32_delete( BaseF32 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseF64 *BaseF64_new( void )
{
    BaseF64 *self = (BaseF64 *)NULL;

    self = ANY_TALLOC( BaseF64 );
    ANY_REQUIRE_MSG( self, "BaseF64_new(): memory allocation failed" );

    return self;
}


void BaseF64_init( BaseF64 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseF64_clear( BaseF64 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseF64_delete( BaseF64 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseI64 *BaseI64_new( void )
{
    BaseI64 *self = (BaseI64 *)NULL;

    self = ANY_TALLOC( BaseI64 );
    ANY_REQUIRE_MSG( self, "BaseI64_new(): memory allocation failed" );

    return self;
}


void BaseI64_init( BaseI64 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI64_clear( BaseI64 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseI64_delete( BaseI64 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


BaseUI64 *BaseUI64_new( void )
{
    BaseUI64 *self = (BaseUI64 *)NULL;

    self = ANY_TALLOC( BaseUI64 );
    ANY_REQUIRE_MSG( self, "BaseUI64_new(): memory allocation failed" );

    return self;
}


void BaseUI64_init( BaseUI64 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI64_clear( BaseUI64 *self )
{
    ANY_REQUIRE( self );

    *( self ) = 0;
}


void BaseUI64_delete( BaseUI64 *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}


/* EOF */
