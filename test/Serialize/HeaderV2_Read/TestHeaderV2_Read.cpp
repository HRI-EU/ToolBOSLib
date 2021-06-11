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


/* main's argc/argv are unused */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include <Serialize.h>
#include <IOChannel.h>
#include <Any.h>

typedef struct MyPoint
{
    int posX;
    int posY;
} MyPoint;

typedef struct MyRectangle
{
    MyPoint leftUpperCorner;
    unsigned int height;
    unsigned int width;
} MyRectangle;

void MyPoint_serialize( MyPoint *self, const char *name, Serialize *s );

void MyRectangle_serialize( MyRectangle *self, const char *name, Serialize *s );

void MyRectangle_toString( MyRectangle *self );


void MyPoint_serialize( MyPoint *self, const char *name, Serialize *s )
{
    Serialize_beginType( s, name, "MyPoint" );
    Int_serialize( &( self->posX ), "posX", s );
    Int_serialize( &( self->posY ), "posY", s );
    Serialize_endType( s );
}


void MyRectangle_serialize( MyRectangle *self, const char *name, Serialize *s )
{
    ANY_REQUIRE( self );
    ANY_REQUIRE( name );
    ANY_REQUIRE( s );

    Serialize_beginType( s, name,
                         "boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::p_square_quantile_impl<double,boost::accumulators::for_median>,boost::accumulators::tag::p_square_quantile_for_median>,boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::median_impl<double>,boost::accumulators::tag::median>,boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::min_impl<double>,boost::accumulators::tag::min>,boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::max_impl<double>,boost::accumulators::tag::max>,boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::density_impl<double>,boost::accumulators::tag::density>,boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::sum_impl<double,boost::accumulators::tag::sample>,boost::accumulators::tag::sum>,boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::mean_impl<double,boost::accumulators::tag::sum>,boost::accumulators::tag::mean>,boost::fusion::cons<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::variance_impl<double,boost::accumulators::tag::mean,boost::accumulators::tag::sample>,boost::accumulators::tag::variance>,boost::accumulators::detail::build_acc_list<boost::fusion::mpl_iterator<boost::mpl::v_iter<boost::mpl::vector9<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::count_impl,boost::accumulators::tag::count>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::p_square_quantile_impl<double,boost::accumulators::for_median>,boost::accumulators::tag::p_square_quantile_for_median>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::median_impl<double>,boost::accumulators::tag::median>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::min_impl<double>,boost::accumulators::tag::min>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::max_impl<double>,boost::accumulators::tag::max>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::density_impl<double>,boost::accumulators::tag::density>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::sum_impl<double,boost::accumulators::tag::sample>,boost::accumulators::tag::sum>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::mean_impl<double,boost::accumulators::tag::sum>,boost::accumulators::tag::mean>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::variance_impl<double,boost::accumulators::tag::mean,boost::accumulators::tag::sample>,boost::accumulators::tag::variance>>,9>>,boost::fusion::mpl_iterator<boost::mpl::v_iter<boost::mpl::vector9<boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::count_impl,boost::accumulators::tag::count>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::p_square_quantile_impl<double,boost::accumulators::for_median>,boost::accumulators::tag::p_square_quantile_for_median>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::median_impl<double>,boost::accumulators::tag::median>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::min_impl<double>,boost::accumulators::tag::min>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::max_impl<double>,boost::accumulators::tag::max>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::density_impl<double>,boost::accumulators::tag::density>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::sum_impl<double,boost::accumulators::tag::sample>,boost::accumulators::tag::sum>,boost::accumulators::detail::accumulator_wrapper<boost::accumulators::impl::mean_impl<double,boost::accumulators<double>>>>>" );
    MyPoint_serialize( &( self->leftUpperCorner ), "leftUpperCorner", s );
    UInt_serialize( &( self->width ), "width", s );
    UInt_serialize( &( self->height ), "height", s );
    Serialize_endType( s );
}


void MyRectangle_toString( MyRectangle *self )
{
    ANY_REQUIRE( self );

    ANY_LOG( 3, "[leftUpperCorner] X = %d Y = %d", ANY_LOG_INFO, self->leftUpperCorner.posX,
             self->leftUpperCorner.posY );
    ANY_LOG( 3, "[width] %d", ANY_LOG_INFO, self->width );
    ANY_LOG( 3, "[height] %d", ANY_LOG_INFO, self->height );
}


