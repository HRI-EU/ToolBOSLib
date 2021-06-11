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


/* some API parameters unused but kept for polymorphism */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include <limits.h>

#include <Serialize.h>


/*--------------------------------------------------------------------------*/
/* ASCII format                                                             */
/*--------------------------------------------------------------------------*/


SERIALIZEFORMAT_CREATE_PLUGIN( Ascii );


#define SERIALIZE_DATABUFFER_MAXLEN  (1024 + SERIALIZE_TYPEMAXTEXTLEN_STRING)
#define SERIALIZE_TAGNBUFFER_MAXLEN                                      (32)
#define SERIALIZE_SPECBUFFER_MAXLEN                                      (32)


typedef struct SerializeFormatAsciiOptions
{
    bool withType;
}
        SerializeFormatAsciiOptions;


static bool SerializeFormatAscii_isWithType( Serialize *self );

static void SerializeFormatAscii_getTypeInfo( SerializeType type,
                                              char *spec,
                                              char *typeTag );

static void SerializeFormatAscii_doSerializeCharType( Serialize *self,
                                                      SerializeType type,
                                                      const char *name,
                                                      void *value,
                                                      const int size,
                                                      const int len,
                                                      const int index );

static void SerializeFormatAscii_doSerializeField( Serialize *self,
                                                   SerializeType type,
                                                   const char *name,
                                                   void *value,
                                                   const int size );

static void SerializeFormatAscii_doSerializeString( Serialize *self,
                                                    SerializeType type,
                                                    const char *name,
                                                    void *value,
                                                    const int size,
                                                    const int len );

static void SerializeFormatAscii_doSerializeArrayElement( Serialize *self,
                                                          SerializeType type,
                                                          const char *name,
                                                          void *value,
                                                          const int size,
                                                          const int len,
                                                          const int index,
                                                          bool reIndexOffset );


static void SerializeFormatAscii_beginType( Serialize *self,
                                            const char *name,
                                            const char *type )
{
    char buffer[SERIALIZE_HEADER_MAXLEN];
    long len = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            char instanceName[512];

            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                len = Any_snprintf( buffer, SERIALIZE_HEADER_MAXLEN, "struct %s %%s = { ", type );
            }
            else
            {
                Any_strncpy( buffer, "%s = { ", SERIALIZE_HEADER_MAXLEN );
                len = Any_strlen( buffer );
            }

            /* Check that snprintf wrote the string we required. */
            if( len < 0 )
            {
                ANY_LOG( 0, "Error while writing the type.", ANY_LOG_ERROR );
                break;
            }
            else if( len >= SERIALIZE_HEADER_MAXLEN)
            {
                ANY_LOG( 0, "Warning: type string was truncated.", ANY_LOG_WARNING );
            }

            /* Only pattern matching. */
            Serialize_scanf( self, buffer, instanceName );

            if( self->indentLevel != SERIALIZE_INDENTLEVEL )
            {
                if( Any_strcmp( name, instanceName ) != 0 )
                {
                    ANY_LOG( 0, "Expected instance name '%s' different than '%s'", ANY_LOG_WARNING,
                             name, instanceName );
                }
            }
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );

            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Serialize_printf( self, "struct %s %s =\n", type, name );
            }
            else
            {
                Serialize_printf( self, "%s =\n", name );
            }

            SERIALIZE_INDENT( self );
            Serialize_printf( self, "{\n" );
            SERIALIZE_INDENT_INCR( self );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_beginBaseType( Serialize *self,
                                                const char *name,
                                                const char *type )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            /* SERIALIZE_INDENT_INCR( self ); */
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_beginArray( Serialize *self,
                                             SerializeType type,
                                             const char *arrayName,
                                             const int arrayLen )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );
    ANY_REQUIRE( arrayLen > 0 );

    SerializeFormatAscii_getTypeInfo( type, spec, typeTag );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " [] = " ),
                              "%s %s[%d] = ", typeTag, arrayName, arrayLen );
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "[] = " ),
                              "%s[%d] = ", arrayName, arrayLen );
            }
            /* Only Pattern Mathing.. */
            Serialize_scanf( self, buffer );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );

            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Serialize_printf( self, "%s %s[", typeTag, arrayName );
            }
            else
            {
                Serialize_printf( self, "%s[", arrayName );
            }

            Serialize_deployDataType( self,
                                      SERIALIZE_TYPE_INT,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      "%d", 0, 0, (void *)&arrayLen );

            Serialize_printf( self, "] =" );

            SERIALIZE_INDENT_INCR( self );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_beginStructArray( Serialize *self,
                                                   const char *arrayName,
                                                   const char *elementType,
                                                   const int arrayLen )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );
    ANY_REQUIRE( elementType );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " [] = { " ),
                              "%s %s[%d] = { ", elementType, arrayName, arrayLen );
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "[] = { " ),
                              "%s[%d] = { ", arrayName, arrayLen );
            }
            /* Only Pattern Matching... */
            Serialize_scanf( self, buffer );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );

            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Serialize_printf( self, "%s %s[", elementType, arrayName );
            }
            else
            {
                Serialize_printf( self, "%s[", arrayName );
            }

            Serialize_deployDataType( self,
                                      SERIALIZE_TYPE_INT,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      "%d", 0, 0, (void *)&arrayLen );

            Serialize_printf( self, "] = \n" );

            SERIALIZE_INDENT( self );

            Serialize_printf( self, "{\n", arrayName );

            SERIALIZE_INDENT_INCR( self );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_beginStructArraySeparator( Serialize *self,
                                                            const char *name,
                                                            const int position,
                                                            const int len )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatAscii_doSerialize( Serialize *self,
                                              SerializeType type,
                                              const char *name,
                                              void *value,
                                              const int size,
                                              const int len )
{
    int i = 0;
    bool isCharType = false;
    bool isField = false;
    bool isString = false;
    bool isArrayElement = false;


    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    isCharType = ((( type == SERIALIZE_TYPE_CHAR ) ||
                   ( type == SERIALIZE_TYPE_UCHAR ) ||
                   ( type == SERIALIZE_TYPE_SCHAR ) ||
                   ( type == SERIALIZE_TYPE_CHARARRAY ) ||
                   ( type == SERIALIZE_TYPE_UCHARARRAY ) ||
                   ( type == SERIALIZE_TYPE_SCHARARRAY )) ? true : false );

    //isField = ( ( len == 1 ) ? true : false );
    isString = (( type == SERIALIZE_TYPE_STRING ) ? true : false );
    //isArrayElement = ( ( len > 1 ) ? true : false );
    isArrayElement = SERIALIZE_IS_ARRAY_ELEMENT( type );
    isField = !isArrayElement;

    for( ; ; )
    {
        /* Wraps Standard Chars */
        if( isCharType == true )
        {
            for( i = 0; i < len; i++ )
            {
                SerializeFormatAscii_doSerializeCharType( self, type, name, value, size, len, i );
            }
            break;
        }

        if( isField == true )
        {
            SerializeFormatAscii_doSerializeField( self, type, name, value, size );
            break;
        }

        if( isString == true )
        {
            SerializeFormatAscii_doSerializeString( self, type, name, value, size, len );
            break;
        }

        if( isArrayElement == true )
        {
            for( i = 0; i < len; i++ )
            {
                SerializeFormatAscii_doSerializeArrayElement( self, type, name, value, size, len, i, true );
            }
            break;
        }

        break;
    }
}


static void SerializeFormatAscii_endStructArraySeparator( Serialize *self,
                                                          const char *name,
                                                          const int position,
                                                          const int len )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatAscii_endStructArray( Serialize *self )
{
    ANY_REQUIRE( self );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "} " );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "}\n" );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_endArray( Serialize *self,
                                           SerializeType type,
                                           const char *arrayName,
                                           const int arrayLen )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );
    ANY_REQUIRE( arrayLen > 0 );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT_DECR( self );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_endBaseType( Serialize *self )
{
    ANY_REQUIRE( self );

    /* End type MUST match also the newline */
    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            /* SERIALIZE_INDENT_DECR( self ); */
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_endType( Serialize *self )
{
    ANY_REQUIRE( self );

    /* End type MUST match also the newline */
    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "}\n" );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "}\n" );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static int SerializeFormatAscii_getAllowedModes( Serialize *self )
{
    int modes = SERIALIZE_MODE_CALC;

    ANY_REQUIRE( self );

    return modes;
}


void *SerializeFormatAsciiOptions_new( void )
{
    SerializeFormatAsciiOptions *data = (SerializeFormatAsciiOptions *)NULL;

    data = ANY_TALLOC( SerializeFormatAsciiOptions );
    ANY_REQUIRE( data );

    return (void *)data;
}


void SerializeFormatAsciiOptions_init( Serialize *self )
{
    SerializeFormatAsciiOptions *data = (SerializeFormatAsciiOptions *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatAsciiOptions *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    /* Default init options */
    data->withType = false;
}


void SerializeFormatAsciiOptions_set( Serialize *self, const char *optionsString )
{
    SerializeFormatAsciiOptions *data = (SerializeFormatAsciiOptions *)NULL;
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char buffer[SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE] = "";
    char localStr[SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE] = "";

    ANY_REQUIRE( self );

    data = (SerializeFormatAsciiOptions *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    /* Default Uses No Type */
    data->withType = false;

    if( optionsString != (char *)NULL)
    {
        Any_sscanf( optionsString, "WITH_TYPE=%2047s", buffer );

        if( Any_strcmp( buffer, "TRUE" ) == 0 )
        {
            data->withType = true;
        }
        else
        {
            data->withType = false;
        }
    }

    Any_snprintf( localStr, SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE - sizeof( "WITH_TYPE=" ),
                  "WITH_TYPE=%s", ( data->withType == true ? "TRUE" : "FALSE" ));

    rvp = SerializeReferenceValue_findReferenceValue( self->header->listHead, "opts" );
    if( !rvp )
    {
        /* There was no RVP for reference 'opts' in the list, we try to
     * look for one in the cache */
        rvp = SerializeReferenceValue_pop( &self->header->poolHead );
        if( !rvp )
        {
            /* TODO: if we are in this branch, we could not find an RVP in
       * the list and we couldn't pop a new RVP instance from the
       * cache. Can we create a new one instead of quitting? */
            ANY_LOG( 0, "ERROR: Could not set option \"%s\". Setting error.", ANY_LOG_ERROR, localStr );
            self->errorOccurred = true;
        }

        /* Append the new rvp we just popped from the cache in the list */
        SerializeReferenceValue_push( &self->header->listHead, rvp );
    }

    /* Finally, store the new option string in the RVP */
    SerializeReferenceValue_update( rvp, "opts", localStr );
}


static bool SerializeFormatAsciiOptions_setProperty( Serialize *self,
                                                     const char *propertyName,
                                                     void *propertyValue )
{
    bool retVal = false;

    ANY_REQUIRE( self );

    return retVal;
}


static void *SerializeFormatAsciiOptions_getProperty( Serialize *self,
                                                      const char *propertyName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );

    return retVal;
}


void SerializeFormatAsciiOptions_clear( Serialize *self )
{
    SerializeFormatAsciiOptions *data = (SerializeFormatAsciiOptions *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatAsciiOptions *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    Any_memset((void *)data, 0, sizeof( SerializeFormatAsciiOptions ));
}


void SerializeFormatAsciiOptions_delete( Serialize *self )
{
    SerializeFormatAsciiOptions *data = (SerializeFormatAsciiOptions *)NULL;

    ANY_REQUIRE( self );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    ANY_FREE( data );
}


static bool SerializeFormatAscii_isWithType( Serialize *self )
{
    SerializeFormatAsciiOptions *ptr = (SerializeFormatAsciiOptions *)NULL;
    bool retVal = false;

    ANY_REQUIRE( self );

    ptr = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( ptr );

    retVal = ptr->withType;

    return retVal;
}


static void SerializeFormatAscii_getTypeInfo( SerializeType type,
                                              char *spec,
                                              char *typeTag )
{
#define TYPEINFO( __specPtr, __spec, __typeTagPtr, __typeTag )  \
  Any_sprintf( __specPtr, "%s", __spec );                       \
  Any_sprintf( __typeTagPtr, "%s", __typeTag )

    switch( type )
    {
        case SERIALIZE_TYPE_CHAR:
        case SERIALIZE_TYPE_CHARARRAY:
        TYPEINFO( spec, "%qc", typeTag, "char" );
            break;

        case SERIALIZE_TYPE_SCHAR:
        case SERIALIZE_TYPE_SCHARARRAY:
        TYPEINFO( spec, "%d", typeTag, "signed_char" );
            break;

        case SERIALIZE_TYPE_UCHAR:
        case SERIALIZE_TYPE_UCHARARRAY:
        TYPEINFO( spec, "%u", typeTag, "unsigned_char" );
            break;

        case SERIALIZE_TYPE_SINT:
        case SERIALIZE_TYPE_SINTARRAY:
        TYPEINFO( spec, "%hd", typeTag, "short_int" );
            break;

        case SERIALIZE_TYPE_USINT:
        case SERIALIZE_TYPE_USINTARRAY:
        TYPEINFO( spec, "%hu", typeTag, "short_unsigned" );
            break;

        case SERIALIZE_TYPE_INT:
        case SERIALIZE_TYPE_INTARRAY:
        TYPEINFO( spec, "%d", typeTag, "int" );
            break;

        case SERIALIZE_TYPE_UINT:
        case SERIALIZE_TYPE_UINTARRAY:
        TYPEINFO( spec, "%u", typeTag, "unsigned_int" );
            break;

        case SERIALIZE_TYPE_LINT:
        case SERIALIZE_TYPE_LINTARRAY:
        TYPEINFO( spec, "%ld", typeTag, "long_int" );
            break;

        case SERIALIZE_TYPE_ULINT:
        case SERIALIZE_TYPE_ULINTARRAY:
        TYPEINFO( spec, "%lu", typeTag, "long_unsigned_int" );
            break;

        case SERIALIZE_TYPE_LL:
        case SERIALIZE_TYPE_LLARRAY:
        TYPEINFO( spec, "%lld", typeTag, "long_long" );
            break;

        case SERIALIZE_TYPE_ULL:
        case SERIALIZE_TYPE_ULLARRAY:
        TYPEINFO( spec, "%llu", typeTag, "long_long_unsigned" );
            break;

        case SERIALIZE_TYPE_FLOAT:
        case SERIALIZE_TYPE_FLOATARRAY:
        TYPEINFO( spec, "%f", typeTag, "float" );
            break;

        case SERIALIZE_TYPE_DOUBLE:
        case SERIALIZE_TYPE_DOUBLEARRAY:
        TYPEINFO( spec, "%lf", typeTag, "double" );
            break;

        case SERIALIZE_TYPE_LDOUBLE:
        case SERIALIZE_TYPE_LDOUBLEARRAY:
        TYPEINFO( spec, "%LF", typeTag, "long_double" );
            break;

        case SERIALIZE_TYPE_STRING :
            Any_sprintf( spec, "%s", "%qs" );
            Any_sprintf( typeTag, "%s", "string" );
            break;
        default:
            ANY_LOG( 0, "SerializeFormatAscii_getTypeInfo. Unknown SerializeType : %d",
                     ANY_LOG_ERROR, type );
            ANY_REQUIRE( NULL );
            break;
    }

#undef TYPEINFO
}


static void SerializeFormatAscii_doSerializeCharType( Serialize *self,
                                                      SerializeType type,
                                                      const char *name,
                                                      void *value,
                                                      const int size,
                                                      const int len,
                                                      const int index )
{
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char *signedCharPtr = (char *)NULL;
    unsigned char *unsignedCharPtr = (unsigned char *)NULL;
    int auxData = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    SerializeFormatAscii_getTypeInfo( type, spec, typeTag );

    if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR ||
        type == SERIALIZE_TYPE_CHARARRAY || type == SERIALIZE_TYPE_SCHARARRAY )
    {
        signedCharPtr = value;
        ANY_REQUIRE( signedCharPtr );
        signedCharPtr += size * index;
        ANY_REQUIRE( signedCharPtr );

        auxData = ((int)*signedCharPtr );
    }
    else
    {
        unsignedCharPtr = value;
        ANY_REQUIRE( unsignedCharPtr );
        unsignedCharPtr += size * index;
        ANY_REQUIRE( unsignedCharPtr );

        auxData = ((int)*unsignedCharPtr );
    }

    if( SERIALIZE_IS_ARRAY_ELEMENT( type ) == false )
    {
        SerializeFormatAscii_doSerializeField( self, type, name, &auxData, size );

        if( self->mode == SERIALIZE_MODE_READ )
        {
            if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR )
            {
                ANY_REQUIRE( auxData <= SCHAR_MAX );
                ANY_REQUIRE( auxData >= SCHAR_MIN );
                ANY_REQUIRE( signedCharPtr );
                *signedCharPtr = (char)auxData;
            }
            else
            {
                ANY_REQUIRE( auxData <= UCHAR_MAX );
                ANY_REQUIRE( unsignedCharPtr );
                *unsignedCharPtr = (unsigned char)auxData;
            }
        }
    }
    else
    {
        SerializeFormatAscii_doSerializeArrayElement( self, type, name, &auxData, size, len, index, false );

        if( self->mode == SERIALIZE_MODE_READ )
        {
            if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR ||
                type == SERIALIZE_TYPE_CHARARRAY || type == SERIALIZE_TYPE_SCHARARRAY )
            {
                ANY_REQUIRE( auxData <= SCHAR_MAX );
                ANY_REQUIRE( auxData >= SCHAR_MIN );
                ANY_REQUIRE( signedCharPtr );
                *signedCharPtr = (char)auxData;
            }
            else
            {
                ANY_REQUIRE( auxData <= UCHAR_MAX );
                ANY_REQUIRE( unsignedCharPtr );
                *unsignedCharPtr = (unsigned char)auxData;
            }
        }
    }
}


