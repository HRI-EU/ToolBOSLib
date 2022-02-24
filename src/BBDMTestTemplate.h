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


#include <stdlib.h>

#include <Any.h>
#include <MemX.h>


/*--------------------------------------------------------------------------*/
/* Main program                                                             */
/*--------------------------------------------------------------------------*/


/*!
 * \brief Define a generic test program for any BBDM
 *
 * \param "__bbdmType__" BBDM name
 * \param "__initXML__"  XML representation to be used
 *
 * This macro defines a generic test program for any BBDM.
 *
 * General code look like:
 *
 * \code
 *
 * #include <BBDMBlockUI8.h>
 * #include <BBDMTestTemplate.h>
 *
 * BBDMBlockF32_initFromXML( myBBDM, "<BBDMBlockF32 width='10' height='10'/>" );
 *
 * \endcode
 */
#define BBDMTESTTEMPLATE_CODE_XML( __bbdmType__, __initXML__ )                          \
int main( int argc, char *argv[] )                                                      \
{                                                                                       \
  (void) argv[argc-1];                                                                  \
                                                                                        \
  __bbdmType__  *outBBDM        = __bbdmType__##_new();                                 \
  __bbdmType__  *inBBDM         = __bbdmType__##_new();                                 \
  __bbdmType__  *cBBDM          = __bbdmType__##_new();                                 \
  __bbdmType__  *fooBBDM        = __bbdmType__##_new();                                 \
  IOChannel     *calcChannel    = IOChannel_new();                                      \
  IOChannel     *outChannel     = IOChannel_new();                                      \
  IOChannel     *inChannel      = IOChannel_new();                                      \
  IOChannel     *conChannel     = IOChannel_new();                                      \
  Serialize     *calcSerializer = Serialize_new();                                      \
  Serialize     *outSerializer  = Serialize_new();                                      \
  Serialize     *inSerializer   = Serialize_new();                                      \
  Serialize     *conSerializer  = Serialize_new();                                      \
  unsigned int   seed           = (unsigned int)Any_time();                             \
  BBDMProperties outProp;                                                               \
  BBDMProperties inProp;                                                                \
  BBDMProperties cProp;                                                                 \
  BBDMProperties fooProp;                                                               \
                                                                                        \
  Any_setDebugLevel( 5 );                                                               \
                                                                                        \
  IOChannel_init( calcChannel );                                                        \
  IOChannel_init( outChannel );                                                         \
  IOChannel_init( inChannel );                                                          \
  IOChannel_init( conChannel );                                                         \
                                                                                        \
  IOChannel_open( calcChannel,                                                          \
                  "Null://",                                                            \
                  IOCHANNEL_MODE_W_ONLY,                                                \
                  IOCHANNEL_PERMISSIONS_ALL );                                          \
                                                                                        \
  IOChannel_open( outChannel,                                                           \
                  "File://testData~",                                                   \
                  IOCHANNEL_MODE_W_ONLY | IOCHANNEL_MODE_CREAT | IOCHANNEL_MODE_TRUNC,  \
                  IOCHANNEL_PERMISSIONS_W_U | IOCHANNEL_PERMISSIONS_R_U );              \
                                                                                        \
  IOChannel_open( inChannel,                                                            \
                  "File://testData~",                                                   \
                  IOCHANNEL_MODE_R_ONLY,                                                \
                  IOCHANNEL_PERMISSIONS_ALL );                                          \
                                                                                        \
  IOChannel_open( conChannel,                                                           \
                  "StdOut://",                                                          \
                  IOCHANNEL_MODE_W_ONLY,                                                \
                  IOCHANNEL_PERMISSIONS_ALL );                                          \
                                                                                        \
  Serialize_init( calcSerializer,                                                       \
                  calcChannel,                                                          \
                  SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_WRITE );                 \
                                                                                        \
  Serialize_init( outSerializer,                                                        \
                  outChannel,                                                           \
                  SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_WRITE );                 \
                                                                                        \
  Serialize_init( inSerializer,                                                         \
                  inChannel,                                                            \
                  SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_READ );                  \
                                                                                        \
  Serialize_init( conSerializer,                                                        \
                  conChannel,                                                           \
                  SERIALIZE_STREAMMODE_NORMAL | SERIALIZE_MODE_WRITE );                 \
                                                                                        \
  Serialize_setFormat( calcSerializer, "Json", NULL );                                  \
  Serialize_setFormat( outSerializer,  "Json", NULL );                                  \
  Serialize_setFormat( inSerializer,   "Json", NULL );                                  \
  Serialize_setFormat( conSerializer,  "Json", NULL );                                  \
                                                                                        \
  /* generate test data */                                                              \
  ANY_LOG( 0, "initXML=%s", ANY_LOG_INFO, __initXML__ );                                \
  __bbdmType__##_initFromXML( outBBDM, (char*)__initXML__ );                            \
  __bbdmType__##_rand( outBBDM, 50, 200, &seed );                                       \
  __bbdmType__##_setTimestep( outBBDM, 67890 );                                         \
                                                                                        \
  /* calculate the size of the serialized output */                                     \
  __bbdmType__##_serialize( outBBDM, (char*)"data", calcSerializer );                   \
  ANY_LOG( 0, "serialized size: %ld", ANY_LOG_INFO,                                     \
           IOChannel_getWrittenBytes( calcChannel ) );                                  \
                                                                                        \
  /* write test data to outputfile */                                                   \
  __bbdmType__##_indirectSerialize( outBBDM, (char*)"data", outSerializer );            \
                                                                                        \
  /* read data into second BBDM (initialize on-the-fly from serialized data) */         \
  Serialize_setInitMode( inSerializer, true );                                          \
  __bbdmType__##_indirectSerialize( inBBDM, (char*)"data", inSerializer );              \
                                                                                        \
  /* dump read-in data to console */                                                    \
  __bbdmType__##_serialize( inBBDM, (char*)"data", conSerializer );                     \
                                                                                        \
  /* rewind the channel before calling the BBDM in initMode */                          \
  IOChannel_rewind( inChannel );                                                        \
                                                                                        \
  /* set the Serialize in Init Mode so that the BBDM is created on the fly */           \
  Serialize_setInitMode( inSerializer, true );                                          \
  __bbdmType__##_indirectSerialize( cBBDM, (char*)"data", inSerializer );               \
                                                                                        \
  /* dump read-in data to console */                                                    \
  __bbdmType__##_serialize( cBBDM, (char*)"data", conSerializer );                      \
                                                                                        \
  /* print meta-information onto console */                                             \
  __bbdmType__##_getProperties( cBBDM, &cProp );                                        \
                                                                                        \
  /* verify that meta-data are equal */                                                 \
  __bbdmType__##_getProperties( outBBDM, &outProp );                                    \
  __bbdmType__##_getProperties( inBBDM, &inProp );                                      \
                                                                                        \
  /* initialize yet another BBDM from properties */                                     \
  __bbdmType__##_initFromProperties( fooBBDM, &cProp );                                 \
  __bbdmType__##_getProperties( fooBBDM, &fooProp );                                    \
                                                                                        \
  ANY_REQUIRE( BBDMProperties_isEQ( &cProp, &outProp ) );                               \
  ANY_REQUIRE( BBDMProperties_isEQ( &cProp, &inProp  ) );                               \
  ANY_REQUIRE( BBDMProperties_isEQ( &cProp, &fooProp  ) );                              \
                                                                                        \
  ANY_TRACE( 3, "%d", fooProp.width );                                                  \
  ANY_TRACE( 3, "%d", fooProp.height );                                                 \
  ANY_TRACE( 3, "%d", fooProp.length );                                                 \
  ANY_TRACE( 3, "%d", fooProp.maxNoSparseEntries );                                     \
  ANY_TRACE( 3, "%d", fooProp.size1 );                                                  \
  ANY_TRACE( 3, "%d", fooProp.size2 );                                                  \
  ANY_TRACE( 3, "%d", fooProp.size3 );                                                  \
  ANY_TRACE( 3, "%d", fooProp.size4 );                                                  \
  ANY_TRACE( 3, "%d", fooProp.type.scalar );                                            \
  ANY_TRACE( 3, "%d", fooProp.type.compound );                                          \
  ANY_TRACE( 3, "%d", fooProp.type.bplType );                                           \
  ANY_TRACE( 3, "%d", fooProp.type.bplArray );                                          \
  ANY_TRACE( 3, "%d", fooProp.type.bplBlock );                                          \
  ANY_TRACE( 3, "%d", fooProp.type.memType );                                           \
  ANY_TRACE( 3, "%d", fooProp.id );                                                     \
                                                                                        \
  Serialize_clear( calcSerializer );                                                    \
  Serialize_clear( outSerializer );                                                     \
  Serialize_clear( inSerializer );                                                      \
  Serialize_clear( conSerializer );                                                     \
                                                                                        \
  Serialize_delete( calcSerializer );                                                   \
  Serialize_delete( outSerializer );                                                    \
  Serialize_delete( inSerializer );                                                     \
  Serialize_delete( conSerializer );                                                    \
                                                                                        \
  IOChannel_close( calcChannel );                                                       \
  IOChannel_close( outChannel );                                                        \
  IOChannel_close( inChannel );                                                         \
  IOChannel_close( conChannel );                                                        \
                                                                                        \
  IOChannel_clear( calcChannel );                                                       \
  IOChannel_clear( outChannel );                                                        \
  IOChannel_clear( inChannel );                                                         \
  IOChannel_clear( conChannel );                                                        \
                                                                                        \
  IOChannel_delete( calcChannel );                                                      \
  IOChannel_delete( outChannel );                                                       \
  IOChannel_delete( inChannel );                                                        \
  IOChannel_delete( conChannel );                                                       \
                                                                                        \
  __bbdmType__##_clear( outBBDM );                                                      \
  __bbdmType__##_clear( inBBDM );                                                       \
  __bbdmType__##_clear( cBBDM );                                                        \
  __bbdmType__##_clear( fooBBDM );                                                      \
                                                                                        \
  __bbdmType__##_delete( outBBDM );                                                     \
  __bbdmType__##_delete( inBBDM );                                                      \
  __bbdmType__##_delete( cBBDM );                                                       \
  __bbdmType__##_delete( fooBBDM );                                                     \
                                                                                        \
  return EXIT_SUCCESS;                                                                  \
}


