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


/*---------------------------------------------------------------------------*/
/* Include files                                                             */
/*---------------------------------------------------------------------------*/

#include <SerializeReferenceValue.h>

/*---------------------------------------------------------------------------*/
/* Public functions                                                          */
/*---------------------------------------------------------------------------*/

SerializeReferenceValue *SerializeReferenceValue_new()
{
    SerializeReferenceValue *self = (SerializeReferenceValue *)NULL;

    self = ANY_TALLOC( SerializeReferenceValue );
    ANY_REQUIRE( self );

    return self;
}


void SerializeReferenceValue_init( SerializeReferenceValue *self,
                                   const char *reference,
                                   const char *value )
{
    long referenceLen = 0;
    long valueLen = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( reference );

    referenceLen = Any_strlen( reference );

    /* Allocate size of string + 1 extra byte to account for
     * terminating '\0', then copy the string */
    self->reference = (char *)ANY_BALLOC( referenceLen + 1 );
    ANY_REQUIRE( self->reference );
    Any_strncpy( self->reference, reference, referenceLen );

    self->referenceLen = referenceLen;

    if( value )
    {
        valueLen = Any_strlen( value );

        self->value = (char *)ANY_BALLOC( valueLen + 1 );
        ANY_REQUIRE( self->value );
        Any_strncpy( self->value, value, valueLen );

    }
    else
    {
        valueLen = SERIALIZEREFERENCEVALUE_DEFAULT_VALUE_SIZE;

        self->value = (char *)ANY_BALLOC( valueLen + 1 );
        ANY_REQUIRE( self->value );
    }

    self->valueLen = valueLen;
}


void SerializeReferenceValue_update( SerializeReferenceValue *self, char *reference, char *value )
{
    long referenceLen = 0;
    long valueLen = 0;

    ANY_REQUIRE( self );
    ANY_REQUIRE( reference );
    ANY_REQUIRE( value );

    referenceLen = Any_strlen( reference );
    valueLen = Any_strlen( value );

    if( self->referenceLen < referenceLen )
    {
        /* Previously allocated memory is not enough, free it (if it
         * was allocated at all) and allocate a new block big enough
         * for our new needs */
        if( self->reference != NULL)
        {
            ANY_FREE( self->reference );
        }

        self->reference = (char *)ANY_BALLOC( referenceLen + 1 );
        ANY_REQUIRE( self->reference );
    }

    if( self->valueLen < valueLen )
    {
        if( self->value != NULL)
        {
            ANY_FREE( self->value );
        }
        self->value = (char *)ANY_BALLOC( valueLen + 1 );
        ANY_REQUIRE( self->value );
    }

    SerializeReferenceValue_reset( self );

    Any_strncpy( self->reference, reference, referenceLen + 1 );
    self->referenceLen = referenceLen;

    Any_strncpy( self->value, value, valueLen + 1 );
    self->valueLen = valueLen;
}