static void SerializeFormatAscii_doSerializeField( Serialize *self,
                                                   SerializeType type,
                                                   const char *name,
                                                   void *value,
                                                   const int size )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    SerializeFormatAscii_getTypeInfo( type, spec, typeTag );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "  = ;" ),
                              "%s %s = %s;", typeTag, name, spec );
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " = ;" ),
                              "%s = %s;", name, spec );
            }
            Serialize_scanf( self, buffer, value );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );
            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Serialize_printf( self, "%s %s = ", typeTag, name );
            }
            else
            {
                Serialize_printf( self, "%s = ", name );
            }

            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, value );

            Serialize_printf( self, ";\n" );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_doSerializeString( Serialize *self,
                                                    SerializeType type,
                                                    const char *name,
                                                    void *value,
                                                    const int size,
                                                    const int len )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN + SERIALIZE_TYPEMAXTEXTLEN_STRING];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    SerializeFormatAscii_getTypeInfo( type, spec, typeTag );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " [] = ; " ),
                              "%s %s[%d] = %s; ", typeTag, name, len, spec );
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "[] = ; " ),
                              "%s[%d] = %s; ", name, len, spec );
            }
            Serialize_scanf( self, buffer, value );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );
            if( SerializeFormatAscii_isWithType( self ) == true )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " [] = ;\n" ),
                              "%s %s[%d] = %s;\n", typeTag, name, len, spec );
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "[] = ;\n" ),
                              "%s[%d] = %s;\n", name, len, spec );
            }
            Serialize_printf( self, buffer, value );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatAscii_doSerializeArrayElement( Serialize *self,
                                                          SerializeType type,
                                                          const char *name,
                                                          void *value,
                                                          const int size,
                                                          const int len,
                                                          const int index,
                                                          bool reIndexOffset )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char *ptr = (char *)NULL;

    bool isLastArrayElement = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );
    ANY_REQUIRE( len > 0 );

    SerializeFormatAscii_getTypeInfo( type, spec, typeTag );

    isLastArrayElement = (( index == ( len - 1 )) ? true : false );

    if( reIndexOffset == true )
    {
        ptr = value;
        ptr += size * index;
        ANY_REQUIRE( ptr );
    }
    else
    {
        ptr = value;
    }

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( isLastArrayElement )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - 2,   // 1 for the semicolon + '\0'
                              "%s;", spec );
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - 2,   // 1 for the blank + '\0'
                              "%s ", spec );
            }
            Serialize_scanf( self, buffer, (void *)ptr );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            if(( self->columnWrap > 0 ) && ( index % self->columnWrap ) == 0 )
            {
                Serialize_printf( self, "\n" );
                SERIALIZE_INDENT( self );
            }

            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, (void *)ptr );

            if( isLastArrayElement )
            {
                Serialize_printf( self, ";\n" );
            }
            else
            {
                Serialize_printf( self, " " );
            }
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


/*--------------------------------------------------------------------------*/
/* Binary format                                                            */
/*--------------------------------------------------------------------------*/


SERIALIZEFORMAT_CREATE_PLUGIN( Binary );


typedef struct SerializeFormatBinaryEndianFlag
{
    bool isLittleEndian;
} SerializeFormatBinaryEndianFlag;


#define SERIALIZEFORMATBINARY_LOCALDEBUGLEVEL 10


/* This is used to check how the endianness option of the plugin is set */
/* Another way to see this is: T if plugin endianness != system endianness, F otherwhise */
/* The macro is pretty much straightforward since the Binary format allows only one option*/
#define SERIALIZEFORMAT_CHECKENDIANNESS( __self )           \
(*((bool*)__self->format->data) != __self->isLittleEndian )

#define Serialize_deploy( __self, __value, __size )         \
Serialize_deployDataType( __self,                           \
                          (SerializeType)NULL,              \
                          SERIALIZE_DEPLOYDATAMODE_BINARY,  \
                          (char*)NULL,                      \
                          0, __size, __value )

#define SERIALIZE_GENERICTYPE( __self, __value, __type, __len )         \
{                                                                       \
  if( Serialize_isReading( __self ) == true )                           \
  {                                                                     \
    if( Serialize_deploy( __self, __value, sizeof( __type ) * __len ) == true  ) \
    {                                                                   \
      if( SERIALIZEFORMAT_CHECKENDIANNESS( __self ) )                   \
      {                                                                 \
        SerializeFormatBinary_swapBuffer( __value, sizeof( __type ), __len ); \
      }                                                                 \
    }                                                                   \
  }                                                                     \
  if( Serialize_isWriting( __self ) == true )                           \
  {                                                                     \
    if( SERIALIZEFORMAT_CHECKENDIANNESS( __self ) )                     \
    {                                                                   \
      int     __i = 0;                                                  \
      __type  __tmp;                                                    \
      __type* __tmpPtr = (__type*) __value;                             \
                                                                        \
      for( ; __i < (int)__len; __i++)                                   \
      {                                                                 \
        __tmp = ( __type ) __tmpPtr[__i];                               \
        SerializeFormatBinary_swapBuffer( &__tmp, sizeof( __type ), 1 ); \
        Serialize_deploy( __self, &__tmp, sizeof( __type ) );           \
      }                                                                 \
    }                                                                   \
    else                                                                \
    {                                                                   \
      Serialize_deploy( __self, __value, sizeof( __type ) * __len ); \
    }                                                                   \
  }                                                                     \
}


#define SERIALIZEFORMAT_TYPE_BEGIN( __type )    \
switch( __type )

#define SERIALIZEFORMAT_TYPE( __name )          \
case SERIALIZE_TYPE_##__name:

#define SERIALIZEFORMAT_TYPE_UNKNOWN \
default :\
ANY_LOG( 5, "------ Unknown Serialization Type -----", ANY_LOG_ERROR );\
ANY_REQUIRE( 1 == 0 );\

#define SERIALIZEFORMAT_TYPE_END


static void SerializeFormatBinary_swapBuffer( void *value,
                                              unsigned int size,
                                              unsigned int len );


static void SerializeFormatBinary_beginType( Serialize *self,
                                             const char *name,
                                             const char *type )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );
}


static void SerializeFormatBinary_beginBaseType( Serialize *self,
                                                 const char *name,
                                                 const char *type )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    SerializeFormatBinary_beginType( self, name, type );
}


static void SerializeFormatBinary_beginArray( Serialize *self,
                                              SerializeType type,
                                              const char *name,
                                              const int size )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatBinary_beginStructArray( Serialize *self,
                                                    const char *name,
                                                    const char *type,
                                                    const int size )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );
}


static void SerializeFormatBinary_beginStructArraySeparator( Serialize *self,
                                                             const char *name,
                                                             const int pos,
                                                             const int len )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatBinary_doSerialize( Serialize *self,
                                               SerializeType type,
                                               const char *name,
                                               void *value,
                                               const int size,
                                               const int len )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    SERIALIZEFORMAT_TYPE_BEGIN( type )
    {
        SERIALIZEFORMAT_TYPE( CHAR )
        SERIALIZEFORMAT_TYPE( CHARARRAY )
        SERIALIZEFORMAT_TYPE( SCHAR )
        SERIALIZEFORMAT_TYPE( SCHARARRAY )
        SERIALIZEFORMAT_TYPE( UCHAR )
        SERIALIZEFORMAT_TYPE( UCHARARRAY )
            Serialize_deploy( self, value, size * len );
            break;

        SERIALIZEFORMAT_TYPE( STRING )
        {
            unsigned short int slen = size * len;

            ANY_REQUIRE( slen < SERIALIZE_TYPEMAXTEXTLEN_STRING );

            /*
             * string serialization uses a short int (2 bytes) to store string length
             * before to serialize the string itself.
             *
             * The string might be 0 length
             */
            SERIALIZE_GENERICTYPE( self, &slen, unsigned short int, 1);

            /* only if the string size is > 0 we read the string */
            if( slen > 0 )
            {
                Serialize_deploy( self, value, slen );


                /* when reading terminate the string buffer before to return */
                if( Serialize_isReading( self ) == true )
                {
                    ((char *)value )[ slen - 1 ] = '\0';
                }
            }
        }
            break;

        SERIALIZEFORMAT_TYPE( SINT )
        SERIALIZEFORMAT_TYPE( SINTARRAY )
        SERIALIZE_GENERICTYPE( self, value, short int, len );
            break;

        SERIALIZEFORMAT_TYPE( USINT )
        SERIALIZEFORMAT_TYPE( USINTARRAY )
        SERIALIZE_GENERICTYPE( self, value, unsigned short int, len );
            break;

        SERIALIZEFORMAT_TYPE( INT )
        SERIALIZEFORMAT_TYPE( INTARRAY )
        SERIALIZE_GENERICTYPE( self, value, int, len );
            break;

        SERIALIZEFORMAT_TYPE( UINT )
        SERIALIZEFORMAT_TYPE( UINTARRAY )
        SERIALIZE_GENERICTYPE( self, value, unsigned int, len );
            break;

        SERIALIZEFORMAT_TYPE( LINT )
        SERIALIZEFORMAT_TYPE( LINTARRAY )
        SERIALIZE_GENERICTYPE( self, value, long int, len );
            break;

        SERIALIZEFORMAT_TYPE( ULINT )
        SERIALIZEFORMAT_TYPE( ULINTARRAY )
        SERIALIZE_GENERICTYPE( self, value, unsigned long int, len );
            break;

        SERIALIZEFORMAT_TYPE( LL )
        SERIALIZEFORMAT_TYPE( LLARRAY )
        SERIALIZE_GENERICTYPE( self, value, long long, len );
            break;

        SERIALIZEFORMAT_TYPE( ULL )
        SERIALIZEFORMAT_TYPE( ULLARRAY )
        SERIALIZE_GENERICTYPE( self, value, unsigned long long, len );
            break;

        SERIALIZEFORMAT_TYPE( FLOAT )
        SERIALIZEFORMAT_TYPE( FLOATARRAY )
        SERIALIZE_GENERICTYPE( self, value, float, len );
            break;

        SERIALIZEFORMAT_TYPE( DOUBLE )
        SERIALIZEFORMAT_TYPE( DOUBLEARRAY )
        SERIALIZE_GENERICTYPE( self, value, double, len );
            break;

        SERIALIZEFORMAT_TYPE( LDOUBLE )
        SERIALIZEFORMAT_TYPE( LDOUBLEARRAY )
            ANY_LOG( 5, " Long type not supported yet", ANY_LOG_INFO );
            break;

        SERIALIZEFORMAT_TYPE_UNKNOWN
            break;
    }
    SERIALIZEFORMAT_TYPE_END
}


static void SerializeFormatBinary_endStructArraySeparator( Serialize *self,
                                                           const char *name,
                                                           const int pos,
                                                           const int len )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatBinary_endStructArray( Serialize *self )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatBinary_endArray( Serialize *self,
                                            SerializeType type,
                                            const char *name,
                                            const int size )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatBinary_endBaseType( Serialize *self )
{
    ANY_REQUIRE( self );

    SerializeFormatBinary_endType( self );
}


static void SerializeFormatBinary_endType( Serialize *self )
{
    ANY_REQUIRE( self );
}


static int SerializeFormatBinary_getAllowedModes( Serialize *self )
{
    int modes = SERIALIZE_MODE_CALC;

    ANY_REQUIRE( self );

    return modes;
}


static void *SerializeFormatBinaryOptions_new( void )
{
    SerializeFormatBinaryEndianFlag *self = (SerializeFormatBinaryEndianFlag *)NULL;

    self = ANY_TALLOC( SerializeFormatBinaryEndianFlag );
    ANY_REQUIRE( self );

    return (void *)self;
}


