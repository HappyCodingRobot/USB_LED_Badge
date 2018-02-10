/**
 * fonts.c
 *
 */
#include "font.h"
#include "lucida_font.h"
#include "courier_font.h"


const font_info_t *fonts[NUM_FONTS] = {
    &lucidaConsole_10pt_fontInfo,
    &courierNew_10pt_fontInfo
};
