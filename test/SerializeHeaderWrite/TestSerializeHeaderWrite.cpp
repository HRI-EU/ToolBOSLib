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


    ANY_REQUIRE( p );
    ANY_REQUIRE( r );
    ANY_REQUIRE( channel );
    ANY_REQUIRE( serializer );


    ANY_LOG( 3, "INITIALIZATION", ANY_LOG_INFO );
    p->posX = 5;
    p->posY = 5;
    r->leftUpperCorner = *p;
    r->width = 10;
    r->height = 10;

    IOChannel_init( channel );
    status = IOChannel_open( channel, "File:///tmp/SerializeTestV20.txt", IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT,
                             IOCHANNEL_PERMISSIONS_ALL);
    ANY_REQUIRE_MSG( status, "Unable to open the IOChannel." );

    Serialize_init( serializer, channel, SERIALIZE_STREAMMODE_NORMAL );
    Serialize_setMode( serializer, SERIALIZE_MODE_WRITE );
    Serialize_setFormat( serializer, "Ascii", "WITH_TYPE=TRUE" );
    Serialize_setHeaderSizes( serializer, 5000, 0, 0, 0 );


    ANY_LOG( 3, "SERIALIZATION", ANY_LOG_INFO );
    MyRectangle_serialize( r, "myMyRectangle", serializer );


    ANY_LOG( 3, "CLEANUP", ANY_LOG_INFO );
    Serialize_clear( serializer );
    Serialize_delete( serializer );
    IOChannel_close( channel );
    IOChannel_clear( channel );
    IOChannel_delete( channel );

    ANY_FREE( r );
    ANY_FREE( p );

    ANY_LOG( 3, "DONE", ANY_LOG_INFO );

    return EXIT_SUCCESS;
}


/* EOF */