static void SerializeFormatBinaryOptions_init( Serialize *self )
{
    SerializeFormatBinaryEndianFlag *data = (SerializeFormatBinaryEndianFlag *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatBinaryEndianFlag *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    data->isLittleEndian = Serialize_isLittleEndian();
}


static void SerializeFormatBinaryOptions_set( Serialize *self,
                                              const char *optionsString )
{
    SerializeFormatBinaryEndianFlag *data = (SerializeFormatBinaryEndianFlag *)NULL;
    char *optionsPtr = (char *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatBinaryEndianFlag *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    /* Set serialized data endianness flag */
    if( optionsString != (char *)NULL)
    {
        if( Any_strcmp( optionsString, "LITTLE_ENDIAN" ) == 0 )
        {
            ANY_LOG( SERIALIZEFORMATBINARY_LOCALDEBUGLEVEL, "Setting endianness to little endian.", ANY_LOG_INFO );
            data->isLittleEndian = true;
        } else if( Any_strcmp( optionsString, "BIG_ENDIAN" ) == 0 )
        {
            ANY_LOG( SERIALIZEFORMATBINARY_LOCALDEBUGLEVEL, "Setting endianness to big endian.", ANY_LOG_INFO );
            data->isLittleEndian = false;
        } else
        {
            ANY_LOG( SERIALIZEFORMATBINARY_LOCALDEBUGLEVEL,
                     "Defaults if option not set: machine endianness if writing, big endian if reading", ANY_LOG_INFO );
            if( Serialize_isWriting( self ) == true )
            {
                data->isLittleEndian = Serialize_isLittleEndian();
            }
            else
            {
                data->isLittleEndian = false;
            }
        }
    }
    else
    {
        /* No valid endianness specified: assuming big endian, since this was the old behaviour */
        ANY_LOG( SERIALIZEFORMATBINARY_LOCALDEBUGLEVEL, "Defaulting to big endian", ANY_LOG_INFO );
        data->isLittleEndian = false;
    }

    optionsPtr = Serialize_getHeaderOptsPtr( self );
    ANY_REQUIRE( optionsPtr );
    /*NOTE: changed from self->isLittleEndian to data->isLittleEndian */
    Any_sprintf( optionsPtr, "%s_ENDIAN", ( data->isLittleEndian ? "LITTLE" : "BIG" ));
}


static bool SerializeFormatBinaryOptions_setProperty( Serialize *self,
                                                      const char *optName,
                                                      void *optValue )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    /* No Option to Set */

    return retVal;
}


static void *SerializeFormatBinaryOptions_getProperty( Serialize *self,
                                                       const char *optName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    /* No Option to Get */

    return retVal;
}


static void SerializeFormatBinaryOptions_clear( Serialize *self )
{
    SerializeFormatBinaryEndianFlag *data = (SerializeFormatBinaryEndianFlag *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatBinaryEndianFlag *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    Any_memset((void *)data, 0, sizeof( SerializeFormatBinaryEndianFlag ));
}


static void SerializeFormatBinaryOptions_delete( Serialize *self )
{
    SerializeFormatBinaryEndianFlag *data = (SerializeFormatBinaryEndianFlag *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatBinaryEndianFlag *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    ANY_FREE( data );
}


static void SerializeFormatBinary_swapBuffer( void *value,
                                              unsigned int size,
                                              unsigned int len )
{
    unsigned int i, j, k, l;
    register char tmp;
    register char *ptr = (char *)value;

    l = ( size - 1 );
    for( k = 0; k < len; k++ )
    {
        for( i = 0, j = l; i < j; i++, j-- )
        {
            tmp = ptr[ i ];
            ptr[ i ] = ptr[ j ];
            ptr[ j ] = tmp;
        }
        ptr += size;
    }
}


#undef SERIALIZEFORMATBINARY_LOCALDEBUGLEVEL


/*--------------------------------------------------------------------------*/
/* Matlab format                                                            */
/*--------------------------------------------------------------------------*/


SERIALIZEFORMAT_CREATE_PLUGIN( Matlab );


#define SERIALIZE_STRING_MAXLEN               1024
#define SERIALIZE_PREFIX_MAXLEN            (2*1024)
#define SERIALIZE_STRUCTURE_MAXNESTING         (32)



/* FIXME the macro actually does not check cast limits..  */
/* __minValue and __maxValue are ignored */
#define SERIALIZECASTFROMDOUBLE_TYPE( __self, __type, __valuePtr, __len, __minValue, __maxValue )\
  do\
  {                                           \
    __type *__ptr = (__type*)__valuePtr;      \
    double __item = 0;                        \
    int __i = 0;                              \
                                              \
    ANY_REQUIRE( __self->mode == SERIALIZE_MODE_READ );\
                                              \
    for( __i = 0; __i < __len ; __i++ )       \
    {                                         \
      Serialize_deployDataType( __self,\
                                SERIALIZE_TYPE_DOUBLE,\
                                SERIALIZE_DEPLOYDATAMODE_ASCII,\
                                "%lf", 0, 0, &__item );\
\
      /* Check limits */                      \
      *__ptr = (__type)__item;                \
      __ptr++;                                \
    }                                         \
  } while( 0 )

#define SERIALIZEPRINT_TYPE( __self, __serializeType, __type, __spec, __valuePtr, __len )\
  do\
  {                                           \
    int __i = 0;                              \
    __type *__ptr = (__type*)__valuePtr;      \
                                              \
    ANY_REQUIRE( __self->mode != SERIALIZE_MODE_READ );\
                                              \
    for( __i = 0; __i < __len ; __i++ )       \
    {                                         \
      Serialize_deployDataType( __self,\
                                __serializeType,\
                                SERIALIZE_DEPLOYDATAMODE_ASCII,\
                                __spec, 0, 0, __ptr );\
      __ptr++;                                \
    }                                         \
  } while( 0 )


typedef struct SerializeFormatMatlabOptions
{
    bool isArrayOfStructElement[SERIALIZE_STRUCTURE_MAXNESTING];
    char prefixBuffer[SERIALIZE_PREFIX_MAXLEN];
    int structNestingLevel;
    int nestingLevel;
    int prefixIndex;
}
        SerializeFormatMatlabOptions;


static void SerializeFormatMatlab_appendPrefix( Serialize *self,
                                                const char *name,
                                                bool dotAlso );

static void SerializeFormatMatlab_removePrefix( Serialize *self );

static void Serialize_formatTypeToFormatString( Serialize *self,
                                                SerializeType type,
                                                char *formatStr,
                                                char *typeNameBuff );


static void SerializeFormatMatlab_beginType( Serialize *self,
                                             const char *name,
                                             const char *type )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    if( opt->isArrayOfStructElement[ opt->structNestingLevel ] == false )
    {
        SerializeFormatMatlab_appendPrefix( self, name, true );
    }

    opt->structNestingLevel++;
    ANY_REQUIRE( opt->structNestingLevel >= 0 );
    ANY_REQUIRE_MSG( opt->structNestingLevel < SERIALIZE_STRUCTURE_MAXNESTING,
                     "Too Structure nesting levels!" );

    opt->isArrayOfStructElement[ opt->structNestingLevel ] = false;
}


static void SerializeFormatMatlab_beginBaseType( Serialize *self,
                                                 const char *name,
                                                 const char *type )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    /*SerializeFormatMatlab_beginType( self, name, type );*/
}


static void SerializeFormatMatlab_beginArray( Serialize *self,
                                              SerializeType type,
                                              const char *arrayName,
                                              const int arrayLen )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatMatlab_doSerialize( Serialize *self,
                                               SerializeType type,
                                               const char *name,
                                               void *value,
                                               const int size,
                                               const int len )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char formatStr[5];
    bool isArray = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    //isArray = (( len > 1 ) ? true : false );
    isArray = SERIALIZE_IS_ARRAY_ELEMENT( type );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    Serialize_formatTypeToFormatString( self, type, formatStr, buffer );

    if( Serialize_isReading( self ) == true )
    {
        Serialize_scanf( self, opt->prefixBuffer );
    }
    else
    {
        Serialize_printf( self, opt->prefixBuffer );
    }

    if( type != SERIALIZE_TYPE_STRING )
    {
        if( isArray == true )
        {
            Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " = [ " ),
                          "%s = [ ", name );
        }
        else
        {
            Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " = " ),
                          "%s = ", name );
        }

        if( Serialize_isReading( self ) == true )
        {
            Serialize_scanf( self, buffer );
        }
        else
        {
            Serialize_printf( self, buffer );
        }
    }

    switch( type )
    {
        case SERIALIZE_TYPE_CHAR:
        case SERIALIZE_TYPE_SCHAR:
        case SERIALIZE_TYPE_CHARARRAY:
        case SERIALIZE_TYPE_SCHARARRAY:
            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, signed char, value, len, SCHAR_MIN, SCHAR_MAX );
            }
            else
            {
                signed char *ptr = (signed char *)value;
                signed int item = 0;
                int i = 0;

                for( i = 0; i < len; i++ )
                {
                    item = *ptr;
                    Serialize_printf( self, "%d ", &item );
                    ptr++;
                }
            }
            break;

        case SERIALIZE_TYPE_UCHAR:
        case SERIALIZE_TYPE_UCHARARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, signed char, value, len, 0, UCHAR_MAX );
            }
            else
            {
                unsigned char *ptr = (unsigned char *)value;
                unsigned int item = 0;
                int i = 0;

                for( i = 0; i < len; i++ )
                {
                    item = *ptr;
                    Serialize_printf( self, "%u ", &item );
                    ptr++;
                }
            }
            break;

        case SERIALIZE_TYPE_SINT:
        case SERIALIZE_TYPE_SINTARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, short int, value, len, SHRT_MIN, SHRT_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_SINT, short int, "%hd ", value, len );
            }
            break;

        case SERIALIZE_TYPE_USINT:
        case SERIALIZE_TYPE_USINTARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, unsigned short int, value, len, 0, USHRT_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_USINT, unsigned short int, "%hu ", value, len );
            }
            break;

        case SERIALIZE_TYPE_INT:
        case SERIALIZE_TYPE_INTARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, int, value, len, INT_MIN, INT_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_INT, int, "%d ", value, len );
            }
            break;

        case SERIALIZE_TYPE_UINT:
        case SERIALIZE_TYPE_UINTARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, unsigned int, value, len, 0, UINT_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_UINT, unsigned int, "%u ", value, len );
            }
            break;

        case SERIALIZE_TYPE_LINT:
        case SERIALIZE_TYPE_LINTARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, long int, value, len, LONG_MIN, LONG_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_LINT, long int, "%ld ", value, len );
            }
            break;

        case SERIALIZE_TYPE_ULINT:
        case SERIALIZE_TYPE_ULINTARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, unsigned long int, value, len, 0, ULONG_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_ULINT, unsigned long int, "%lu ", value, len );
            }
            break;

        case SERIALIZE_TYPE_LL:
        case SERIALIZE_TYPE_LLARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, long long int, value, len, LLONG_MIN, LLONG_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_LL, long long int, "%lld ", value, len );
            }
            break;

        case SERIALIZE_TYPE_ULL:
        case SERIALIZE_TYPE_ULLARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, unsigned long long int, value, len, 0, ULLONG_MAX );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_ULL, unsigned long long int, "%llu ", value, len );
            }
            break;


        case SERIALIZE_TYPE_FLOAT:
        case SERIALIZE_TYPE_FLOATARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, float, value, len, 0, 0 );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_FLOAT, float, "%f ", value, len );
            }
            break;

        case SERIALIZE_TYPE_DOUBLE:
        case SERIALIZE_TYPE_DOUBLEARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, double, value, len, 0, 0 );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_DOUBLE, double, "%lf ", value, len );
            }
            break;

        case SERIALIZE_TYPE_LDOUBLE:
        case SERIALIZE_TYPE_LDOUBLEARRAY:

            if( Serialize_isReading( self ) == true )
            {
                SERIALIZECASTFROMDOUBLE_TYPE( self, long double, value, len, 0, 0 );
            }
            else
            {
                SERIALIZEPRINT_TYPE( self, SERIALIZE_TYPE_LDOUBLE, long double, "%LF ", value, len );
            }
            break;

        case SERIALIZE_TYPE_STRING:

            if( Serialize_isReading( self ) == true )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " = ''; " ),
                              "%s = %s; ", name, "%qs" );
                Serialize_scanf( self, buffer, ((char *)value ));
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( " = '';\n" ),
                              "%s = '%s';\n", name, (char *)value );
                Serialize_printf( self, buffer );
            }
            break;

        default:
            ANY_LOG( 0, "Serialize_formatTypeToFormatString. Unknown SerializeType : %d",
                     ANY_LOG_ERROR, type );
            ANY_REQUIRE( NULL );
            break;
    }

    if( type != SERIALIZE_TYPE_STRING )
    {
        if( isArray == true )
        {
            Any_sprintf( buffer, "] ;\n" );
        }
        else
        {
            Any_sprintf( buffer, ";\n" );
        }

        if( Serialize_isReading( self ) == true )
        {
            Serialize_scanf( self, buffer );
        }
        else
        {
            Serialize_printf( self, buffer );
        }
    }
}


static void SerializeFormatMatlab_beginStructArray( Serialize *self,
                                                    const char *arrayName,
                                                    const char *elementType,
                                                    const int arrayLen )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );
    ANY_REQUIRE( elementType );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    opt->isArrayOfStructElement[ opt->structNestingLevel ] = true;
}


static void SerializeFormatMatlab_endStructArray( Serialize *self )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;

    ANY_REQUIRE( self );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    opt->isArrayOfStructElement[ opt->structNestingLevel ] = false;
}


static void SerializeFormatMatlab_beginStructArraySeparator( Serialize *self,
                                                             const char *name,
                                                             const int position,
                                                             const int len )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    bool isTheFirstElement = false;

    ANY_REQUIRE( self );

    isTheFirstElement = ( position == 0 ? true : false );

    /* Matlab array indeces must start from 1! */
    Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN, "%s(%d)", name, ( position + 1 ));

    if( isTheFirstElement == false )
    {
        SerializeFormatMatlab_removePrefix( self );
    }

    SerializeFormatMatlab_appendPrefix( self, buffer, true );
}


static void SerializeFormatMatlab_endStructArraySeparator( Serialize *self,
                                                           const char *name,
                                                           const int position,
                                                           const int len )
{
    bool isTheLastElement = false;

    ANY_REQUIRE( self );

    isTheLastElement = ( position == ( len - 1 ) ? true : false );

    if( isTheLastElement == true )
    {
        SerializeFormatMatlab_removePrefix( self );
    }
}


static void SerializeFormatMatlab_endArray( Serialize *self,
                                            SerializeType type,
                                            const char *arrayName,
                                            const int arrayLen )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatMatlab_endBaseType( Serialize *self )
{
    ANY_REQUIRE( self );

    /*SerializeFormatMatlab_endType( self );*/
}


static void SerializeFormatMatlab_endType( Serialize *self )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;

    ANY_REQUIRE( self );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    opt->structNestingLevel--;
    ANY_REQUIRE( opt->structNestingLevel >= 0 );

    if( opt->isArrayOfStructElement[ opt->structNestingLevel ] == false )
    {
        SerializeFormatMatlab_removePrefix( self );
    }
}


static int SerializeFormatMatlab_getAllowedModes( Serialize *self )
{
    int modes = SERIALIZE_MODE_CALC;

    ANY_REQUIRE( self );

    return modes;
}


static void *SerializeFormatMatlabOptions_new( void )
{
    SerializeFormatMatlabOptions *data = (SerializeFormatMatlabOptions *)NULL;

    data = ANY_TALLOC( SerializeFormatMatlabOptions );
    ANY_REQUIRE( data );

    return (void *)data;
}


static void SerializeFormatMatlabOptions_init( Serialize *self )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;
    int i = 0;

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    opt->prefixIndex = 0;
    opt->nestingLevel = 0;
    opt->structNestingLevel = 0;

    for( i = 0; i < SERIALIZE_STRUCTURE_MAXNESTING; i++ )
    {
        opt->isArrayOfStructElement[ i ] = false;
    }

    Any_memset( opt->prefixBuffer, '\0', SERIALIZE_PREFIX_MAXLEN );
}


static void SerializeFormatMatlabOptions_set( Serialize *self, const char *optionsString )
{
    ANY_REQUIRE( self );
}


static bool SerializeFormatMatlabOptions_setProperty( Serialize *self, const char *optName, void *optValue )
{
    bool retVal = false;

    ANY_REQUIRE( self );
    /* No Option to Set */

    return retVal;
}


static void *SerializeFormatMatlabOptions_getProperty( Serialize *self, const char *optName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );
    /* No Option to Get */

    return retVal;
}


static void SerializeFormatMatlabOptions_clear( Serialize *self )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;
    int i = 0;

    ANY_REQUIRE( self );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    opt->prefixIndex = 0;
    opt->nestingLevel = 0;
    opt->structNestingLevel = 0;

    for( i = 0; i < SERIALIZE_STRUCTURE_MAXNESTING; i++ )
    {
        opt->isArrayOfStructElement[ i ] = false;
    }

    Any_memset( opt->prefixBuffer, '\0', SERIALIZE_PREFIX_MAXLEN );
}


static void SerializeFormatMatlabOptions_delete( Serialize *self )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;

    ANY_REQUIRE( self );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    ANY_FREE( opt );
}


static void SerializeFormatMatlab_appendPrefix( Serialize *self,
                                                const char *name,
                                                bool dotAlso )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;
    char buff[256];
    int len = 0;
    char *ptr;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    len = Any_strlen( name );
    ANY_REQUIRE( len > 0 );

    ptr = opt->prefixBuffer + opt->prefixIndex;

    if( dotAlso == true )
    {
        len = Any_snprintf( buff, 254, "%s.", name );   // 1 for the dot + '\0'
    }
    else
    {
        len = Any_snprintf( buff, 255, "%s", name );
    }

    opt->prefixIndex += len;
    ANY_REQUIRE( opt->prefixIndex < SERIALIZE_PREFIX_MAXLEN );
    strcat( ptr, buff );

    opt->nestingLevel++;
}


static void SerializeFormatMatlab_removePrefix( Serialize *self )
{
    SerializeFormatMatlabOptions *opt = (SerializeFormatMatlabOptions *)NULL;
    char *ptr = (char *)NULL;

    ANY_REQUIRE( self );

    opt = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( opt );

    opt->nestingLevel--;
    ANY_REQUIRE( opt->nestingLevel >= 0 );

    if( opt->nestingLevel == 0 )
    {
        *opt->prefixBuffer = '\0';
        opt->prefixIndex = 0;
        if( Serialize_isReading( self ) == true )
        {
            Serialize_scanf( self, "\n" );
        }
        else
        {
            Serialize_printf( self, "\n" );
        }
    }
    else
    {
        ptr = opt->prefixBuffer + opt->prefixIndex;
        ANY_REQUIRE( ptr );
        ptr--;
        ANY_REQUIRE( ptr );
        ptr--;
        ANY_REQUIRE( ptr );

        while( *ptr != '.' )
        {
            ptr--;
            ANY_REQUIRE( ptr );
            opt->prefixIndex--;
            ANY_REQUIRE( ptr );
            ANY_REQUIRE( opt->prefixIndex >= 0 );
        }
        opt->prefixIndex--;
        ANY_REQUIRE( ptr );
        opt->prefixBuffer[ opt->prefixIndex ] = '\0';
    }
}


static void Serialize_formatTypeToFormatString( Serialize *self,
                                                SerializeType type,
                                                char *formatStr,
                                                char *typeNameBuff )
{
    switch( type )
    {
        case SERIALIZE_TYPE_CHAR:
        case SERIALIZE_TYPE_CHARARRAY:
            Any_snprintf( formatStr, 5, "%s", "%qc" );
            Any_snprintf( typeNameBuff, 32, "%s", "char" );
            break;

        case SERIALIZE_TYPE_SCHAR:
        case SERIALIZE_TYPE_SCHARARRAY:
            Any_snprintf( formatStr, 5, "%s", "%d" );
            Any_snprintf( typeNameBuff, 32, "%s", "signed_char" );
            break;

        case SERIALIZE_TYPE_UCHAR:
        case SERIALIZE_TYPE_UCHARARRAY:
            Any_snprintf( formatStr, 5, "%s", "%u" );
            Any_snprintf( typeNameBuff, 32, "%s", "unsigned_char" );
            break;

        case SERIALIZE_TYPE_SINT:
        case SERIALIZE_TYPE_SINTARRAY:
            Any_snprintf( formatStr, 5, "%s", "%hd" );
            Any_snprintf( typeNameBuff, 32, "%s", "short_int" );
            break;

        case SERIALIZE_TYPE_USINT:
        case SERIALIZE_TYPE_USINTARRAY:
            Any_snprintf( formatStr, 5, "%s", "%hu" );
            Any_snprintf( typeNameBuff, 32, "%s", "short_unsigned_int" );
            break;

        case SERIALIZE_TYPE_INT:
        case SERIALIZE_TYPE_INTARRAY:
            Any_snprintf( formatStr, 5, "%s", "%d" );
            Any_snprintf( typeNameBuff, 32, "%s", "int" );
            break;

        case SERIALIZE_TYPE_UINT:
        case SERIALIZE_TYPE_UINTARRAY:
            Any_snprintf( formatStr, 5, "%s", "%u" );
            Any_snprintf( typeNameBuff, 32, "%s", "unsigned_int" );
            break;

        case SERIALIZE_TYPE_LINT:
        case SERIALIZE_TYPE_LINTARRAY:
            Any_snprintf( formatStr, 5, "%s", "%ld" );
            Any_snprintf( typeNameBuff, 32, "%s", "long_int" );
            break;

        case SERIALIZE_TYPE_ULINT:
        case SERIALIZE_TYPE_ULINTARRAY:
            Any_snprintf( formatStr, 5, "%s", "%lu" );
            Any_snprintf( typeNameBuff, 32, "%s", "long_unsigned_int" );
            break;

        case SERIALIZE_TYPE_LL:
        case SERIALIZE_TYPE_LLARRAY:
            Any_snprintf( formatStr, 5, "%s", "%lld" );
            Any_snprintf( typeNameBuff, 32, "%s", "long_long_int" );
            break;

        case SERIALIZE_TYPE_ULL:
        case SERIALIZE_TYPE_ULLARRAY:
            Any_snprintf( formatStr, 5, "%s", "%llu" );
            Any_snprintf( typeNameBuff, 32, "%s", "unsigned_long_long_int" );
            break;

        case SERIALIZE_TYPE_FLOAT:
        case SERIALIZE_TYPE_FLOATARRAY:
            Any_snprintf( formatStr, 5, "%s", "%f" );
            Any_snprintf( typeNameBuff, 32, "%s", "float" );
            break;

        case SERIALIZE_TYPE_DOUBLE:
        case SERIALIZE_TYPE_DOUBLEARRAY:
            Any_snprintf( formatStr, 5, "%s", "%lf" );
            Any_snprintf( typeNameBuff, 32, "%s", "double" );
            break;

        case SERIALIZE_TYPE_LDOUBLE:
        case SERIALIZE_TYPE_LDOUBLEARRAY:
            Any_snprintf( formatStr, 5, "%s", "%LF" );
            Any_snprintf( typeNameBuff, 32, "%s", "long_double" );
            break;

        case SERIALIZE_TYPE_STRING:
            Any_snprintf( formatStr, 5, "%s", "%qs" );
            Any_snprintf( typeNameBuff, 32, "%s", "char*" );
            break;

        default:
            ANY_LOG( 5,
                     "Serialize_formatTypeToFormatString. Unknown SerializeType : %d",
                     ANY_LOG_ERROR, type );
            ANY_REQUIRE( NULL );
            break;
    }
}


