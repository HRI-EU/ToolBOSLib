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


#include <BBDM-C.h>
#include <DynamicLoader.h>


#define BBDMPROPERTIES_CHECK_EQUALITY( __field )                             \
do                                                                           \
{                                                                            \
  if( src->__field != self->__field )                                        \
  {                                                                          \
    ANY_LOG( 5,                                                              \
             "self->" ANY_MACRO_STRING( __field )"=%d, src->"ANY_MACRO_STRING( __field )"=%d",\
             ANY_LOG_DATA,                                                   \
             self->__field,                                                  \
             src->__field );                                                 \
                                                                             \
    return false;                                                            \
  }                                                                          \
}                                                                            \
while( 0 )


static BBDMFunction BBDM_getBBDMFunction( BBDM *self, char *functionName );


const char *BBDM_getTypeName( const void *self )
{
    ANY_REQUIRE( self );

    return BBDM_GET_TYPENAME_FIELD( self );
}


const char *BBDM_getInstanceName( const void *self )
{
    BBDMGetInstanceNameFunc getInstanceNameFunc = (BBDMGetInstanceNameFunc)NULL;

    ANY_REQUIRE( self );

    getInstanceNameFunc = (BBDMGetInstanceNameFunc)BBDM_getBBDMFunction((BBDM *)self, "getInstanceName" );
    ANY_REQUIRE( getInstanceNameFunc );

    return getInstanceNameFunc( self );
}


void BBDM_setInstanceName( void *self, const char *instanceName )
{
    BBDMSetInstanceNameFunc setInstanceNameFunc = (BBDMSetInstanceNameFunc)NULL;

    ANY_REQUIRE( self );

    setInstanceNameFunc = (BBDMSetInstanceNameFunc)BBDM_getBBDMFunction((BBDM *)self, "setInstanceName" );
    ANY_REQUIRE( setInstanceNameFunc );

    setInstanceNameFunc( self, instanceName );
}


BaseI64 BBDM_getTimestep( const void *self )
{
    BBDMGetTimestepFunc getTimestepFunc = (BBDMGetTimestepFunc)NULL;

    ANY_REQUIRE( self );

    getTimestepFunc = (BBDMGetTimestepFunc)BBDM_getBBDMFunction((BBDM *)self, "getTimestep" );
    ANY_REQUIRE( getTimestepFunc );

    return getTimestepFunc( self );
}


void BBDM_setTimestep( void *self, BaseI64 timestep )
{
    BBDMSetTimestepFunc setTimestepFunc = (BBDMSetTimestepFunc)NULL;

    ANY_REQUIRE( self );

    setTimestepFunc = (BBDMSetTimestepFunc)BBDM_getBBDMFunction((BBDM *)self, "setTimestep" );
    ANY_REQUIRE( setTimestepFunc );

    setTimestepFunc( self, timestep );
}


void *BBDM_getData( void *self )
{
    BBDMGetDataFunc getDataFunc = (BBDMGetDataFunc)NULL;

    ANY_REQUIRE( self );

    getDataFunc = (BBDMGetDataFunc)BBDM_getBBDMFunction((BBDM *)self, "getData" );
    ANY_REQUIRE( getDataFunc );

    return getDataFunc( self );
}


int BBDM_copyData( void *self, const void *src )
{
    BBDMCopyDataFunc copyDataFunc = (BBDMCopyDataFunc)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    copyDataFunc = (BBDMCopyDataFunc)BBDM_getBBDMFunction( (BBDM *)self, "indirectCopyData" );
    ANY_REQUIRE( copyDataFunc );

    return copyDataFunc( self, src );
}


void BBDM_getProperties( const void *self, BBDMProperties *p )
{
    BBDMGetPropertiesFunc getPropertiesFunc = (BBDMGetPropertiesFunc)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( p );

    getPropertiesFunc = (BBDMGetPropertiesFunc)BBDM_getBBDMFunction((BBDM *)self, "getProperties" );
    ANY_REQUIRE_MSG( getPropertiesFunc,
                     "Function _indirectGetProperties not implemented for this BBDM" );

    getPropertiesFunc( self, p );
}


