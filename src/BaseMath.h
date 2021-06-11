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


#ifndef BASE_MATH_H
#define BASE_MATH_H


#include <BaseTypes.h>
#include <Base2DX.h>


#ifdef __cplusplus
extern "C" {
#endif


/*-------------------------------------------------------------------------*/
/* Essentials                                                              */
/*-------------------------------------------------------------------------*/


#define BASE_MATH_MAX( a, b ) (((a) >= (b)) ? (a) : (b))
#define BASE_MATH_MIN( a, b ) (((a) <= (b)) ? (a) : (b))
#define BASE_MATH_CENTER( a ) ((a) / 2)


#if defined(__msvc__)
#define BASE_MATH_ROUND( a ) ( a >= 0.0 ? floor( a + 0.5 ) : ceil( a - 0.5 ) )
#else
#define BASE_MATH_ROUND( a ) ( round((a)) )
#endif


/*-------------------------------------------------------------------------*/
/* Geometric operations                                                    */
/*-------------------------------------------------------------------------*/


/*!
 * \brief Clip point to closest coordinates inside given size.
 * \param self Pointer to result point.
 * \param src Pointer to source point.
 * \param size Pointer to size used for clipping.
 * \return Pointer to result point (self).
 */
Base2DPoint *Base2DPoint_clipToSize( Base2DPoint *self, const Base2DPoint *src,
                                     const Base2DSize *size );

/*!
 * \brief Clip point to closest coordinates inside given size (in-place).
 * \param self Pointer to source and result point.
 * \param size Pointer to size used for clipping.
 * \return Pointer to result point (self).
 */
Base2DPoint *Base2DPoint_clipToSize_I( Base2DPoint *self,
                                       const Base2DSize *size );

/*!
 * \brief Clips point to closest coordinates inside given rect.
 * \param self Pointer to result point.
 * \param src Pointer to source point.
 * \param rect Pointer to rect used for clipping.
 * \return Pointer to result point (self).
 */
Base2DPoint *Base2DPoint_clipToRect( Base2DPoint *self, const Base2DPoint *src,
                                     const Base2DRect *rect );

/*!
 * \brief Clips point to closest coordinates inside given rect (in-place).
 * \param self Pointer to source and result point.
 * \param rect Pointer to rect used for clipping.
 * \return Pointer to result point (self).
 */
Base2DPoint *Base2DPoint_clipToRect_I( Base2DPoint *self,
                                       const Base2DRect *rect );

/*!
 * \brief Checks if size is valid (width > 0 and height > 0).
 * \param self Pointer to size.
 * \return True if width > 0 and height > 0.
 */
BaseBool Base2DSize_isValid( const Base2DSize *self );

/*!
 * \brief Check if a Base2DPoint is
 *        inside of {0,0}-{self->width-1,self->height-1}
 */
BaseBool Base2DSize_isPointInside( const Base2DSize *self, const Base2DPoint *point );

/*!
 * \brief Check if self contains a given size.
 * \param self Pointer to size that should contain given size.
 * \param size Pointer to size that should be contained in self.
 * \return True if given size is not larger than self in width and height.
 */
BaseBool Base2DSize_isSizeInside( const Base2DSize *self, const Base2DSize *size );

/*!
 * \brief Check if self contains a given rect.
 * \param self Pointer to size that should contain given rect.
 * \param rect Pointer to rect that should be contained in self.
 * \return True if given rect is contained in self.
 */
BaseBool Base2DSize_isRectInside( const Base2DSize *self, const Base2DRect *rect );

/*!
 * \brief Set size invalid (width = height = 0)
 * \param self Pointer to result size.
 * \return Pointer to result size (self).
 */
Base2DSize *Base2DSize_setInvalid( Base2DSize *self );

/*!
 * \brief Build size that is union of both source sizes.
 * \param self Pointer to result size.
 * \param src Pointer to source size.
 * \param src2 Pointer to source size.
 * \return Pointer to result size (self).
 */
Base2DSize *Base2DSize_or( Base2DSize *self, const Base2DSize *src,
                           const Base2DSize *src2 );

/*!
 * \brief Build size that is union of both source sizes (in-place).
 * \param self Pointer to source and result size.
 * \param src Pointer to source size.
 * \return Pointer to result size (self).
 */
Base2DSize *Base2DSize_or_I( Base2DSize *self, const Base2DSize *src );

/*!
 * \brief Build size that is intersection of both source sizes.
 * \param self Pointer to result size.
 * \param src Pointer to source size.
 * \param src2 Pointer to source size.
 * \return Pointer to result size (self).
 */
Base2DSize *Base2DSize_and( Base2DSize *self, const Base2DSize *src,
                            const Base2DSize *src2 );

/*!
 * \brief Build size that is intersection of both source sizes (in-place).
 * \param self Pointer to source and result size.
 * \param src Pointer to source size.
 * \return Pointer to result size (self).
 */
Base2DSize *Base2DSize_and_I( Base2DSize *self, const Base2DSize *src );

/*!
 * \brief Scale size by given factor.
 * \param self Pointer to result size.
 * \param src Pointer to source size.
 * \param scale Scaling factor.
 * \return Pointer to result size (self).
 */
Base2DSize *Base2DSize_scale( Base2DSize *self, const Base2DSize *src,
                              const BaseF64 scale );

/*!
 * \brief Scale size by given factor (in-place).
 * \param self Pointer to source and result size.
 * \param scale Scaling factor.
 * \return Pointer to result size (self).
 */
Base2DSize *Base2DSize_scale_I( Base2DSize *self, const BaseF64 scale );

/*!
 * \brief Checks if rect is valid (width > 0 and height > 0).
 * \param self Pointer to rect.
 * \return True if width > 0 and height > 0.
 */
BaseBool Base2DRect_isValid( const Base2DRect *self );

/*!
 * \brief Check if a Base2DPoint is
 *        inside of {0,0}-{self->width-1,self->height-1}
 */
BaseBool Base2DRect_isPointInside( const Base2DRect *self, const Base2DPoint *point );

/*!
 * \brief Check if self contains a given rect.
 * \param self Pointer to rect that should contain given rect.
 * \param rect Pointer to rect that should be contained in self.
 * \return True if given rect is contained in self.
 */
BaseBool Base2DRect_isRectInside( const Base2DRect *self, const Base2DRect *rect );

/*!
 * \brief Set rect invalid (width = height = 0)
 * \param self Pointer to result rect.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_setInvalid( Base2DRect *self );

/*!
 * \brief Compute center point of self.
 * \param self Pointer to source rect.
 * \param point Pointer to result point.
 * \return Pointer to result point (point).
 */
Base2DPoint *Base2DRect_getCenterPoint( const Base2DRect *self,
                                        Base2DPoint *point );

/*!
 * \brief Build rect that is union of both source rects.
 * \param self Pointer to result rect.
 * \param src Pointer to source rect.
 * \param src2 Pointer to source rect.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_or( Base2DRect *self, const Base2DRect *src,
                           const Base2DRect *src2 );

/*!
 * \brief Build rect that is union of both source rects (in-place).
 * \param self Pointer to source and result rect.
 * \param src Pointer to source rect.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_or_I( Base2DRect *self, const Base2DRect *src );

/*!
 * \brief Build rect that is intersection of both source rects.
 * \param self Pointer to result rect.
 * \param src Pointer to source rect.
 * \param src2 Pointer to source rect.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_and( Base2DRect *self, const Base2DRect *src,
                            const Base2DRect *src2 );

/*!
 * \brief Build rect that is intersection of both source rects (in-place).
 * \param self Pointer to source and result rect.
 * \param src Pointer to source rect.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_and_I( Base2DRect *self, const Base2DRect *src );

/*!
 * \brief Scale all values of rect by given factor.
 * \param self Pointer to result rect.
 * \param src Pointer to source rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scale( Base2DRect *self, const Base2DRect *src,
                              const BaseF64 scale );

/*!
 * \brief Scale all values of rect by given factor (in-place).
 * \param self Pointer to source and result rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scale_I( Base2DRect *self, const BaseF64 scale );

/*!
 * \brief Scale only width of rect by given factor while keeping center.
 * \param self Pointer to result rect.
 * \param src Pointer to source rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scaleCenterWidth( Base2DRect *self,
                                         const Base2DRect *src,
                                         const BaseF64 scale );

/*!
 * \brief Scale only width of rect by given factor while keeping center (in-place).
 * \param self Pointer to source and result rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scaleCenterWidth_I( Base2DRect *self,
                                           const BaseF64 scale );

/*!
 * \brief Scale only height of rect by given factor while keeping center.
 * \param self Pointer to result rect.
 * \param src Pointer to source rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scaleCenterHeight( Base2DRect *self,
                                          const Base2DRect *src,
                                          const BaseF64 scale );

/*!
 * \brief Scale only height of rect by given factor while keeping center (in-place).
 * \param self Pointer to source and result rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scaleCenterHeight_I( Base2DRect *self,
                                            const BaseF64 scale );

/*!
 * \brief Scale only size of rect by given factor while keeping center.
 * \param self Pointer to result rect.
 * \param src Pointer to source rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scaleCenter( Base2DRect *self, const Base2DRect *src,
                                    const BaseF64 scale );

/*!
 * \brief Scale only size of rect by given factor while keeping center (in-place).
 * \param self Pointer to source and result rect.
 * \param scale Scaling factor.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_scaleCenter_I( Base2DRect *self, const BaseF64 scale );

/*!
 * \brief Crop rect to fit into given size.
 * \param self Pointer to result rect.
 * \param src Pointer to source rect.
 * \param size Pointer to source size.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_clipToSize( Base2DRect *self, const Base2DRect *src,
                                   const Base2DSize *size );

/*!
 * \brief Crop rect to fit into given size (in-place).
 * \param self Pointer to source and result rect.
 * \param size Pointer to source size.
 * \return Pointer to result rect (self).
 */
Base2DRect *Base2DRect_clipToSize_I( Base2DRect *self,
                                     const Base2DSize *size );


/*-------------------------------------------------------------------------*/
/* Byte-order conversion                                                   */
/*-------------------------------------------------------------------------*/


#define BASEI8_FLIPENDIAN( a )   (a&0xFF)

#define BASEUI8_FLIPENDIAN( a )  (a&0xFF)

#define BASEI16_FLIPENDIAN( a )  (((a&0xFF00)>>8) | ((a&0x00FF)<<8))

#define BASEUI16_FLIPENDIAN( a ) (((a&0xFF00)>>8) | ((a&0x00FF)<<8))

#define BASEBOOL_FLIPENDIAN( a ) (((a&0xFF000000)>>24) | ((a&0x00FF0000)>>8) | \
                                ((a&0x0000FF00)<<8) | ((a&0x000000FF)<<24))