#undef SERIALIZE_STRING_MAXLEN
#undef SERIALIZE_PREFIX_MAXLEN
#undef SERIALIZE_STRUCTURE_MAXNESTING


/*--------------------------------------------------------------------------*/
/* XML format                                                               */
/*--------------------------------------------------------------------------*/


SERIALIZEFORMAT_CREATE_PLUGIN( Xml );


typedef struct SerializeFormatXmlOptions
{
    bool baseTypeEnable;
}
        SerializeFormatXmlOptions;


static void SerializeFormatXml_getTypeInfo( SerializeType type,
                                            char *spec,
                                            char *typeName );

static void SerializeFormatXml_doSerializeCharType( Serialize *self,
                                                    SerializeType type,
                                                    const char *name,
                                                    void *value,
                                                    const int size,
                                                    const int len,
                                                    const int index );

static void SerializeFormatXml_doSerializeField( Serialize *self,
                                                 SerializeType type,
                                                 const char *name,
                                                 void *value,
                                                 const int size );

static void SerializeFormatXml_doSerializeString( Serialize *self,
                                                  SerializeType type,
                                                  const char *name,
                                                  void *value,
                                                  const int size,
                                                  const int len );

static void SerializeFormatXml_doSerializeArrayElement( Serialize *self,
                                                        SerializeType type,
                                                        const char *name,
                                                        void *value,
                                                        const int size,
                                                        const int len,
                                                        const int index,
                                                        bool reIndexOffset );

static bool SerializeFormatXml_doSerializeStringCharEscaping( Serialize *self, char *value );

static bool SerializeFormatXml_doSerializeStringCharUnescaping( Serialize *self, char *value );


static void SerializeFormatXml_beginType( Serialize *self,
                                          const char *name,
                                          const char *type )
{
    SerializeFormatXmlOptions *ptr = NULL;
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    ptr = (SerializeFormatXmlOptions *)Serialize_getFormatDataPtr( self );
    ptr->baseTypeEnable = self->baseTypeEnable;

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            char instanceName[256];

            Any_snprintf( buffer,
                          SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<struct type=\"\" name=\"%\"> " ),
                          "<struct type=\"%s\" name=\"%%s\"> ", type );

            Serialize_scanf( self, buffer, instanceName );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "<struct type=\"%s\" name=\"%s\">\n", type, name );
            SERIALIZE_INDENT_INCR( self );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_beginBaseType( Serialize *self,
                                              const char *name,
                                              const char *type )
{
    SerializeFormatXmlOptions *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    ptr = (SerializeFormatXmlOptions *)Serialize_getFormatDataPtr( self );
    ptr->baseTypeEnable = self->baseTypeEnable;

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            /* SERIALIZE_INDENT_INCR( self ); */
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static int SerializeFormatXml_getAllowedModes( Serialize *self )
{
    int modes = SERIALIZE_MODE_CALC;

    ANY_REQUIRE( self );

    return modes;
}


static void SerializeFormatXml_beginArray( Serialize *self,
                                           SerializeType type,
                                           const char *arrayName,
                                           const int arrayLen )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );

    SerializeFormatXml_getTypeInfo( type, spec, typeTag );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            /* Only Pattern Matching */
            Any_snprintf( buffer,
                          SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<array type=\"\" name=\"\" size=\"\"> " ),
                          "<array type=\"%s\" name=\"%s\" size=\"%d\"> ",
                          typeTag, arrayName, arrayLen );

            Serialize_scanf( self, buffer );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );

            Serialize_printf( self, "<array type=\"%s\" name=\"%s\" size=\"",
                              typeTag, arrayName );

            Serialize_deployDataType( self,
                                      SERIALIZE_TYPE_INT,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      "%d", 0, 0, (void *)&arrayLen );

            Serialize_printf( self, "\">\n" );

            SERIALIZE_INDENT_INCR( self );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_doSerialize( Serialize *self,
                                            SerializeType type,
                                            const char *name,
                                            void *value,
                                            const int size,
                                            const int len )
{
    int i = 0;
    bool isCharType = false;
    bool isField = false;
    bool isString = false;
    bool isArrayElement = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );

    isCharType = (( type == SERIALIZE_TYPE_CHAR ) ||
                  ( type == SERIALIZE_TYPE_UCHAR ) ||
                  ( type == SERIALIZE_TYPE_SCHAR ) ||
                  ( type == SERIALIZE_TYPE_CHARARRAY ) ||
                  ( type == SERIALIZE_TYPE_UCHARARRAY ) ||
                  ( type == SERIALIZE_TYPE_SCHARARRAY ) ? true : false );

    //isField = ( ( len == 1 ) ? true : false );
    isString = (( type == SERIALIZE_TYPE_STRING ) ? true : false );
    //isArrayElement = ( ( len > 1 ) ? true : false );

    /* Single-element array fix */
    //isArrayElement = SERIALIZE_IS_ARRAY_ELEM( type );
    isArrayElement = SERIALIZE_IS_ARRAY_ELEMENT( type );
    isField = !isArrayElement;

    for( ; ; )
    {
        /* Wraps Standard Chars */
        if( isCharType == true )
        {
            for( i = 0; i < len; i++ )
            {
                SerializeFormatXml_doSerializeCharType( self, type, name, value, size, len, i );
            }
            break;
        }

        if( isString == true )
        {
            SerializeFormatXml_doSerializeString( self, type, name, value, size, len );
            break;
        }

        if( isField == true )
        {
            SerializeFormatXml_doSerializeField( self, type, name, value, size );
            break;
        }

        if( isArrayElement == true )
        {
            for( i = 0; i < len; i++ )
            {
                SerializeFormatXml_doSerializeArrayElement( self, type, name, value, size, len, i, true );
            }
            break;
        }

        break;
    }
}


static void SerializeFormatXml_beginStructArray( Serialize *self,
                                                 const char *arrayName,
                                                 const char *elementType,
                                                 const int arrayLen )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );
    ANY_REQUIRE( elementType );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Any_snprintf( buffer,
                          SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<array type=\"\" name=\"\" size=\"\"> " ),
                          "<array type=\"%s\" name=\"%s\" size=\"%d\"> ",
                          elementType, arrayName, arrayLen );

            Serialize_scanf( self, buffer );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );

            Any_snprintf( buffer,
                          SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<array type=\"\" name=\"\" size=\"" ),
                          "<array type=\"%s\" name=\"%s\" size=\"", elementType, arrayName );

            Serialize_printf( self, buffer );

            Serialize_deployDataType( self,
                                      SERIALIZE_TYPE_INT,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      "%d", 0, 0, (void *)&arrayLen );

            Serialize_printf( self, "\">\n" );

            SERIALIZE_INDENT_INCR( self );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_beginStructArraySeparator( Serialize *self,
                                                          const char *name,
                                                          const int pos,
                                                          const int len )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];

    ANY_REQUIRE( self );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Any_sprintf( buffer, "<element index=\"%d\"> ", pos );
            Serialize_scanf( self, buffer );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );

            Serialize_printf( self, "<element index=\"" );

            Serialize_deployDataType( self,
                                      SERIALIZE_TYPE_INT,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      "%d", 0, 0, (void *)&pos );

            Serialize_printf( self, "\">\n" );

            SERIALIZE_INDENT_INCR( self );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_endStructArraySeparator( Serialize *self,
                                                        const char *name,
                                                        const int pos,
                                                        const int len )
{
    ANY_REQUIRE( self );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "</element> " );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "</element>\n" );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_endStructArray( Serialize *self )
{
    ANY_REQUIRE( self );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "</array> " );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "</array>\n" );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_endArray( Serialize *self,
                                         SerializeType type,
                                         const char *arrayName,
                                         const int arrayLen )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "</array> " );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "</array>\n" );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_endBaseType( Serialize *self )
{
    ANY_REQUIRE( self );

    /* End type MUST match also the newline */
    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            /* SERIALIZE_INDENT_DECR( self ); */
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_endType( Serialize *self )
{
    SerializeFormatXmlOptions *ptr = NULL;

    ANY_REQUIRE( self );

    /* End type MUST match also the newline */
    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "</struct>\n" );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "</struct>\n" );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }

    ptr = (SerializeFormatXmlOptions *)Serialize_getFormatDataPtr( self );
    ptr->baseTypeEnable = self->baseTypeEnable;

}


static void *SerializeFormatXmlOptions_new( void )
{
    SerializeFormatXmlOptions *data = (SerializeFormatXmlOptions *)NULL;

    data = ANY_TALLOC( SerializeFormatXmlOptions );
    ANY_REQUIRE( data );

    return (void *)data;
}


static void SerializeFormatXmlOptions_init( Serialize *self )
{
    SerializeFormatXmlOptions *data = (SerializeFormatXmlOptions *)NULL;

    ANY_REQUIRE( self );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    /* Default init option */
    data->baseTypeEnable = false;
}


static void SerializeFormatXmlOptions_set( Serialize *self,
                                           const char *optionsString )
{
    ANY_REQUIRE( self );
}


static bool SerializeFormatXmlOptions_setProperty( Serialize *self,
                                                   const char *propertyName, void *propertyValue )
{
    bool retVal = false;

    ANY_REQUIRE( self );

    return retVal;
}


static void *SerializeFormatXmlOptions_getProperty( Serialize *self, const char *propertyName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );

    return retVal;
}


static void SerializeFormatXmlOptions_clear( Serialize *self )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatXmlOptions_delete( Serialize *self )
{
    SerializeFormatXmlOptions *data = (SerializeFormatXmlOptions *)NULL;

    ANY_REQUIRE( self );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    ANY_REQUIRE( self );

    ANY_FREE( data );
}


static void SerializeFormatXml_getTypeInfo( SerializeType type,
                                            char *spec,
                                            char *typeTag )
{
#define TYPEINFO( __specPtr, __spec, __typeTagPtr, __typeTag )  \
  do{                                                           \
    Any_sprintf( __specPtr, "%s", __spec );                     \
    Any_sprintf( __typeTagPtr, "%s", __typeTag );               \
  } while( 0 )

    /* Warning: for array types, the type of the single element is returned */

    switch( type )
    {
        case SERIALIZE_TYPE_CHAR:
        case SERIALIZE_TYPE_CHARARRAY:
            TYPEINFO( spec, "%d", typeTag, "char" );
            break;

        case SERIALIZE_TYPE_SCHAR:
        case SERIALIZE_TYPE_SCHARARRAY:
            TYPEINFO( spec, "%d", typeTag, "signed char" );
            break;

        case SERIALIZE_TYPE_UCHAR:
        case SERIALIZE_TYPE_UCHARARRAY:
            TYPEINFO( spec, "%u", typeTag, "unsigned char" );
            break;

        case SERIALIZE_TYPE_SINT:
        case SERIALIZE_TYPE_SINTARRAY:
            TYPEINFO( spec, "%hd", typeTag, "short int" );
            break;

        case SERIALIZE_TYPE_USINT:
        case SERIALIZE_TYPE_USINTARRAY:
            TYPEINFO( spec, "%hu", typeTag, "unsigned short int" );
            break;

        case SERIALIZE_TYPE_INT:
        case SERIALIZE_TYPE_INTARRAY:
            TYPEINFO( spec, "%d", typeTag, "int" );
            break;

        case SERIALIZE_TYPE_UINT:
        case SERIALIZE_TYPE_UINTARRAY:
            TYPEINFO( spec, "%u", typeTag, "unsigned int" );
            break;

        case SERIALIZE_TYPE_LINT:
        case SERIALIZE_TYPE_LINTARRAY:
            TYPEINFO( spec, "%ld", typeTag, "long int" );
            break;

        case SERIALIZE_TYPE_ULINT:
        case SERIALIZE_TYPE_ULINTARRAY:
            TYPEINFO( spec, "%lu", typeTag, "unsigned long int" );
            break;

        case SERIALIZE_TYPE_LL:
        case SERIALIZE_TYPE_LLARRAY:
            TYPEINFO( spec, "%lld", typeTag, "long long int" );
            break;

        case SERIALIZE_TYPE_ULL:
        case SERIALIZE_TYPE_ULLARRAY:
            TYPEINFO( spec, "%llu", typeTag, "unsigned long long int" );
            break;

        case SERIALIZE_TYPE_FLOAT:
        case SERIALIZE_TYPE_FLOATARRAY:
            TYPEINFO( spec, "%f", typeTag, "float" );
            break;

        case SERIALIZE_TYPE_DOUBLE:
        case SERIALIZE_TYPE_DOUBLEARRAY:
            TYPEINFO( spec, "%lf", typeTag, "double" );
            break;
        case SERIALIZE_TYPE_LDOUBLE:
        case SERIALIZE_TYPE_LDOUBLEARRAY:
            TYPEINFO( spec, "%LF", typeTag, "long double" );
            break;

        case SERIALIZE_TYPE_STRING :
            /* Size is not used on Strings */
            TYPEINFO( spec, "%qs", typeTag, "string" );
            break;
        default:
            ANY_LOG( 0, "SerializeFormatXml_getTypeInfo. Unknown SerializeType : %d",
                     ANY_LOG_ERROR, type );
            ANY_REQUIRE( NULL );
            break;
    }
#undef TYPEINFO
}


static void SerializeFormatXml_doSerializeCharType( Serialize *self,
                                                    SerializeType type,
                                                    const char *name,
                                                    void *value,
                                                    const int size,
                                                    const int len,
                                                    const int index )
{
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char *signedCharPtr = (char *)NULL;
    unsigned char *unsignedCharPtr = (unsigned char *)NULL;
    int auxData = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    SerializeFormatXml_getTypeInfo( type, spec, typeTag );

    if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR ||
        type == SERIALIZE_TYPE_CHARARRAY || type == SERIALIZE_TYPE_SCHARARRAY )
    {
        signedCharPtr = value;
        ANY_REQUIRE( signedCharPtr );
        signedCharPtr += size * index;
        ANY_REQUIRE( signedCharPtr );

        auxData = ((int)*signedCharPtr );
    }
    else
    {
        unsignedCharPtr = value;
        ANY_REQUIRE( unsignedCharPtr );
        unsignedCharPtr += size * index;
        ANY_REQUIRE( unsignedCharPtr );

        auxData = ((int)*unsignedCharPtr );
    }

    /* Single-element array fix */
    //if( len == 1 )
    if( SERIALIZE_IS_ARRAY_ELEMENT( type ) == false )
    {
        SerializeFormatXml_doSerializeField( self, type, name, &auxData, size );

        if( self->mode == SERIALIZE_MODE_READ )
        {
            if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR )
            {
                ANY_REQUIRE( auxData <= SCHAR_MAX );
                ANY_REQUIRE( auxData >= SCHAR_MIN );
                ANY_REQUIRE( signedCharPtr );
                *signedCharPtr = (char)auxData;
            }
            else
            {
                ANY_REQUIRE( auxData <= UCHAR_MAX );
                ANY_REQUIRE( unsignedCharPtr );
                *unsignedCharPtr = (unsigned char)auxData;
            }
        }
    }
    else
    {
        SerializeFormatXml_doSerializeArrayElement( self, type, name, &auxData, size, len, index, false );

        if( self->mode == SERIALIZE_MODE_READ )
        {
            if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR ||
                type == SERIALIZE_TYPE_CHARARRAY || type == SERIALIZE_TYPE_SCHARARRAY )
            {
                ANY_REQUIRE( auxData <= SCHAR_MAX );
                ANY_REQUIRE( auxData >= SCHAR_MIN );
                ANY_REQUIRE( signedCharPtr );
                *signedCharPtr = (char)auxData;
            }
            else
            {
                ANY_REQUIRE( auxData <= UCHAR_MAX );
                ANY_REQUIRE( unsignedCharPtr );
                *unsignedCharPtr = (unsigned char)auxData;
            }
        }
    }
}