void SerializeReferenceValue_getRVP( SerializeReferenceValue **list, SerializeReferenceValue **cache,
                                     SerializeReferenceValue **listTail, char *headerString )
{
    SerializeReferenceValue *rvp = (SerializeReferenceValue *)NULL;
    char *referenceBuffer = (char *)NULL;
    char *valueBuffer = (char *)NULL;
    char *tmp = (char *)NULL;

    ANY_REQUIRE( list );
    /* Cache can be NULL */
    ANY_REQUIRE( listTail );
    ANY_REQUIRE( headerString );

    while( *headerString != SERIALIZEREFERENCEVALUE_EOF)
    {
        /* Skip leading whitespace */
        SERIALIZEREFERENCEVALUE_SKIPSPACES( headerString );

        if( *headerString == SERIALIZEREFERENCEVALUE_EOF)
        {
            /* The string ended, quit */
            break;
        }

        if( SERIALIZEREFERENCEVALUE_ISADMITTEDREFERENCE( *headerString ) == false )
        {
            ANY_LOG( 5, "Warning! Probable error in string format. Found unadmitted '%c' character.",
                     ANY_LOG_WARNING, *headerString );
            break;
        }
        else
        {
            /* This branch parses a string in the format "reference =
             * value" and extracts both 'reference' and 'value'*/

            /* Extract reference */
            SERIALIZEREFERENCEVALUE_GETTOKEN( headerString,
                                              referenceBuffer,
                                              SERIALIZEREFERENCEVALUE_ISADMITTEDREFERENCE );

            /* Skip whitespaces */
            SERIALIZEREFERENCEVALUE_SKIPSPACES( headerString );

            /* Skip '=' */
            if( *headerString != '=' )
            {
                ANY_LOG( 5, "Error in headerString. Expected '=', found '%c'.",
                         ANY_LOG_WARNING, *headerString );
                ANY_FREE( referenceBuffer );
                referenceBuffer = NULL;
                break;
            }
            else
            {
                headerString++;
            }

            /* Skip whitespaces */
            SERIALIZEREFERENCEVALUE_SKIPSPACES( headerString );

            /* Check for EOF */
            if( *headerString == SERIALIZEREFERENCEVALUE_EOF)
            {
                ANY_LOG( 0, "EOF found while parsing the string. This is an error.", ANY_LOG_ERROR );
                ANY_FREE( referenceBuffer );
                referenceBuffer = NULL;
                break;
            }

            if( *headerString == '\'' )         /* value is surrounded by
                                           * single quotes */
            {
                /* Store current position in string */
                tmp = ++headerString;
                /* Advance the string until closing single quote or EOF is found */
                while( *headerString != SERIALIZEREFERENCEVALUE_EOF && *headerString != '\'' )
                {
                    headerString++;
                }

                /* EOF was found advancing the string, quit */
                if( *headerString != '\'' )
                {
                    ANY_LOG( 5, "Expected \"'\" but never found. Reference \"%s\".",
                             ANY_LOG_WARNING, referenceBuffer );
                    ANY_FREE( referenceBuffer );
                    referenceBuffer = NULL;
                    break;
                }
                else
                {
                    /* Everything looks fine, extract the string surrounded by quotes */
                    SERIALIZEREFERENCEVALUE_EXTRACTTOKEN( tmp,
                                                          valueBuffer,
                                                          ( headerString - tmp ));
                    headerString++;
                }
            }
            else                      /* value is not surrounded by quotes */
            {
                SERIALIZEREFERENCEVALUE_GETTOKEN( headerString,
                                                  valueBuffer,
                                                  SERIALIZEREFERENCEVALUE_ISADMITTEDREFERENCE );
            }

            /* Look for element in list that contains the reference we just
             * read and update its value */
            rvp = SerializeReferenceValue_findReferenceValue( *list, referenceBuffer );

            if( !rvp )
            {
                /* We didn't find a reference */

                /* Let's try to take an element from the cache pool and use
                 * that one */
                if( cache != NULL)
                {
                    rvp = SerializeReferenceValue_pop( cache );
                }

                if( !rvp )
                {
                    /* We didn't find any elements in the pool - maybe it's
                     * empty. Create a new one as last option. */
                    rvp = SerializeReferenceValue_new();
                    SerializeReferenceValue_init( rvp, referenceBuffer, valueBuffer );
                }
            }

            SerializeReferenceValue_update( rvp, referenceBuffer, valueBuffer );
            /* SerializeReferenceValue_append( *list, listTail, rvp ); */
            SerializeReferenceValue_push( list, rvp );

            rvp = NULL;
        }

        ANY_FREE( referenceBuffer );
        ANY_FREE( valueBuffer );
    }
}


SerializeReferenceValue *SerializeReferenceValue_findReferenceValue( SerializeReferenceValue *list, char *ref )
{
    SerializeReferenceValue *current = (SerializeReferenceValue *)NULL;
    long refLen = 0;

    ANY_REQUIRE( ref );

    refLen = Any_strlen( ref );

    current = list;

    while( current != NULL)
    {
        if( Any_strncmp( current->reference, ref, refLen ) == 0 )
        {
            return current;
        }
        current = current->next;
    }

    return current;
}