#define BBDMTESTTEMPLATE_COPYDATAFUNC_CODE_XML( __bbdmType__, __initXML__ )             \
int main( int argc, char *argv[] )                                                      \
{                                                                                       \
  (void) argv[argc-1];                                                                  \
                                                                                        \
  __bbdmType__ *src = (__bbdmType__*)NULL;                                              \
  __bbdmType__ *dst = (__bbdmType__*)NULL;                                              \
                                                                                        \
  src = __bbdmType__##_new();                                                           \
  dst = __bbdmType__##_new();                                                           \
                                                                                        \
  __bbdmType__##_initFromXML( src, (char*)__initXML__ );                                \
  __bbdmType__##_initFromXML( dst, (char*)__initXML__ );                                \
                                                                                        \
  __bbdmType__##_copyData( dst, src );                                                  \
                                                                                        \
  __bbdmType__##_clear( src );                                                          \
  __bbdmType__##_clear( dst );                                                          \
                                                                                        \
  __bbdmType__##_delete( src );                                                         \
  __bbdmType__##_delete( dst );                                                         \
                                                                                        \
  return EXIT_SUCCESS;                                                                  \
}


#define BBDMTESTTEMPLATE_INDIRECTCOPYDATAFUNC_CODE_XML( __bbdmType__, __initXML__ )     \
int main( int argc, char *argv[] )                                                      \
{                                                                                       \
  (void) argv[argc-1];                                                                  \
                                                                                        \
  __bbdmType__ *src = (__bbdmType__*)NULL;                                              \
  __bbdmType__ *dst = (__bbdmType__*)NULL;                                              \
                                                                                        \
  src = __bbdmType__##_new();                                                           \
  dst = __bbdmType__##_new();                                                           \
                                                                                        \
  __bbdmType__##_initFromXML( src, (char*)__initXML__ );                                \
  __bbdmType__##_initFromXML( dst, (char*)__initXML__ );                                \
                                                                                        \
  BBDM_copyData( dst, src );                                                            \
                                                                                        \
  __bbdmType__##_clear( src );                                                          \
  __bbdmType__##_clear( dst );                                                          \
                                                                                        \
  __bbdmType__##_delete( src );                                                         \
  __bbdmType__##_delete( dst );                                                         \
                                                                                        \
  return EXIT_SUCCESS;                                                                  \
}