static void SerializeFormatXml_doSerializeField( Serialize *self,
                                                 SerializeType type,
                                                 const char *name,
                                                 void *value,
                                                 const int size )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    SerializeFormatXmlOptions *ptr = NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );

    SerializeFormatXml_getTypeInfo( type, spec, typeTag );

    ptr = (SerializeFormatXmlOptions *)Serialize_getFormatDataPtr( self );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( ptr->baseTypeEnable == true )
            {
                char instanceName[256];

                Any_snprintf( buffer,
                              SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<field type=\"\" name=\"%\"></field> " ),
                              "<field type=\"%s\" name=\"%%s\">%s</field> ", typeTag, spec );

                Serialize_scanf( self, buffer, instanceName, value );
                break;
            } else
            {
                Any_snprintf( buffer,
                              SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<field type=\"\" name=\"\"></field> " ),
                              "<field type=\"%s\" name=\"%s\">%s</field> ", typeTag, name, spec );

                Serialize_scanf( self, buffer, value );
                break;
            }
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "<field type=\"%s\" name=\"%s\">", typeTag, name );

            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, value );

            Serialize_printf( self, "</field>\n" );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_doSerializeString( Serialize *self,
                                                  SerializeType type,
                                                  const char *name,
                                                  void *value,
                                                  const int size,
                                                  const int len )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            /* 1: Read first part of the string */
            Any_snprintf( buffer,
                          SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<field type=\"string\" name=\"\">" ),
                          "<field type=\"string\" name=\"%s\">", name );

            Serialize_scanf( self, buffer, (char *)name );
            /* 2: Parse field value, correctly translating from the escaped sequences */
            SerializeFormatXml_doSerializeStringCharUnescaping( self, (char *)value );
            /* 3: Terminate string */
            Serialize_scanf( self, "/field>" );
            break;
        }

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );
            /* 1: Write first part of the string */
            Serialize_printf( self, "<field type=\"string\" name=\"%s\">", name );
            /* 2: Write field value, correctly translating from the escaped sequences */
            SerializeFormatXml_doSerializeStringCharEscaping( self, (char *)value );
            /* 3: Terminate string */
            Serialize_printf( self, "</field>\n", name );
            break;
        }

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


static void SerializeFormatXml_doSerializeArrayElement( Serialize *self,
                                                        SerializeType type,
                                                        const char *name,
                                                        void *value,
                                                        const int size,
                                                        const int len,
                                                        const int index,
                                                        bool reIndexOffset )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char *ptr = (char *)NULL;

    /*bool isLastArrayElement = ((index == (len-1))  ? true : false );*/

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );

    SerializeFormatXml_getTypeInfo( type, spec, typeTag );

    if( reIndexOffset == true )
    {
        ptr = value;
        ptr += size * index;
        ANY_REQUIRE( ptr );
    }
    else
    {
        ptr = value;
    }

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Any_snprintf( buffer,
                          SERIALIZE_DATABUFFER_MAXLEN - sizeof( "<element index=\"\"></element> " ),
                          "<element index=\"%d\">%s</element> ", index, spec );

            Serialize_scanf( self, buffer, ptr );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            SERIALIZE_INDENT( self );

            Serialize_printf( self, "<element index=\"" );

            Serialize_deployDataType( self,
                                      SERIALIZE_TYPE_INT,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      "%d", 0, 0, (void *)&index );

            Serialize_printf( self, "\">" );

            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, ptr );

            Serialize_printf( self, "</element>\n" );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_ERROR, self->mode );
            break;
    }
}


/*
 * Predefined entities in XML
 * NAME   CHARACTER
 * amp        &
 * lt         <
 * gt         >
 * quot       "
 * apos       '
 */
static bool SerializeFormatXml_doSerializeStringCharEscaping( Serialize *self, char *value )
{
    long printedBytes = 0;
    bool retVal = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( value );

    /*
   * Strightforward substitution: any time we read from source
   *  a character belonging to the list of XML predefined
   *  entities, we swap it with its escaping sequence;
   *  otherwise, we write the value read verbatim.
   */
    do
    {
        if( *value == '"' )
        {
            printedBytes = IOChannel_printf( self->stream, "&quot;" );
            if( printedBytes != sizeof( "&quot;" ) - 1 )
            {
                ANY_LOG( 0, "Error while writing to stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
        }
        else if( *value == '\'' )
        {
            printedBytes = IOChannel_printf( self->stream, "&apos;" );
            if( printedBytes != sizeof( "&apos;" ) - 1 )
            {
                ANY_LOG( 0, "Error while writing to stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
        }
        else if( *value == '<' )
        {
            printedBytes = IOChannel_printf( self->stream, "&lt;" );
            if( printedBytes != sizeof( "&lt;" ) - 1 )
            {
                ANY_LOG( 0, "Error while writing to stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
        }
        else if( *value == '>' )
        {
            printedBytes = IOChannel_printf( self->stream, "&gt;" );
            if( printedBytes != sizeof( "&gt;" ) - 1 )
            {
                ANY_LOG( 0, "Error while writing to stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
        }
        else if( *value == '&' )
        {
            printedBytes = IOChannel_printf( self->stream, "&amp;" );
            if( printedBytes != sizeof( "&amp;" ) - 1 )
            {
                ANY_LOG( 0, "Error while writing to stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
        }
        else if( *value == '\0' )
        {
            /* Empty string condition. */
            printedBytes = IOChannel_printf( self->stream, "\0" );
            if( printedBytes == -1 )
            {
                ANY_LOG( 0, "Error while writing to stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
            retVal = true;
            goto exitLabel;
        }
        else
        {
            /* No special character handling needed, let's just write the value as it is! */
            printedBytes = IOChannel_printf( self->stream, "%c", value );
            if( printedBytes != 1 )
            {
                ANY_LOG( 0, "Error while writing to stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
        }
    } while( *++value );

    retVal = true;

    exitLabel:
    return retVal;
}


static bool SerializeFormatXml_doSerializeStringCharUnescaping( Serialize *self, char *destination )
{
    char value[10];
    long readBytes = 0;
    bool retVal = false;

    Any_memset( value, 0, 10 );

    /*
   * Entry point: read first character in the string
   *   and begin parsing.
   */
    readBytes = IOChannel_read( self->stream, value, 1 );
    if( readBytes != 1 )
    {
        ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
        goto exitLabel;
    }

    do
    {
        /*
     * Check if we are reading a predefined entity or
     *  if we reached the EOS.
     */
        if( *value == '&' || *value == '<' )
        {
            /* End of string - terminate destination string with EOS and exit successfully */
            if( *value == '<' )
            {
                *destination = '\0';
                retVal = true;
                goto exitLabel;
            }
            /*
       * No EOS: determine which predefined entity we
       *  are dealing with.
       */
            readBytes = IOChannel_read( self->stream, value, 1 );
            if( readBytes != 1 )
            {
                ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
                goto exitLabel;
            }
            switch( *value )
            {
                case 'q':
                    /* &quot; -> `"' */
                    readBytes = IOChannel_read( self->stream, value, 4 );
                    if( readBytes != 4 )
                    {
                        ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
                        goto exitLabel;
                    }
                    if( Any_strcmp( value, "uot;" ) == 0 )
                    {
                        *destination++ = '"';
                    }
                    else
                    {
                        ANY_LOG( 0, "Error: string not well-formed. Expected [&quot;] but got [&q%s]", ANY_LOG_ERROR,
                                 value );
                        goto exitLabel;
                    }
                    break;
                case 'l':
                    /* &lt; -> `<' */
                    readBytes = IOChannel_read( self->stream, value, 2 );
                    if( readBytes != 2 )
                    {
                        ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
                        goto exitLabel;
                    }
                    if( Any_strcmp( value, "t;" ) == 0 )
                    {
                        *destination++ = '<';
                    }
                    else
                    {
                        ANY_LOG( 0, "Error: string not well-formed. Expected [&lt;] but got [&l%s]", ANY_LOG_ERROR,
                                 value );
                        goto exitLabel;
                    }
                    break;
                case 'g':
                    /* &gt; -> `>' */
                    readBytes = IOChannel_read( self->stream, value, 2 );
                    if( readBytes != 2 )
                    {
                        ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
                        goto exitLabel;
                    }
                    if( Any_strcmp( value, "t;" ) == 0 )
                    {
                        *destination++ = '>';
                    }
                    else
                    {
                        ANY_LOG( 0, "Error: string not well-formed. Expected [&gt;] but got [&g%s]", ANY_LOG_ERROR,
                                 value );
                        goto exitLabel;
                    }
                    break;
                case 'a':
                    readBytes = IOChannel_read( self->stream, value, 1 );
                    if( readBytes != 1 )
                    {
                        ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
                        goto exitLabel;
                    }
                    switch( *value )
                    {
                        case 'p':
                            /* &apos; -> `'' */
                            readBytes = IOChannel_read( self->stream, value, 3 );
                            if( readBytes != 3 )
                            {
                                ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
                                goto exitLabel;
                            }
                            if( Any_strcmp( value, "os;" ) == 0 )
                            {
                                *destination++ = '\'';
                            }
                            else
                            {
                                ANY_LOG( 0, "Error: string not well-formed. Expected [&apos;] but got [&ap%s]",
                                         ANY_LOG_ERROR, value );
                                goto exitLabel;
                            }
                            break;
                        case 'm':
                            /* &amp; -> `&' */
                            readBytes = IOChannel_read( self->stream, value, 2 );
                            if( readBytes != 2 )
                            {
                                ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
                                goto exitLabel;
                            }
                            if( Any_strcmp( value, "p;" ) == 0 )
                            {
                                *destination++ = '&';
                            }
                            else
                            {
                                ANY_LOG( 0, "Error: string not well-formed. Expected [&amp;] but got [&am%s]",
                                         ANY_LOG_ERROR, value );
                                goto exitLabel;
                            }
                            break;
                    }
                    break;
                default:
                    /* & followed by no valid character -> invalid escape sequence */
                    ANY_LOG( 0, "Error: string not well-formed.", ANY_LOG_ERROR );
                    goto exitLabel;
            }
        }
        else
        {
            /*
       * We did not find any predefined entity nor
       *   the EOS, so we write the value verbatim.
       */
            *destination++ = *value;
        }
        /*
     * This is really important since it prevents clobbering
     *   in the 'value' buffer, which would be detrimental to
     *   the correct parsing of the string.
     */
        Any_memset( value, 0, 10 );
        readBytes = IOChannel_read( self->stream, value, 1 );
        if( readBytes != 1 )
        {
            ANY_LOG( 0, "Error while reading from stream.", ANY_LOG_ERROR );
            goto exitLabel;
        }
    } while( true );

    exitLabel:;
    return retVal;
}


/*--------------------------------------------------------------------------*/
/* Python format                                                            */
/*--------------------------------------------------------------------------*/


SERIALIZEFORMAT_CREATE_PLUGIN( Python );


typedef enum SerializeFormatPythonType
{
    AS_TUPLE,
    AS_LIST,
    AS_DICT,
    AS_TUPLE_NO_KEY,
    AS_LIST_NO_KEY,
} SerializeFormatPythonType;

typedef enum SerializeFormatPythonArrayType
{
    ARRAY_AS_TUPLE = AS_LIST_NO_KEY + 1,
    ARRAY_AS_LIST,
    ARRAY_AS_DICT,
    ARRAY_AS_TUPLE_NO_INDEX,
    ARRAY_AS_LIST_NO_INDEX,
} SerializeFormatPythonArrayType;

typedef enum SerializeFormatPythonStructArrayType
{
    STRUCTARRAY_AS_TUPLE = ARRAY_AS_LIST_NO_INDEX + 1,
    STRUCTARRAY_AS_LIST,
    STRUCTARRAY_AS_DICT,
    STRUCTARRAY_AS_TUPLE_NO_INDEX,
    STRUCTARRAY_AS_LIST_NO_INDEX,
} SerializeFormatPythonStructArrayType;

typedef struct SerializeFormatPythonOptions
{
    int type;
    int arrayType;
    int structArrayType;
    bool beginStructArrayElem;
    int endStructArrayLevel;
} SerializeFormatPythonOptions;


#define SERIALIZE_STRUCT_OPEN( __type )\
  ( ( __type == AS_TUPLE || __type == AS_TUPLE_NO_KEY ) ? "(" :        \
    ( __type == AS_LIST || __type == AS_LIST_NO_KEY ) ? "[" : "{" )

#define SERIALIZE_STRUCT_CLOSE( __type )\
  ( ( __type == AS_TUPLE || __type == AS_TUPLE_NO_KEY ) ? ")" :        \
    ( __type == AS_LIST || __type == AS_LIST_NO_KEY ) ? "]" : "}" )

#define SERIALIZE_STRUCT_HAS_KEY( __type )\
  ( ( ( __type == AS_TUPLE ) ||        \
      ( __type == AS_LIST ) ||        \
      ( __type == AS_DICT ) ) ? true : false )

#define SERIALIZE_KEY_OPEN( __type ) \
  ( ( __type == AS_TUPLE || \
      __type == AS_LIST ) ? "( %qs, " : "%qs : " )

#define SERIALIZE_KEY_CLOSE( __type ) \
  ( ( __type == AS_TUPLE || \
      __type == AS_LIST ) ? " )" : "" )

#define SERIALIZE_ARRAY_OPEN( __arrayType )\
  ( ( __arrayType == ARRAY_AS_TUPLE || \
      __arrayType == ARRAY_AS_TUPLE_NO_INDEX ) ? "(" : \
    ( __arrayType == ARRAY_AS_LIST || \
      __arrayType == ARRAY_AS_LIST_NO_INDEX ) ? "[" : "{" )

#define SERIALIZE_ARRAY_CLOSE( __arrayType )\
  ( ( __arrayType == ARRAY_AS_TUPLE || \
      __arrayType == ARRAY_AS_TUPLE_NO_INDEX ) ? ")" : \
    ( __arrayType == ARRAY_AS_LIST || \
      __arrayType == ARRAY_AS_LIST_NO_INDEX ) ? "]" : "}" )

#define SERIALIZE_ARRAY_HAS_INDEX( __arrayType )\
  ( ( __arrayType == ARRAY_AS_TUPLE || \
      __arrayType == ARRAY_AS_LIST || \
      __arrayType == ARRAY_AS_DICT ) ? true : false )

#define SERIALIZE_ARRAY_INDEX_OPEN( __arrayType ) \
  ( ( __arrayType == ARRAY_AS_TUPLE || \
      __arrayType == ARRAY_AS_LIST ) ? "( %d, " : "%d : " )

#define SERIALIZE_ARRAY_INDEX_CLOSE( __arrayType ) \
  ( ( __arrayType == ARRAY_AS_TUPLE || \
      __arrayType == ARRAY_AS_LIST ) ? " )" : "" )

#define SERIALIZE_STRUCTARRAY_OPEN( __structArrayType )\
  ( ( __structArrayType == STRUCTARRAY_AS_TUPLE || \
      __structArrayType == STRUCTARRAY_AS_TUPLE_NO_INDEX ) ? "(" : \
    ( __structArrayType == STRUCTARRAY_AS_LIST || \
      __structArrayType == STRUCTARRAY_AS_LIST_NO_INDEX ) ? "[" : "{" )

#define SERIALIZE_STRUCTARRAY_CLOSE( __structArrayType )\
  ( ( __structArrayType == STRUCTARRAY_AS_TUPLE || \
      __structArrayType == STRUCTARRAY_AS_TUPLE_NO_INDEX ) ? ")" : \
    ( __structArrayType == STRUCTARRAY_AS_LIST || \
      __structArrayType == STRUCTARRAY_AS_LIST_NO_INDEX ) ? "]" : "}" )

#define SERIALIZE_STRUCTARRAY_HAS_INDEX( __structArrayType )\
  ( ( __structArrayType == STRUCTARRAY_AS_TUPLE || \
      __structArrayType == STRUCTARRAY_AS_LIST || \
      __structArrayType == STRUCTARRAY_AS_DICT ) ? true : false )

#define SERIALIZE_STRUCTARRAY_INDEX_OPEN( __structArrayType ) \
  ( ( __structArrayType == STRUCTARRAY_AS_TUPLE || \
      __structArrayType == STRUCTARRAY_AS_LIST ) ? "( %d, " : "%d : " )

#define SERIALIZE_STRUCTARRAY_INDEX_CLOSE( __structArrayType ) \
  ( ( __structArrayType == STRUCTARRAY_AS_TUPLE || \
      __structArrayType == STRUCTARRAY_AS_LIST ) ? ")" : "" )


static void SerializeFormatPython_getTypeInfo( SerializeType type,
                                               char *spec );

static void SerializeFormatPython_doSerializeField( Serialize *self, \
                                                    SerializeType type, \
                                                    const void *name, \
                                                    void *value, \
                                                    const int size, \
                                                    bool isArrayElem );

static void SerializeFormatPython_doSerializeString( Serialize *self, \
                                                     SerializeType type, \
                                                     const char *name, \
                                                     void *value, \
                                                     const int size, \
                                                     const int len );

static void SerializeFormatPython_doSerializeArrayElement( Serialize *self, \
                                                           SerializeType type, \
                                                           const char *name, \
                                                           void *value, \
                                                           const int size, \
                                                           const int len, \
                                                           const int index );


static void SerializeFormatPython_beginType( Serialize *self, \
                                             const char *name, \
                                             const char *type )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    char bufferName[SERIALIZE_DATABUFFER_MAXLEN];
    int nestingLevels;

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );
    ANY_REQUIRE( data );

    nestingLevels = Serialize_getBeginTypeNestingLevel( self );

    if( nestingLevels == 1 || data->beginStructArrayElem == true )
    {
        /* Root structure */
        if( Serialize_isReading( self ))
        {
            Serialize_scanf( self,
                             SERIALIZE_STRUCT_OPEN( data->type ));
            Serialize_scanf( self, "\\ " );
        }
        else
        {
            SERIALIZE_INDENT( self );
            Serialize_printf( self,
                              SERIALIZE_STRUCT_OPEN( data->type ));
            Serialize_printf( self, "\\\n" );
        }

        /* Reset flag used to begin struct array elements.  */
        data->beginStructArrayElem = false;

    }
    else
    {

        if( Serialize_isReading( self ))
        {
            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {

                Serialize_scanf( self, SERIALIZE_KEY_OPEN( data->type ), bufferName );

                /* Check if serialized and deserialized filed names are equal */
                ANY_REQUIRE_VMSG( !strcmp( bufferName, name ), \
                          "Different serialized-deserialized field names: found %s, expected %s", \
                          name, bufferName );
            }
            Serialize_scanf( self, SERIALIZE_STRUCT_OPEN( data->type ));
            Serialize_scanf( self, "\\ " );
        }
        else
        {
            /* Append an inner structure */
            SERIALIZE_INDENT( self );
            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {
                Serialize_printf( self, SERIALIZE_KEY_OPEN( data->type ), name );
            }
            Serialize_printf( self,
                              SERIALIZE_STRUCT_OPEN( data->type ));
            Serialize_printf( self, "\\\n" );
        }

    }

    SERIALIZE_INDENT_INCR( self );

}


static void SerializeFormatPython_beginBaseType( Serialize *self, \
                                                 const char *name, \
                                                 const char *type )
{
    ;
}


static void SerializeFormatPython_beginArray( Serialize *self, \
                                              SerializeType type, \
                                              const char *name, \
                                              const int len )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( len > 0 );
    ANY_REQUIRE( data );

    if( Serialize_isReading( self ))
    {

        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_scanf( self, SERIALIZE_KEY_OPEN( data->type ), buffer );
            /* Check if serialized and deserialized array names are equal */
            ANY_REQUIRE_VMSG( !strcmp( buffer, name ), \
                        "Different serialized-deserialized array names: found %s, expected %s", \
                        name, buffer );
        }
        Serialize_scanf( self, SERIALIZE_ARRAY_OPEN( data->arrayType ));

    }
    else
    {

        SERIALIZE_INDENT( self );
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_printf( self, SERIALIZE_KEY_OPEN( data->type ), name );
        }
        Serialize_printf( self, SERIALIZE_ARRAY_OPEN( data->arrayType ));

    }

    SERIALIZE_INDENT_INCR( self );

}


static void SerializeFormatPython_beginStructArray( Serialize *self, \
                                                    const char *name, \
                                                    const char *elementType, \
                                                    const int len )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    char buffer[SERIALIZE_DATABUFFER_MAXLEN] = "";
    char bufferName[SERIALIZE_DATABUFFER_MAXLEN] = "";

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( elementType );
    ANY_REQUIRE( len > 0 );
    ANY_REQUIRE( data );

    if( Serialize_isReading( self ))
    {

        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_scanf( self, SERIALIZE_KEY_OPEN( data->type ), bufferName );
            /* Check if serialized and deserialized struct array names are equal */
            ANY_REQUIRE_VMSG( !strcmp( bufferName, name ), \
                        "Different serialized-deserialized struct array names: found %s, expected %s", \
                        name, buffer );
        }
        Serialize_scanf( self, SERIALIZE_STRUCTARRAY_OPEN( data->structArrayType ));
        Serialize_scanf( self, "\\ " );

    }
    else
    {

        SERIALIZE_INDENT( self );
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_printf( self, SERIALIZE_KEY_OPEN( data->type ), name );
        }
        Serialize_printf( self, SERIALIZE_STRUCTARRAY_OPEN( data->structArrayType ));
        Serialize_printf( self, "\\\n" );

    }

    SERIALIZE_INDENT_INCR( self );

}


static void SerializeFormatPython_beginStructArraySeparator( Serialize *self, \
                                                             const char *name, \
                                                             const int pos, \
                                                             const int len )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    int tmpPos = 0;

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( pos >= 0 );
    ANY_REQUIRE( data );

    /* Set flag used to open a struct array a element without element name. */
    data->beginStructArrayElem = true;

    if( Serialize_isReading( self ))
    {
        if( SERIALIZE_STRUCTARRAY_HAS_INDEX( data->structArrayType ))
        {

            Serialize_scanf( self, SERIALIZE_STRUCTARRAY_INDEX_OPEN( data->structArrayType ),
                             &tmpPos );
            ANY_REQUIRE_VMSG( tmpPos == pos,
                              "Different serialized-deserialized StructArray separator: found %d, expected %d",
                              pos, tmpPos );

            Serialize_scanf( self, "\\ " );
            SERIALIZE_INDENT_INCR( self );

        }
    }
    else
    {
        if( SERIALIZE_STRUCTARRAY_HAS_INDEX( data->structArrayType ))
        {
            tmpPos = pos;
            SERIALIZE_INDENT( self );
            Serialize_printf( self, SERIALIZE_STRUCTARRAY_INDEX_OPEN( data->structArrayType ),
                              &tmpPos );

            Serialize_printf( self, "\\\n" );
            SERIALIZE_INDENT_INCR( self );
        }
    }

    /* Save indent level.
     It will be useful when the struct array element needs to be closed.  */
    data->endStructArrayLevel = self->indentLevel;

}


static void SerializeFormatPython_doSerialize( Serialize *self, \
                                               SerializeType type, \
                                               const char *name, \
                                               void *value, \
                                               const int size, \
                                               const int len )
{
    bool isString;
    bool isArray;
    int i;

    ANY_REQUIRE( self );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    isString = ( type == SERIALIZE_TYPE_STRING ) ? true : false;
    isArray = SERIALIZE_IS_ARRAY_ELEMENT( type );

    while( true )
    {
        if( isString )
        {
            /* Serialize string */
            SerializeFormatPython_doSerializeString( self, type, name, \
                                               value, size, len );
            break;
        }
        else if( isArray )
        {
            for( i = 0; i < len; i++ )
            {
                /* Serialize array element */
                SerializeFormatPython_doSerializeArrayElement( self, type, \
                                                       name, value, \
                                                       size, len, i );
            }
            break;
        }
        else
        {
            /* Serialize field */
            SerializeFormatPython_doSerializeField( self, type, \
                                              name, value, \
                                              size, false );
            break;
        }
    }

}


static void SerializeFormatPython_endStructArraySeparator( Serialize *self, \
                                                           const char *name, \
                                                           const int pos, \
                                                           const int len )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( pos >= 0 );
    ANY_REQUIRE( data );

    if( Serialize_isReading( self ))
    {
        if( SERIALIZE_STRUCTARRAY_HAS_INDEX( data->structArrayType ))
        {
            SERIALIZE_INDENT_DECR( self );

            if( data->structArrayType != STRUCTARRAY_AS_DICT )
            {

                Serialize_scanf( self,
                                 SERIALIZE_STRUCTARRAY_INDEX_CLOSE( data->structArrayType ));
                Serialize_scanf( self, ",\\ " );

            }

        }
    }
    else
    {
        if( SERIALIZE_STRUCTARRAY_HAS_INDEX( data->structArrayType ))
        {
            SERIALIZE_INDENT_DECR( self );

            if( data->structArrayType != STRUCTARRAY_AS_DICT )
            {

                SERIALIZE_INDENT( self );
                Serialize_printf( self,
                                  SERIALIZE_STRUCTARRAY_INDEX_CLOSE( data->structArrayType ));
                Serialize_printf( self, ",\\\n" );

            }

        }
    }

}


static void SerializeFormatPython_endStructArray( Serialize *self )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( data );

    SERIALIZE_INDENT_DECR( self );

    if( Serialize_isReading( self ))
    {
        Serialize_scanf( self, SERIALIZE_STRUCTARRAY_CLOSE( data->structArrayType ));
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_scanf( self, SERIALIZE_KEY_CLOSE( data->type ));
        }
        Serialize_scanf( self, ",\\ " );
    }
    else
    {

        SERIALIZE_INDENT( self );
        Serialize_printf( self, SERIALIZE_STRUCTARRAY_CLOSE( data->structArrayType ));
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_printf( self, SERIALIZE_KEY_CLOSE( data->type ));
        }
        Serialize_printf( self, ",\\\n" );
    }

}


static void SerializeFormatPython_endArray( Serialize *self, \
                                            SerializeType type, \
                                            const char *name, \
                                            const int len )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( len > 0 );
    ANY_REQUIRE( data );

    if( Serialize_isReading( self ))
    {
        SERIALIZE_INDENT_DECR( self );
        Serialize_scanf( self, "\\ " );
        Serialize_scanf( self, SERIALIZE_ARRAY_CLOSE( data->arrayType ));
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_scanf( self, SERIALIZE_KEY_CLOSE( data->type ));
        }
        Serialize_scanf( self, ",\\ " );
    }
    else
    {
        SERIALIZE_INDENT_DECR( self );
        Serialize_printf( self, "\\\n" );
        SERIALIZE_INDENT( self );
        Serialize_printf( self, SERIALIZE_ARRAY_CLOSE( data->arrayType ));
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_printf( self, SERIALIZE_KEY_CLOSE( data->type ));
        }
        Serialize_printf( self, ",\\\n" );
    }

}


