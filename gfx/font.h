#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED

#include <stdint.h>

//! @brief Character descriptor
typedef struct font_char_desc_s
{
    uint8_t width;      //!< Character width in pixel
    uint16_t offset;    //!< Offset of this character in bitmap
} font_char_desc_t;

//! @brief Font information
typedef struct font_info_s
{
    uint8_t height;         //!< Character height in pixel, all characters have same height
    char char_start;        //!< First character
    char char_end;          //!< Last character
    const font_char_desc_t* char_descriptors; //! descriptor for each character
    const uint8_t *bitmap;  //!< Character bitmap
} font_info_t;


#define NUM_FONTS 2    //!< Number of built-in fonts

extern const font_info_t * fonts[NUM_FONTS];  //!< Built-in fonts


#endif // FONT_H_INCLUDED