void BBDM_serialize( void *self, const char *name, void *stream )
{
    SerializeFunction serializeFunc = (SerializeFunction)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( stream );

    serializeFunc = BBDM_getSerializeFunctionPtr((BBDM *)self );
    ANY_REQUIRE( serializeFunc );

    serializeFunc( self, name, stream );
}


SerializeFunction BBDM_getSerializeFunctionPtr( void *self )
{
    ANY_REQUIRE( self );

    return (SerializeFunction)BBDM_getBBDMFunction((BBDM *)self, "serialize" );
}


const char *BBDM_getDataTypeName( const void *self )
{
    char *bbdmType = (char *)NULL;
    char *bbdmPrefix = "BBDM";
    unsigned int bbdmPrefixLen = 0;

    ANY_REQUIRE( self );

    bbdmType = BBDM_GET_TYPENAME_FIELD( self );
    ANY_REQUIRE( bbdmType );

    bbdmPrefixLen = Any_strlen( bbdmPrefix );

    ANY_REQUIRE_VMSG(( Any_strncmp( bbdmType, bbdmPrefix, bbdmPrefixLen ) == 0 ) &&
                     ( Any_strlen( bbdmType ) > bbdmPrefixLen ),
                     "BBDM_getDataType(): BBDM typename '%s' does not start with '%s'",
                     bbdmType,
                     bbdmPrefix );

    /* By convention, currently all data components are named "BBDMxyz" where
     * xyz is the placeholder for the datatype ("payload type") which is
     * wrapped inside. So, as a current rule we can assume that the datatype
     * is starting at position 5 in the string of the BBDM typename. */

    return ( bbdmType + 4 * sizeof( char ));
}


SerializeFunction BBDM_getDataSerializeFunctionPtr( void *self )
{
    ANY_REQUIRE( self );

    return (SerializeFunction)DynamicLoader_getSymbolByClassAndMethodName((DynamicLoader *)NULL,
                                                                          BBDM_getDataTypeName( self ),
                                                                          "serialize" );
}


BaseBool BBDMProperties_isEQ( const BBDMProperties *self,
                              const BBDMProperties *src )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( src );

    /* these macros "return false" if self->... != src->... */
    BBDMPROPERTIES_CHECK_EQUALITY( width );
    BBDMPROPERTIES_CHECK_EQUALITY( height );
    BBDMPROPERTIES_CHECK_EQUALITY( length );
    BBDMPROPERTIES_CHECK_EQUALITY( maxNoSparseEntries );
    BBDMPROPERTIES_CHECK_EQUALITY( size1 );
    BBDMPROPERTIES_CHECK_EQUALITY( size2 );
    BBDMPROPERTIES_CHECK_EQUALITY( size3 );
    BBDMPROPERTIES_CHECK_EQUALITY( size4 );
    BBDMPROPERTIES_CHECK_EQUALITY( totalSize );
    BBDMPROPERTIES_CHECK_EQUALITY( type.scalar );
    BBDMPROPERTIES_CHECK_EQUALITY( type.compound );
    BBDMPROPERTIES_CHECK_EQUALITY( type.bplType );
    BBDMPROPERTIES_CHECK_EQUALITY( type.bplArray );
    BBDMPROPERTIES_CHECK_EQUALITY( type.bplBlock );
    BBDMPROPERTIES_CHECK_EQUALITY( type.memType );
    BBDMPROPERTIES_CHECK_EQUALITY( id );

    return true;
}


static BBDMFunction BBDM_getBBDMFunction( BBDM *self, char *functionName )
{
    ANY_REQUIRE( self );

    return (BBDMFunction)DynamicLoader_getSymbolByClassAndMethodName((DynamicLoader *)NULL,
                                                                     BBDM_getTypeName( self ),
                                                                     functionName );
}


/* EOF */