static void SerializeFormatPython_endBaseType( Serialize *self )
{
    /* Not yet implemented */
    ;
}


static void SerializeFormatPython_endType( Serialize *self )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    int nestingLevels;

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( data );

    nestingLevels = Serialize_getBeginTypeNestingLevel( self );

    SERIALIZE_INDENT_DECR( self );

    if( nestingLevels == 1 ||
        self->indentLevel == data->endStructArrayLevel )
    {

        /* Root structure */
        if( Serialize_isReading( self ))
        {
            Serialize_scanf( self, SERIALIZE_STRUCT_CLOSE( data->type ));
            /* If endStructArrayLevel flag and the instance indent level are equale,
       we're closing a struct array element */
            if( data->endStructArrayLevel == self->indentLevel )
            {
                Serialize_scanf( self, ",\\ " );
            }
            else
            {
                Serialize_scanf( self, " " );
            }
        }
        else
        {
            SERIALIZE_INDENT( self );
            Serialize_printf( self, SERIALIZE_STRUCT_CLOSE( data->type ));
            /* If endStructArrayLevel flag and the instance indent level are equale,
       we're closing a struct array element */
            if( data->endStructArrayLevel == self->indentLevel )
            {
                Serialize_printf( self, ",\\\n" );
            }
            else
            {
                Serialize_printf( self, "\n" );
            }
        }

        /* Reset endStructArrayLevel flag */
        data->endStructArrayLevel = -1;

    }
    else
    {

        /* Nested structure */
        if( Serialize_isReading( self ))
        {
            Serialize_scanf( self, SERIALIZE_STRUCT_CLOSE( data->type ));
            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {
                if( data->type != AS_DICT )
                {
                    Serialize_scanf( self, SERIALIZE_KEY_CLOSE( data->type ));
                }
            }
            Serialize_scanf( self, ", \\ " );
        }
        else
        {
            SERIALIZE_INDENT( self );
            Serialize_printf( self, SERIALIZE_STRUCT_CLOSE( data->type ));
            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {
                if( data->type != AS_DICT )
                {
                    Serialize_printf( self, SERIALIZE_KEY_CLOSE( data->type ));
                }
            }
            Serialize_printf( self, ", \\\n" );
        }

    }

}


static int SerializeFormatPython_getAllowedModes( Serialize *self )
{
    int modes = SERIALIZE_MODE_CALC;

    ANY_REQUIRE( self );

    return modes;
}


static void *SerializeFormatPythonOptions_new( void )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    data = ANY_TALLOC( SerializeFormatPythonOptions );

    ANY_REQUIRE( data );

    return data;
}


static void SerializeFormatPythonOptions_init( Serialize *self )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;

    ANY_REQUIRE( self );
    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    data->beginStructArrayElem = false;
    data->endStructArrayLevel = -1;

    /* Set default values */
    data->type = AS_LIST;
    data->arrayType = ARRAY_AS_LIST;
    data->structArrayType = STRUCTARRAY_AS_LIST;

}


static void SerializeFormatPythonOptions_set( Serialize *self,
                                              const char *optionsString )
{
/* Option values validity checks */
#define SERIALIZE_TYPE_VALID( __type )\
  ( ( !Any_strcmp( __type, "AS_TUPLE" ) || \
      !Any_strcmp( __type, "AS_LIST" ) ||        \
      !Any_strcmp( __type, "AS_DICT" ) ||        \
      !Any_strcmp( __type, "AS_TUPLE_NO_KEY" ) ||         \
      !Any_strcmp( __type, "AS_LIST_NO_KEY" ) ) ? true : false )

#define SERIALIZE_ARRAY_VALID( __arrayType )\
  ( ( !Any_strcmp( __arrayType, "ARRAY_AS_TUPLE" ) || \
      !Any_strcmp( __arrayType, "ARRAY_AS_LIST" ) ||  \
      !Any_strcmp( __arrayType, "ARRAY_AS_DICT" ) ||   \
      !Any_strcmp( __arrayType, "ARRAY_AS_TUPLE_NO_INDEX" ) ||         \
      !Any_strcmp( __arrayType, "ARRAY_AS_LIST_NO_INDEX" ) ) ? true : false )

#define SERIALIZE_STRUCTARRAY_VALID( __structArrayType )\
  ( ( !Any_strcmp( __structArrayType, "STRUCTARRAY_AS_TUPLE" ) || \
      !Any_strcmp( __structArrayType, "STRUCTARRAY_AS_LIST" ) ||  \
      !Any_strcmp( __structArrayType, "STRUCTARRAY_AS_DICT" ) ||   \
      !Any_strcmp( __structArrayType, "STRUCTARRAY_AS_TUPLE_NO_INDEX" ) || \
      !Any_strcmp( __structArrayType, "STRUCTARRAY_AS_LIST_NO_INDEX" ) ) ? true : false )

#define SERIALIZE_CHECK_OPTION( __option, __type )\
  ( !Any_strcmp( __option, #__type ) ? true : false )

    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    char bufferOption[SERIALIZE_DATABUFFER_MAXLEN];
    char bufferValue[SERIALIZE_DATABUFFER_MAXLEN];

    char bufferType[SERIALIZE_DATABUFFER_MAXLEN + 7];  /* 7 == space for "STRUCT" */
    char bufferArray[SERIALIZE_DATABUFFER_MAXLEN];
    char bufferStructArray[SERIALIZE_DATABUFFER_MAXLEN];

    char *charPtr = (char *)NULL;
    int offset = 0;
    int len = Any_strlen( optionsString );

    ANY_REQUIRE( self );
    ANY_REQUIRE( optionsString );

    data = Serialize_getFormatDataPtr( self );
    charPtr = Serialize_getHeaderOptsPtr( self );
    ANY_REQUIRE( data );
    ANY_REQUIRE( charPtr );

    /* Get and store previous values */
    if( Any_sscanf( charPtr, "%518s %511s %511s",
                    bufferType, bufferArray, bufferStructArray ) != 3 )
    {
        /* Discard previous values and set default values */
        Any_strcpy( bufferType, "STRUCT=AS_LIST" );
        Any_strcpy( bufferArray, "ARRAY=ARRAY_AS_LIST" );
        Any_strcpy( bufferStructArray, "STRUCTARRAY=STRUCTARRAY_AS_LIST" );
    }

    if( optionsString )
    {
        /* Parse format options string */
        while(( offset < len ) &&
              ( Any_sscanf( optionsString + offset, "%511[^'=']", bufferOption ) != -1 ))
        {
            offset += Any_strlen( bufferOption ) + 1;
            Any_sscanf( optionsString + offset, "%511s", bufferValue );
            ANY_LOG( 10, "Format: %s %s", ANY_LOG_INFO, bufferOption, bufferValue );

            offset += Any_strlen( bufferValue ) + 1;

            /* Check option-value couple: */

            /* Check type */
            if( !Any_strcmp( bufferOption, "STRUCT" ))
            {

                if( SERIALIZE_TYPE_VALID( bufferValue ))
                {
                    Any_strcpy( bufferType, "STRUCT=" );
                    Any_strcat( bufferType, bufferValue );
                }
                else
                {
                    ANY_LOG( 0, "Warning: unknown structure type [%s]", ANY_LOG_WARNING, bufferValue );
                    break;
                }

                while( true )
                {
                    if( SERIALIZE_CHECK_OPTION( bufferValue, AS_TUPLE ))
                    {
                        data->type = AS_TUPLE;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, AS_LIST ))
                    {
                        data->type = AS_LIST;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, AS_DICT ))
                    {
                        data->type = AS_DICT;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, AS_TUPLE_NO_KEY ))
                    {
                        data->type = AS_TUPLE_NO_KEY;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, AS_LIST_NO_KEY ))
                    {
                        data->type = AS_LIST_NO_KEY;
                        break;
                    }
                }
            }

                /* Check array type */
            else if( !Any_strcmp( bufferOption, "ARRAY" ))
            {

                if( SERIALIZE_ARRAY_VALID( bufferValue ))
                {
                    Any_strncpy( bufferArray, "ARRAY=", SERIALIZE_DATABUFFER_MAXLEN - sizeof( "ARRAY=" ) - 1 );
                    Any_strncat( bufferArray, bufferValue, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "ARRAY=" ) - 1 );
                }
                else
                {
                    ANY_LOG( 0, "Warning: unknown array type [%s]", ANY_LOG_WARNING, bufferValue );
                    break;
                }


                while( true )
                {
                    if( SERIALIZE_CHECK_OPTION( bufferValue, ARRAY_AS_TUPLE ))
                    {
                        data->arrayType = ARRAY_AS_TUPLE;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, ARRAY_AS_LIST ))
                    {
                        data->arrayType = ARRAY_AS_LIST;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, ARRAY_AS_DICT ))
                    {
                        data->arrayType = ARRAY_AS_DICT;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue,
                                                ARRAY_AS_TUPLE_NO_INDEX ))
                    {
                        data->arrayType = ARRAY_AS_TUPLE_NO_INDEX;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue,
                                                ARRAY_AS_LIST_NO_INDEX ))
                    {
                        data->arrayType = ARRAY_AS_LIST_NO_INDEX;
                        break;
                    }
                }
            }

                /* Check struct array */
            else if( !Any_strcmp( bufferOption, "STRUCTARRAY" ))
            {

                if( SERIALIZE_STRUCTARRAY_VALID( bufferValue ))
                {
                    Any_strncpy( bufferStructArray, "STRUCTARRAY=",
                                 SERIALIZE_DATABUFFER_MAXLEN - sizeof( "STRUCTARRAY=" ));

                    Any_strncat( bufferStructArray, bufferValue,
                                 SERIALIZE_DATABUFFER_MAXLEN - sizeof( "STRUCTARRAY=" ));
                }
                else
                {
                    ANY_LOG( 0, "Warning: unknown struct array type [%s]", ANY_LOG_WARNING, bufferValue );
                    break;
                }

                while( true )
                {
                    if( SERIALIZE_CHECK_OPTION( bufferValue, STRUCTARRAY_AS_TUPLE ))
                    {
                        data->structArrayType = STRUCTARRAY_AS_TUPLE;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, STRUCTARRAY_AS_LIST ))
                    {
                        data->structArrayType = STRUCTARRAY_AS_LIST;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue, STRUCTARRAY_AS_DICT ))
                    {
                        data->structArrayType = STRUCTARRAY_AS_DICT;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue,
                                                STRUCTARRAY_AS_TUPLE_NO_INDEX ))
                    {
                        data->structArrayType = STRUCTARRAY_AS_TUPLE_NO_INDEX;
                        break;
                    }
                    if( SERIALIZE_CHECK_OPTION( bufferValue,
                                                STRUCTARRAY_AS_LIST_NO_INDEX ))
                    {
                        data->structArrayType = STRUCTARRAY_AS_LIST_NO_INDEX;
                        break;
                    }
                }
            }

            else
            {
                ANY_LOG( 0, "Warning: unknown option [%s]",
                         ANY_LOG_WARNING, bufferOption );
            }

        }
    }

    /* Save format options string */
    charPtr = Serialize_getHeaderOptsPtr( self );
    Any_sprintf( charPtr, "%s %s %s", bufferType, bufferArray, bufferStructArray );

}