#define BBDMTESTTEMPLATE_INSTANCENAME_CODE_XML( __bbdmType__, __initXML__ )             \
int main( int argc, char *argv[] )                                                      \
{                                                                                       \
  (void) argv[argc-1];                                                                  \
                                                                                        \
   __bbdmType__ *myBBDM = ( __bbdmType__*)NULL;                                         \
  myBBDM =  __bbdmType__##_new();                                                       \
   __bbdmType__##_initFromXML( myBBDM, (char*)__initXML__ );                            \
                                                                                        \
   __bbdmType__##_setInstanceName( myBBDM, "myBBDM" );                                  \
  ANY_TRACE( 0, "%s",  __bbdmType__##_getInstanceName( myBBDM ) );                      \
                                                                                        \
   __bbdmType__##_indirectGetDataElement( myBBDM, -1 );                                 \
                                                                                        \
   __bbdmType__##_clear( myBBDM );                                                      \
   __bbdmType__##_delete( myBBDM );                                                     \
                                                                                        \
  return EXIT_SUCCESS;                                                                  \
}


#define BBDMTESTTEMPLATE_GETTIMESTEPFUNC_CODE_XML( __bbdmType__, __initXML__ )          \
int main( int argc, char *argv[] )                                                      \
{                                                                                       \
  (void) argv[argc-1];                                                                  \
                                                                                        \
  signed long int result = 0;                                                           \
   __bbdmType__ *myBBDM = ( __bbdmType__*)NULL;                                         \
                                                                                        \
  myBBDM =  __bbdmType__##_new();                                                       \
   __bbdmType__##_initFromXML( myBBDM, (char*)__initXML__ );                            \
                                                                                        \
   __bbdmType__##_setTimestep( myBBDM, 12345 );                                         \
  result = __bbdmType__##_indirectGetTimestep( myBBDM );                                \
                                                                                        \
  printf( "Timestep: %ld\n", result );                                                  \
                                                                                        \
   __bbdmType__##_clear( myBBDM );                                                      \
   __bbdmType__##_delete( myBBDM );                                                     \
                                                                                        \
  return ( result == 12345 ? EXIT_SUCCESS : EXIT_FAILURE );                             \
}


/* EOF */