int main( int argc, char *argv[] )
{
    MyPoint *p = ANY_TALLOC( MyPoint );
    MyRectangle *r = ANY_TALLOC( MyRectangle );
    IOChannel *channel = IOChannel_new();
    Serialize *serializer = Serialize_new();
    bool status = false;
    char *type = (char *)NULL;
    char *name = (char *)NULL;
    char *format = (char *)NULL;
    void *ungetBuffer = NULL;
    int ungetBufferSize = 5000;
    int objSize = -1;
    char *opts = (char *)NULL;
    char *header = (char *)NULL;
    char *verbose = (char *)NULL;

    verbose = getenv((char *)"VERBOSE" );
    if( verbose != NULL && Any_strcmp( verbose, (char *)"TRUE" ) == 0 )
    {
        Any_setDebugLevel( 10 );
    }
    else
    {
        Any_setDebugLevel( 1 );
    }


    /*****************
     * Sanity checks *
     *****************/
    ANY_REQUIRE( p );
    ANY_REQUIRE( r );
    ANY_REQUIRE( channel );
    ANY_REQUIRE( serializer );

    /*******************
     * Initializations *
     *******************/
    /* Local data types */
    p->posX = 5;
    p->posY = 5;

    r->leftUpperCorner = *p;
    r->width = 10;
    r->height = 10;

    /* IOChannel */
    IOChannel_init( channel );
    status = IOChannel_open( channel, "File:///tmp/SerializeTestV20.txt", IOCHANNEL_MODE_R_ONLY,
                             IOCHANNEL_PERMISSIONS_ALL);
    ANY_REQUIRE_MSG( status == true, "Unable to open the IOChannel." );

    ungetBuffer = ANY_BALLOC( ungetBufferSize );
    IOChannel_setUngetBuffer( channel, ungetBuffer, ungetBufferSize );

    /* Serialize */
    Serialize_init( serializer, channel, SERIALIZE_STREAMMODE_NORMAL );
    Serialize_setMode( serializer, SERIALIZE_MODE_READ );
    /* Increase 'type' element size */
    Serialize_setHeaderSizes( serializer, 5000, 0, 0, 0 );

    type = (char *)ANY_BALLOC( 5000 );
    name = (char *)ANY_BALLOC( SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE );
    format = (char *)ANY_BALLOC( SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE );
    opts = (char *)ANY_BALLOC( SERIALIZE_HEADER_ELEMENT_DEFAULT_SIZE );
    header = (char *)ANY_BALLOC( SERIALIZE_HEADER_MAXLEN );

    ANY_REQUIRE( type );
    ANY_REQUIRE( name );
    ANY_REQUIRE( format );
    ANY_REQUIRE( opts );
    ANY_REQUIRE( header );

    /*********
     * Tests *
     *********/

    if( Serialize_peekHeader( serializer, type, name, &objSize, format, opts ) == false )
    {
        ANY_LOG( 3, "An error occurred while calling Serialize#peekHeader.", ANY_LOG_ERROR );
        goto cleanup;
    } else
    {
        ANY_LOG( 3, "Peeked: type = \'%s\' name = %s objSize = %d format = %s opts = \'%s\'\n", ANY_LOG_INFO, type,
                 name, objSize, format, opts );
    }

    /*****************
     * Serialization *
     *****************/
    MyRectangle_toString( r );

    MyRectangle_serialize( r, "r", serializer );

    MyRectangle_toString( r );

    /**************
     * More tests *
     **************/

    /* Increasing header element sizes, leaving 'type' default */
    Serialize_setHeaderSizes( serializer, 0, 4096, 4096, 4096 );

    IOChannel_close( channel );

    status = IOChannel_open( channel, "File:///tmp/SerializeTestV20.txt", IOCHANNEL_MODE_R_ONLY,
                             IOCHANNEL_PERMISSIONS_ALL);
    ANY_REQUIRE_MSG( status == true, "Unable to open the IOChannel." );

    MyRectangle_serialize( r, "r", serializer );

    MyRectangle_toString( r );

    if( !Serialize_getHeader( serializer, header, SERIALIZE_HEADER_MAXLEN))
    {
        ANY_LOG( 3, "Warning: could not get header.", ANY_LOG_WARNING );
    }
    else
    {
        ANY_LOG( 3, "Header: %s", ANY_LOG_INFO, header );
    }

    /***********
     * Cleanup *
     ***********/
    cleanup:
    Serialize_clear( serializer );
    Serialize_delete( serializer );

    IOChannel_close( channel );
    IOChannel_clear( channel );
    IOChannel_delete( channel );

    ANY_FREE( r );
    ANY_FREE( p );

    ANY_FREE( type );
    ANY_FREE( name );
    ANY_FREE( format );
    ANY_FREE( opts );

    ANY_FREE( header );

    ANY_FREE( ungetBuffer );

    return EXIT_SUCCESS;
}