static bool SerializeFormatPythonOptions_setProperty( Serialize *self, \
                                                     const char *propertyName, \
                                                     void *propertyValue )
{
    /* Not yet implemented */
    return false;
}


static void *SerializeFormatPythonOptions_getProperty( Serialize *self, \
                                                       const char *propertyName )
{
    /* Not yet implemented */
    return false;
}


static void SerializeFormatPythonOptions_clear( Serialize *self )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;

    ANY_REQUIRE( self );
    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    Any_memset( data, '0', sizeof( SerializeFormatPythonOptions ));

}


static void SerializeFormatPythonOptions_delete( Serialize *self )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;

    ANY_REQUIRE( self );
    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    ANY_FREE( data );
}


static void SerializeFormatPython_getTypeInfo( SerializeType type,
                                               char *spec )
{
#define TYPEINFO( __specPtr, __spec )\
  Any_sprintf( __specPtr, "%s", __spec )

    switch( type )
    {
        case SERIALIZE_TYPE_CHAR:
        case SERIALIZE_TYPE_CHARARRAY:
            TYPEINFO( spec, "%d" );
            break;
        case SERIALIZE_TYPE_SCHAR:
        case SERIALIZE_TYPE_SCHARARRAY:
            TYPEINFO( spec, "%d" );
            break;
        case SERIALIZE_TYPE_UCHAR:
        case SERIALIZE_TYPE_UCHARARRAY:
            TYPEINFO( spec, "%u" );
            break;
        case SERIALIZE_TYPE_SINT:
        case SERIALIZE_TYPE_SINTARRAY:
            TYPEINFO( spec, "%hd" );
            break;
        case SERIALIZE_TYPE_USINT:
        case SERIALIZE_TYPE_USINTARRAY:
            TYPEINFO( spec, "%hu" );
            break;
        case SERIALIZE_TYPE_INT:
        case SERIALIZE_TYPE_INTARRAY:
            TYPEINFO( spec, "%d" );
            break;
        case SERIALIZE_TYPE_UINT:
        case SERIALIZE_TYPE_UINTARRAY:
            TYPEINFO( spec, "%u" );
            break;
        case SERIALIZE_TYPE_LINT:
        case SERIALIZE_TYPE_LINTARRAY:
            TYPEINFO( spec, "%ld" );
            break;
        case SERIALIZE_TYPE_ULINT:
        case SERIALIZE_TYPE_ULINTARRAY:
            TYPEINFO( spec, "%lu" );
            break;
        case SERIALIZE_TYPE_LL:
        case SERIALIZE_TYPE_LLARRAY:
            TYPEINFO( spec, "%lld" );
            break;
        case SERIALIZE_TYPE_ULL:
        case SERIALIZE_TYPE_ULLARRAY:
            TYPEINFO( spec, "%llu" );
            break;
        case SERIALIZE_TYPE_FLOAT:
        case SERIALIZE_TYPE_FLOATARRAY:
            TYPEINFO( spec, "%f" );
            break;
        case SERIALIZE_TYPE_DOUBLE:
        case SERIALIZE_TYPE_DOUBLEARRAY:
            TYPEINFO( spec, "%lf" );
            break;
        case SERIALIZE_TYPE_LDOUBLE:
        case SERIALIZE_TYPE_LDOUBLEARRAY:
            TYPEINFO( spec, "%LF" );

        case SERIALIZE_TYPE_STRING :
            Any_sprintf( spec, "%s", "%qs" );
            break;
        default:
            ANY_LOG( 5,
                     "SerializeFormatPython_getTypeInfo. Unknown SerializeType : %d",
                     ANY_LOG_ERROR, type );
            ANY_REQUIRE( NULL );
            break;
    }

#undef TYPEINFO
}


static void SerializeFormatPython_doSerializeField( Serialize *self, \
                                                    SerializeType type, \
                                                    const void *name, \
                                                    void *value, \
                                                    const int size, \
                                                    bool isArrayElem )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char bufferName[SERIALIZE_DATABUFFER_MAXLEN];
    int tmpKey = 0;

    char *tmpChar;
    unsigned char *tmpUChar;
    int auxData = 0;

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    SerializeFormatPython_getTypeInfo( type, spec );

    if( Serialize_isReading( self ))
    {
        if( isArrayElem == false )
        {

            /*************** FIELD **************/

            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {
                /* Check if serialized and deserialized array indices are equal */
                Serialize_scanf( self, SERIALIZE_KEY_OPEN( data->type ), bufferName );
                ANY_REQUIRE_VMSG( !Any_strcmp( bufferName, (char *)name ), \
                          "Different serialized-deserialized indx: found %s, expected %s", \
                          (char *)name, bufferName );
            }

            /*************** END FIELD **************/

        }
        else
        {
            /***************** ARRAY ELEMENT *****************/

            if( SERIALIZE_ARRAY_HAS_INDEX( data->arrayType ))
            {
                Serialize_scanf( self, SERIALIZE_ARRAY_INDEX_OPEN( data->arrayType ), &tmpKey );

                /* Check if serialized and deserialized nested type name are equal */
                ANY_REQUIRE_VMSG( *(int *)name == tmpKey, \
                          "Different serialized-deserialized type names: %d %d", \
                          *(int *)name, tmpKey );
            }

            /***************** END ARRAY ELEMENT *****************/

        }

        /* Read the value */
        Serialize_scanf( self, spec, value );

        /* Closing operations */
        if( isArrayElem == false )
        {
            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {
                Serialize_scanf( self, SERIALIZE_KEY_CLOSE( data->type ));
            }
        }
        else
        {
            if( SERIALIZE_ARRAY_HAS_INDEX( data->arrayType ))
            {
                Serialize_scanf( self,
                                 SERIALIZE_ARRAY_INDEX_CLOSE( data->arrayType ));
            }
        }

        Serialize_scanf( self, ", " );
        if( isArrayElem == false )
        {
            Serialize_scanf( self, "\\ " );
        }

    }
    else
    {

        if( isArrayElem == false )
        {

            /*************** FIELD **************/

            SERIALIZE_INDENT( self );
            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {
                Serialize_printf( self, SERIALIZE_KEY_OPEN( data->type ), name );
            }

            /*************** END FIELD **************/
        }
        else
        {

            /***************** ARRAY ELEMENT *****************/

            if( SERIALIZE_ARRAY_HAS_INDEX( data->arrayType ))
            {
                Serialize_printf( self, SERIALIZE_ARRAY_INDEX_OPEN( data->arrayType ), (int *)name );
            }

            /***************** END ARRAY ELEMENT *****************/

        }

        /* Read the value: */

        /* Char types fix */
        if( type == SERIALIZE_TYPE_CHAR ||
            type == SERIALIZE_TYPE_SCHAR ||
            type == SERIALIZE_TYPE_CHARARRAY ||
            type == SERIALIZE_TYPE_SCHARARRAY )
        {
            tmpChar = (char *)value;
            auxData = (int)*tmpChar;
            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, &auxData );
        }
        else if( type == SERIALIZE_TYPE_UCHAR ||
                 type == SERIALIZE_TYPE_UCHARARRAY )
        {
            tmpUChar = (unsigned char *)value;
            auxData = (unsigned char)*tmpUChar;
            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, &auxData );
        }
        else
        {
            /* Non-char value */
            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, value );
        }

        /* Closing operations */
        if( isArrayElem == false )
        {
            if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
            {
                Serialize_printf( self, SERIALIZE_KEY_CLOSE( data->type ));
            }
        }
        else
        {
            if( SERIALIZE_ARRAY_HAS_INDEX( data->arrayType ))
            {
                Serialize_printf( self,
                                  SERIALIZE_ARRAY_INDEX_CLOSE( data->arrayType ));
            }
        }

        Serialize_printf( self, ", " );
        if( isArrayElem == false )
        {
            Serialize_printf( self, "\\\n" );
        }

    }

}


static void SerializeFormatPython_doSerializeString( Serialize *self, \
                                                     SerializeType type, \
                                                     const char *name, \
                                                     void *value, \
                                                     const int size, \
                                                     const int len )
{
    SerializeFormatPythonOptions *data = (SerializeFormatPythonOptions *)NULL;
    char bufferName[SERIALIZE_DATABUFFER_MAXLEN];

    data = Serialize_getFormatDataPtr( self );

    ANY_REQUIRE( self );
    ANY_REQUIRE( type );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );
    ANY_REQUIRE( data );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    if( Serialize_isReading( self ))
    {

        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_scanf( self, SERIALIZE_KEY_OPEN( data->type ), bufferName );
            ANY_REQUIRE_VMSG( !Any_strcmp( bufferName, name ), \
                        "Different serialized-deserialized type names: %s %s", \
                        name, bufferName );
        }
        Serialize_scanf( self, "%qs", (char *)value );
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_scanf( self, SERIALIZE_KEY_CLOSE( data->type ));
        }
        Serialize_scanf( self, ", \\ " );
    }
    else
    {

        SERIALIZE_INDENT( self );
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_printf( self, SERIALIZE_KEY_OPEN( data->type ), name );
        }
        Serialize_printf( self, "%qs", (char *)value );
        if( SERIALIZE_STRUCT_HAS_KEY( data->type ))
        {
            Serialize_printf( self, SERIALIZE_KEY_CLOSE( data->type ));
        }
        Serialize_printf( self, ", \\\n" );
    }

}


static void SerializeFormatPython_doSerializeArrayElement( Serialize *self, \
                                                           SerializeType type, \
                                                           const char *name, \
                                                           void *value, \
                                                           const int size, \
                                                           const int len, \
                                                           const int index )
{
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char *ptr;
    int columnWrap;

    ANY_REQUIRE( self );
    ANY_REQUIRE( size > 0 );
    ANY_REQUIRE( len > 0 );
    ANY_REQUIRE(( index >= 0 ) && (( index < len )));

    SerializeFormatPython_getTypeInfo( type, spec );

    /* Get i-th value inside array */
    ptr = value;
    ptr += ( size * index );

    ANY_REQUIRE( ptr );

    columnWrap = Serialize_getColumnWrap( self );

    if( Serialize_isReading( self ))
    {
        if(( columnWrap > 0 ) &&
           ( index % columnWrap ) == 0 )
        {
            Serialize_scanf( self, "\\ " );
        }

        SerializeFormatPython_doSerializeField( self, type,
                                                (void *)&index,
                                                (void *)ptr, size, true );
    }
    else
    {
        if(( columnWrap > 0 ) &&
           ( index % columnWrap ) == 0 )
        {
            Serialize_printf( self, "\\\n" );
            SERIALIZE_INDENT( self );
        }

        SerializeFormatPython_doSerializeField( self, type,
                                                (void *)&index,
                                                (void *)ptr, size, true );
    }

}


/*--------------------------------------------------------------------------*/
/* JSON format                                                              */
/*--------------------------------------------------------------------------*/


SERIALIZEFORMAT_CREATE_PLUGIN( Json );


typedef struct SerializeFormatJsonOptions
{
    bool withType;
    bool isFirst;
    bool beginStructArrayElem;
}
        SerializeFormatJsonOptions;


static void SerializeFormatJson_getTypeInfo( SerializeType type,
                                             char *spec,
                                             char *typeTag );

static void SerializeFormatJson_doSerializeCharType( Serialize *self,
                                                     SerializeType type,
                                                     const char *name,
                                                     void *value,
                                                     const int size,
                                                     const int len,
                                                     const int index );

static void SerializeFormatJson_doSerializeField( Serialize *self,
                                                  SerializeType type,
                                                  const char *name,
                                                  void *value,
                                                  const int size );

static void SerializeFormatJson_doSerializeString( Serialize *self,
                                                   SerializeType type,
                                                   const char *name,
                                                   void *value,
                                                   const int size,
                                                   const int len );

static void SerializeFormatJson_doSerializeArrayElement( Serialize *self,
                                                         SerializeType type,
                                                         const char *name,
                                                         void *value,
                                                         const int size,
                                                         const int len,
                                                         const int index,
                                                         bool reIndexOffset );


static void SerializeFormatJson_beginType( Serialize *self,
                                           const char *name,
                                           const char *type )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN] = "";
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    if(data->beginStructArrayElem == true)
    {
        data->beginStructArrayElem = false;
    }
    else
    {
        switch( self->mode )
        {
          case SERIALIZE_MODE_READ:
          {
              char instanceName[512] = "";

              data->isFirst = true;

              if( self->numTypeCalls == 1 )
              {
                  Any_strcat( buffer, "{\n" );
              }

              Any_strcat( buffer, "\"%s\": {" );

              /* Only pattern matching */
              Serialize_scanf( self, buffer, instanceName );

              if( Any_strcmp( name, instanceName ) != 0 )
              {
                  ANY_LOG( 0, "Expected instance name '%s' different than '%s'", ANY_LOG_WARNING,
                           name, instanceName );
              }
            }
            break;

          case SERIALIZE_MODE_WRITE:
          case SERIALIZE_MODE_CALC:
          {
              if( data->isFirst == false )
              {
                  Serialize_printf( self, ",\n" );
              }

              data->isFirst = true;

              if( self->indentLevel == 0 )
              {
                  Serialize_printf( self, "{\n" );
                  SERIALIZE_INDENT_INCR( self );
              }

              SERIALIZE_INDENT( self );

              Serialize_printf( self, "\"%s\": {\n", name );

              SERIALIZE_INDENT_INCR( self );
          }
              break;

          default:
              ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
              break;
        }
    }
}