#define BASECHAR_FLIPENDIAN( a ) (a)

#define BASEI32_FLIPENDIAN( a )  (((a&0xFF000000)>>24) | ((a&0x00FF0000)>>8) | \
                                ((a&0x0000FF00)<<8) | ((a&0x000000FF)<<24))

#define BASEUI32_FLIPENDIAN( a ) (((a&0xFF000000)>>24) | ((a&0x00FF0000)>>8) | \
                                ((a&0x0000FF00)<<8) | ((a&0x000000FF)<<24))

#define BASEI64_FLIPENDIAN( a )  (((a&0xFF00000000000000LL)>>56) | \
                                ((a&0x00FF000000000000LL)>>40) | \
                                ((a&0x0000FF0000000000LL)>>24) | \
                                ((a&0x000000FF00000000LL)>>8)  | \
                                ((a&0x00000000FF000000LL)<<8)  | \
                                ((a&0x0000000000FF0000LL)<<24) | \
                                ((a&0x000000000000FF00LL)<<40) | \
                                ((a&0x00000000000000FFLL)<<56))

#define BASEUI64_FLIPENDIAN( a ) (((a&0xFF00000000000000LL)>>56) | \
                                ((a&0x00FF000000000000LL)>>40) | \
                                ((a&0x0000FF0000000000LL)>>24) | \
                                ((a&0x000000FF00000000LL)>>8)  | \
                                ((a&0x00000000FF000000LL)<<8)  | \
                                ((a&0x0000000000FF0000LL)<<24) | \
                                ((a&0x000000000000FF00LL)<<40) | \
                                ((a&0x00000000000000FFLL)<<56))

BaseF32 BaseF32_flipEndian( BaseF32 a );

#define BASEF32_FLIPENDIAN( a ) (BaseF32_flipEndian(a))

BaseF64 BaseF64_flipEndian( BaseF64 a );

#define BASEF64_FLIPENDIAN( a ) (BaseF64_flipEndian(a))


#ifdef __cplusplus
}
#endif


#endif


/* EOF */