char *SerializeReferenceValue_findValue( SerializeReferenceValue *list, char *ref )
{
    SerializeReferenceValue *current = (SerializeReferenceValue *)NULL;
    long refLen = 0;

    ANY_REQUIRE( list );
    ANY_REQUIRE( ref );

    refLen = Any_strlen( ref );

    current = list;

    while( current != NULL)
    {
        if( Any_strncmp( current->reference, ref, refLen ) == 0 )
        {
            return current->value;
        }
        current = current->next;
    }

    ANY_LOG( 5, "Reference %s could not be found in the list.", ANY_LOG_WARNING, ref );
    return NULL;
}


char *SerializeReferenceValue_getValue( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    return self->value;
}


char *SerializeReferenceValue_getReference( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    return self->reference;
}


long SerializeReferenceValue_getValueLen( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    return self->valueLen;
}


long SerializeReferenceValue_getReferenceLen( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    return self->referenceLen;
}


SerializeReferenceValue *SerializeReferenceValue_getNext( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    return self->next;
}


/* Append `first` on top of `second` */
void SerializeReferenceValue_join( SerializeReferenceValue **first, SerializeReferenceValue **second,
                                   SerializeReferenceValue **tail )
{
    SerializeReferenceValue *originalHead = (SerializeReferenceValue *)NULL;

    ANY_REQUIRE( first );
    ANY_REQUIRE( second );
    ANY_REQUIRE( tail );

    originalHead = ( *second )->next;

    if(( *first )->next != NULL)
    {
        ( *second )->next = ( *first )->next;

        ( *tail )->next = originalHead;
    }
}


void SerializeReferenceValue_append( SerializeReferenceValue **head, SerializeReferenceValue *newElement )
{
    SerializeReferenceValue *current = (SerializeReferenceValue *)NULL;

    ANY_REQUIRE( head );
    ANY_REQUIRE( newElement );

    if(( *head ) != NULL)
    {
        current = ( *head );

        while( current->next != NULL)
        {
            current = current->next;
        }

        current->next = newElement;
    }
    else
    {
        ( *head ) = newElement;
    }
}


void SerializeReferenceValue_push( SerializeReferenceValue **self, SerializeReferenceValue *item )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( item );

    ANY_REQUIRE( item->next == NULL );

    /* Make new element point to top of the list */
    item->next = ( *self );

    /* New element is now top of the list */
    ( *self ) = item;
}


SerializeReferenceValue *SerializeReferenceValue_pop( SerializeReferenceValue **self )
{
    SerializeReferenceValue *retList = (SerializeReferenceValue *)NULL;

    ANY_REQUIRE( self );

    /* Accessing self->next when NULL gives segfault. */
    /* if( (*self)->next != NULL ) */
    if(( *self ) != NULL)
    {
        /* Get pointer to first element in list */
        retList = *self;

        /* Advance list */
        *self = ( *self )->next;

        /* This effectively severs the link between elements, making it so
         * that SerializeReferenceValue#pop return a single element, not
         * what otherwise would be a member of a list */
        retList->next = NULL;
    }

    return ( retList );
}


void SerializeReferenceValue_destroyList( SerializeReferenceValue *list )
{
    SerializeReferenceValue *current = (SerializeReferenceValue *)NULL;

    if( list != NULL)
    {
        while( list->next != NULL)
        {
            current = list;
            list = list->next;

            SerializeReferenceValue_clear( current );
            SerializeReferenceValue_delete( current );
            current = NULL;
        }

        SerializeReferenceValue_clear( list );
        SerializeReferenceValue_delete( list );
        list = NULL;
    }
}


void SerializeReferenceValue_reset( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    Any_memset( self->reference, '\0', self->referenceLen );

    Any_memset( self->value, '\0', self->valueLen );
}


void SerializeReferenceValue_clear( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    if( self->reference != NULL)
    {
        ANY_FREE( self->reference );
        self->reference = NULL;
        self->referenceLen = 0;
    }

    if( self->value != NULL)
    {
        ANY_FREE( self->value );
        self->value = NULL;
        self->valueLen = 0;
    }
}


void SerializeReferenceValue_delete( SerializeReferenceValue *self )
{
    ANY_REQUIRE( self );

    ANY_FREE( self );
}