static void SerializeFormatJson_beginBaseType( Serialize *self,
                                               const char *name,
                                               const char *type )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( type );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_beginArray( Serialize *self,
                                            SerializeType type,
                                            const char *arrayName,
                                            const int arrayLen )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( type );
    ANY_REQUIRE( arrayName );

    /* Serializing empty arrays should be no problem? (TBCORE-1059) */
    // ANY_REQUIRE( arrayLen );
    ANY_OPTIONAL( arrayLen );


    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    SerializeFormatJson_getTypeInfo( type, spec, typeTag );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( data->isFirst == false )
            {
                Serialize_printf( self, ",\n" );
            }

            data->isFirst = true;

            Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "\"\": [" ),
                          "\"%s\": [", arrayName );

            Serialize_scanf( self, buffer );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            if( data->isFirst == false )
            {
                Serialize_printf( self, ",\n" );
            }

            data->isFirst = false;

            SERIALIZE_INDENT( self );

            Serialize_printf( self, "\"%s\": [", arrayName );

            SERIALIZE_INDENT_INCR( self );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_beginStructArray( Serialize *self,
                                                  const char *arrayName,
                                                  const char *elementType,
                                                  const int arrayLen )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );
    ANY_REQUIRE( elementType );

    /* Serializing empty arrays should be no problem? (TBCORE-1059) */
    // ANY_REQUIRE( arrayLen );
    ANY_OPTIONAL( arrayLen );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "\"\": [ " ),
                          "\"%s\": [ ", arrayName );

            /* Only Pattern Matching... */
            Serialize_scanf( self, buffer );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            if( data->isFirst == false )
            {
                Serialize_printf( self, ",\n" );
            }

            data->isFirst = true;

            SERIALIZE_INDENT( self );

            Serialize_printf( self, "\"%s\": [\n", arrayName );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_beginStructArraySeparator( Serialize *self,
                                                           const char *name,
                                                           const int position,
                                                           const int len )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    data->beginStructArrayElem = true;
    data->isFirst = true;

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "{\n" );
        }
        break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            if( position != 0 )
            {
                Serialize_printf( self, ",\n" );
            }

            SERIALIZE_INDENT( self );

            Serialize_printf( self, "{\n" );

            SERIALIZE_INDENT_INCR( self );
        }
        break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_doSerialize( Serialize *self,
                                             SerializeType type,
                                             const char *name,
                                             void *value,
                                             const int size,
                                             const int len )
{
    int i = 0;
    bool isCharType = false;
    bool isField = false;
    bool isString = false;
    bool isArrayElement = false;
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    isCharType = ((( type == SERIALIZE_TYPE_CHAR ) ||
                   ( type == SERIALIZE_TYPE_UCHAR ) ||
                   ( type == SERIALIZE_TYPE_SCHAR ) ||
                   ( type == SERIALIZE_TYPE_CHARARRAY ) ||
                   ( type == SERIALIZE_TYPE_UCHARARRAY ) ||
                   ( type == SERIALIZE_TYPE_SCHARARRAY )) ? true : false );

    isString = (( type == SERIALIZE_TYPE_STRING ) ? true : false );
    isArrayElement = SERIALIZE_IS_ARRAY_ELEMENT( type );
    isField = !isArrayElement;

    for( ; ; )
    {
        /* Wraps Standard Chars */
        if( isCharType == true )
        {
            for( i = 0; i < len; i++ )
            {
                SerializeFormatJson_doSerializeCharType( self, type, name, value, size, len, i );
            }
            break;
        }

        if( isField == true )
        {
            SerializeFormatJson_doSerializeField( self, type, name, value, size );
            break;
        }

        if( isString == true )
        {
            SerializeFormatJson_doSerializeString( self, type, name, value, size, len );
            break;
        }

        if( isArrayElement == true )
        {
            for( i = 0; i < len; i++ )
            {
                SerializeFormatJson_doSerializeArrayElement( self, type, name, value, size, len, i, true );
            }
            break;
        }

        break;
    }
}


static void SerializeFormatJson_endStructArraySeparator( Serialize *self,
                                                         const char *name,
                                                         const int position,
                                                         const int len )
{
    ANY_REQUIRE( self );
}


static void SerializeFormatJson_endStructArray( Serialize *self )
{
    char cc;

    ANY_REQUIRE( self );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Serialize_scanf( self, "]" );

            cc = IOChannel_getc( self->stream );

            if( cc != ',' && cc != -1 )
            {
                IOChannel_unget( self->stream, &cc, 1 );
            }

        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            Serialize_printf( self, "\n" );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "]" );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_endArray( Serialize *self,
                                          SerializeType type,
                                          const char *arrayName,
                                          const int arrayLen )
{
    char cc;

    ANY_REQUIRE( self );
    ANY_REQUIRE( arrayName );

    /* Serializing empty arrays should be no problem? (TBCORE-1059) */
    // ANY_REQUIRE( arrayLen );
    ANY_OPTIONAL( arrayLen );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            Serialize_scanf( self, "]" );

            cc = IOChannel_getc( self->stream );

            if( cc != ',' && cc != -1 )
            {
                IOChannel_unget( self->stream, &cc, 1 );
            }

            break;

        case SERIALIZE_MODE_WRITE:
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "]" );
        case SERIALIZE_MODE_CALC:
        {
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_endBaseType( Serialize *self )
{
    ANY_REQUIRE( self );

    /* End type MUST match also the newline */
    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_endType( Serialize *self )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;
    char buffer[SERIALIZE_DATABUFFER_MAXLEN] = "";
    char cc;

    ANY_REQUIRE( self );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    /* End type MUST match also the newline */
    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( self->numTypeCalls == 1 )
            {
                Any_strcat( buffer, "}\n}" );
            }
            else
            {
                Any_strcat( buffer, "}" );
            }

            Serialize_scanf( self, buffer );

            if( self->numTypeCalls > 1 )
            {
                cc = IOChannel_getc( self->stream );

                if( cc != ',' && cc != -1 )
                {
                    IOChannel_unget( self->stream, &cc, 1 );
                }
            }
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            Serialize_printf( self, "\n" );
            SERIALIZE_INDENT_DECR( self );
            SERIALIZE_INDENT( self );
            Serialize_printf( self, "}" );

            if( self->indentLevel == 2 )
            {
                Serialize_printf( self, "\n" );
                Serialize_printf( self, "}\n" );
                SERIALIZE_INDENT_DECR( self );
                SERIALIZE_INDENT( self );
                data->isFirst = true;
            }
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static int SerializeFormatJson_getAllowedModes( Serialize *self )
{
    int modes = SERIALIZE_MODE_CALC;

    ANY_REQUIRE( self );

    return modes;
}


void *SerializeFormatJsonOptions_new( void )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    data = ANY_TALLOC( SerializeFormatJsonOptions );
    ANY_REQUIRE( data );

    return (void *)data;
}


void SerializeFormatJsonOptions_init( Serialize *self )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatJsonOptions *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    /* Default init options */
    data->withType = false;
    data->isFirst = true;
}


void SerializeFormatJsonOptions_set( Serialize *self, const char *optionsString )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;
    char *optionsPtr = (char *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatJsonOptions *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    /* Default Uses No Type */
    data->withType = false;

    if( optionsString != (char *)NULL)
    {
        /* We don't really do anything here */
    }

    optionsPtr = Serialize_getHeaderOptsPtr( self );
    ANY_REQUIRE( optionsPtr );
}


static bool SerializeFormatJsonOptions_setProperty( Serialize *self,
                                                    const char *propertyName,
                                                    void *propertyValue )
{
    bool retVal = false;

    ANY_REQUIRE( self );

    return retVal;
}


static void *SerializeFormatJsonOptions_getProperty( Serialize *self,
                                                     const char *propertyName )
{
    void *retVal = (void *)NULL;

    ANY_REQUIRE( self );

    return retVal;
}


void SerializeFormatJsonOptions_clear( Serialize *self )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );

    data = (SerializeFormatJsonOptions *)Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    Any_memset((void *)data, 0, sizeof( SerializeFormatJsonOptions ));
}


void SerializeFormatJsonOptions_delete( Serialize *self )
{
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    ANY_FREE( data );
}


static void SerializeFormatJson_getTypeInfo( SerializeType type,
                                             char *spec,
                                             char *typeTag )
{
#define TYPEINFO( __specPtr, __spec, __typeTagPtr, __typeTag )  \
  Any_sprintf( __specPtr, "%s", __spec );                       \
  Any_sprintf( __typeTagPtr, "%s", __typeTag )

    switch( type )
    {
        case SERIALIZE_TYPE_CHAR:
        case SERIALIZE_TYPE_CHARARRAY:
        TYPEINFO( spec, "%qc", typeTag, "char" );
            break;

        case SERIALIZE_TYPE_SCHAR:
        case SERIALIZE_TYPE_SCHARARRAY:
        TYPEINFO( spec, "%d", typeTag, "signed_char" );
            break;

        case SERIALIZE_TYPE_UCHAR:
        case SERIALIZE_TYPE_UCHARARRAY:
        TYPEINFO( spec, "%u", typeTag, "unsigned_char" );
            break;

        case SERIALIZE_TYPE_SINT:
        case SERIALIZE_TYPE_SINTARRAY:
        TYPEINFO( spec, "%hd", typeTag, "short_int" );
            break;

        case SERIALIZE_TYPE_USINT:
        case SERIALIZE_TYPE_USINTARRAY:
        TYPEINFO( spec, "%hu", typeTag, "short_unsigned" );
            break;

        case SERIALIZE_TYPE_INT:
        case SERIALIZE_TYPE_INTARRAY:
        TYPEINFO( spec, "%d", typeTag, "int" );
            break;

        case SERIALIZE_TYPE_UINT:
        case SERIALIZE_TYPE_UINTARRAY:
        TYPEINFO( spec, "%u", typeTag, "unsigned_int" );
            break;

        case SERIALIZE_TYPE_LINT:
        case SERIALIZE_TYPE_LINTARRAY:
        TYPEINFO( spec, "%ld", typeTag, "long_int" );
            break;

        case SERIALIZE_TYPE_ULINT:
        case SERIALIZE_TYPE_ULINTARRAY:
        TYPEINFO( spec, "%lu", typeTag, "long_unsigned_int" );
            break;

        case SERIALIZE_TYPE_LL:
        case SERIALIZE_TYPE_LLARRAY:
        TYPEINFO( spec, "%lld", typeTag, "long_long" );
            break;

        case SERIALIZE_TYPE_ULL:
        case SERIALIZE_TYPE_ULLARRAY:
        TYPEINFO( spec, "%llu", typeTag, "long_long_unsigned" );
            break;

        case SERIALIZE_TYPE_FLOAT:
        case SERIALIZE_TYPE_FLOATARRAY:
        TYPEINFO( spec, "%f", typeTag, "float" );
            break;

        case SERIALIZE_TYPE_DOUBLE:
        case SERIALIZE_TYPE_DOUBLEARRAY:
        TYPEINFO( spec, "%lf", typeTag, "double" );
            break;

        case SERIALIZE_TYPE_LDOUBLE:
        case SERIALIZE_TYPE_LDOUBLEARRAY:
        TYPEINFO( spec, "%LF", typeTag, "long_double" );
            break;

        case SERIALIZE_TYPE_STRING :
            Any_sprintf( spec, "%s", "%qs" );
            Any_sprintf( typeTag, "%s", "string" );
            break;
        default:
            ANY_LOG( 0, "SerializeFormatJson_getTypeInfo. Unknown SerializeType : %d",
                     ANY_LOG_FATAL, type );
            ANY_REQUIRE( NULL );
            break;
    }

#undef TYPEINFO
}


static void SerializeFormatJson_doSerializeCharType( Serialize *self,
                                                     SerializeType type,
                                                     const char *name,
                                                     void *value,
                                                     const int size,
                                                     const int len,
                                                     const int index )
{
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char *signedCharPtr = (char *)NULL;
    unsigned char *unsignedCharPtr = (unsigned char *)NULL;
    int auxData = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    SerializeFormatJson_getTypeInfo( type, spec, typeTag );

    if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR ||
        type == SERIALIZE_TYPE_CHARARRAY || type == SERIALIZE_TYPE_SCHARARRAY )
    {
        signedCharPtr = value;
        signedCharPtr += size * index;
        ANY_REQUIRE( signedCharPtr );

        auxData = ((int)*signedCharPtr );
    }
    else
    {
        unsignedCharPtr = value;
        unsignedCharPtr += size * index;
        ANY_REQUIRE( unsignedCharPtr );

        auxData = ((int)*unsignedCharPtr );
    }

    if( SERIALIZE_IS_ARRAY_ELEMENT( type ) == false )
    {
        SerializeFormatJson_doSerializeField( self, type, name, &auxData, size );

        if( self->mode == SERIALIZE_MODE_READ )
        {
            if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR )
            {
                ANY_REQUIRE( auxData <= SCHAR_MAX );
                ANY_REQUIRE( auxData >= SCHAR_MIN );
                *signedCharPtr = (char)auxData;
            }
            else
            {
                ANY_REQUIRE( auxData <= UCHAR_MAX );
                *unsignedCharPtr = (unsigned char)auxData;
            }
        }
    }
    else
    {
        SerializeFormatJson_doSerializeArrayElement( self, type, name, &auxData, size, len, index, false );

        if( self->mode == SERIALIZE_MODE_READ )
        {
            if( type == SERIALIZE_TYPE_CHAR || type == SERIALIZE_TYPE_SCHAR ||
                type == SERIALIZE_TYPE_CHARARRAY || type == SERIALIZE_TYPE_SCHARARRAY )
            {
                ANY_REQUIRE( auxData <= SCHAR_MAX );
                ANY_REQUIRE( auxData >= SCHAR_MIN );
                ANY_REQUIRE( signedCharPtr );
                *signedCharPtr = (char)auxData;
            }
            else
            {
                ANY_REQUIRE( auxData <= UCHAR_MAX );
                ANY_REQUIRE( unsignedCharPtr );
                *unsignedCharPtr = (unsigned char)auxData;
            }
        }
    }
}


static void SerializeFormatJson_doSerializeField( Serialize *self,
                                                  SerializeType type,
                                                  const char *name,
                                                  void *value,
                                                  const int size )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char cc;
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    SerializeFormatJson_getTypeInfo( type, spec, typeTag );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "\"\": " ),
                          "\"%s\": %s", name, spec );

            Serialize_scanf( self, buffer, value );

            cc = IOChannel_getc( self->stream );

            if( cc != ',' && cc != -1 )
            {
                IOChannel_unget( self->stream, &cc, 1 );
            }
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            if( data->isFirst == false )
            {
                Serialize_printf( self, ",\n" );
            }

            data->isFirst = false;

            SERIALIZE_INDENT( self );

            Serialize_printf( self, "\"%s\": ", name );

            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, value );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_doSerializeString( Serialize *self,
                                                   SerializeType type,
                                                   const char *name,
                                                   void *value,
                                                   const int size,
                                                   const int len )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char cc;
    SerializeFormatJsonOptions *data = (SerializeFormatJsonOptions *)NULL;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );

    if( type != SERIALIZE_TYPE_STRING )
    {
        ANY_REQUIRE( len > 0 );
    }

    data = Serialize_getFormatDataPtr( self );
    ANY_REQUIRE( data );

    SerializeFormatJson_getTypeInfo( type, spec, typeTag );

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "\"\": " ),
                          "\"%s\": %s", name, spec );

            Serialize_scanf( self, buffer, value );

            cc = IOChannel_getc( self->stream );

            if( cc != ',' && cc != -1 )
            {
                IOChannel_unget( self->stream, &cc, 1 );
            }
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            if( data->isFirst == false )
            {
                Serialize_printf( self, ",\n" );
            }

            data->isFirst = false;

            SERIALIZE_INDENT( self );

            Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - sizeof( "\"\": " ),
                          "\"%s\": %s", name, spec );

            Serialize_printf( self, buffer, value );
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}


static void SerializeFormatJson_doSerializeArrayElement( Serialize *self,
                                                         SerializeType type,
                                                         const char *name,
                                                         void *value,
                                                         const int size,
                                                         const int len,
                                                         const int index,
                                                         bool reIndexOffset )
{
    char buffer[SERIALIZE_DATABUFFER_MAXLEN];
    char typeTag[SERIALIZE_TAGNBUFFER_MAXLEN];
    char spec[SERIALIZE_SPECBUFFER_MAXLEN];
    char *ptr = (char *)NULL;

    bool isLastArrayElement = false;

    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( value );
    ANY_REQUIRE( size > 0 );
    ANY_REQUIRE( len > 0 );

    SerializeFormatJson_getTypeInfo( type, spec, typeTag );

    isLastArrayElement = (( index == ( len - 1 )) ? true : false );

    if( reIndexOffset == true )
    {
        ptr = value;
        ptr += size * index;
        ANY_REQUIRE( ptr );
    }
    else
    {
        ptr = value;
    }

    switch( self->mode )
    {
        case SERIALIZE_MODE_READ:
        {
            if( isLastArrayElement )
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - 1, "%s", spec );
            }
            else
            {
                Any_snprintf( buffer, SERIALIZE_DATABUFFER_MAXLEN - 2, // 1 for the blank + '\0'
                              "%s, ", spec );
            }

            Serialize_scanf( self, buffer, (void *)ptr );
        }
            break;

        case SERIALIZE_MODE_WRITE:
        case SERIALIZE_MODE_CALC:
        {
            if(( self->columnWrap > 0 ) && ( index % self->columnWrap ) == 0 )
            {
                Serialize_printf( self, "\n" );
                SERIALIZE_INDENT( self );
            }

            Serialize_deployDataType( self,
                                      type,
                                      SERIALIZE_DEPLOYDATAMODE_ASCII,
                                      spec, 0, 0, (void *)ptr );

            if( isLastArrayElement )
            {
                Serialize_printf( self, "\n" );
            }
            else
            {
                Serialize_printf( self, ", " );
            }
        }
            break;

        default:
            ANY_LOG( 5, "Unknown SerializeMode! [%d]", ANY_LOG_FATAL, self->mode );
            break;
    }
}

/* EOF */
