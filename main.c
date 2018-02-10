#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <byteswap.h>
//#include <libusb-1.0/libusb.h>
//#include <hidapi/hidapi.h>
#include <getopt.h>
#include <png.h>

#include "badge.h"
#include "gfx/gfx_invader_vert.h"
#include "gfx/gfx_press_start_long.h"
// TODO: add license file

/* Prototypes */
void spaceinvaders(void);


/* all the main stuff */

// TODO: implement libpng read image
// TODO: convert to usable bitmap for badge
// use libpng to read image file
void readPNGtoMap() {
}




#ifdef DEBUG_VERBOSE
//#include <hidapi/hidapi.h>
void readpng_version_info() {
    fprintf(stderr,"Library versions:\n");
    fprintf(stderr,"   Compiled with libpng %s; using libpng %s.\n",
      PNG_LIBPNG_VER_STRING, png_libpng_ver);
    //fprintf(stderr, "   Compiled with zlib %s; using zlib %s.\n",
      //ZLIB_VERSION, zlib_version);
}
#endif // DEBUG_VERBOSE


int main(int argc, char* argv[]) {
    int i;
    (void)argc;
    (void)argv;

    //char m1[]="Hello!_";
    //char m2[]="World";
    //char m3[]="blub!";
#ifdef DEBUG_VERBOSE
    readpng_version_info();
#endif // DEBUG_VERBOSE

    i=badgeOpen();
    printf("open: %i\n",i);
    if (i<0) {
        printf("! Failed to open badge. Existing ..\n");
        return EXIT_FAILURE;
    }

    spaceinvaders();

/*
    badgeAddTextMessage(0,m1,6);
    badgeSetEffects(0, E_up, MAX_SPEED, false, false);
    badgeSetFont(1,0);
    badgeAddTextMessage(1,m2,4);
    badgeSetEffects(1, E_down, MAX_SPEED, false, false);
    //badgeAddTextMessage(1,m2,5,E_right,DEF_SPEED,false,false);
    //badgeAddTextMessage(2,m3,7,E_up,DEF_SPEED,false,true);

    badgeSetBrightness(3);
*/

    printf("send: %i\n",badgeSend());
    printf("close: %i\n",badgeClose());

    return EXIT_SUCCESS;
}


// call after badgeOpen() with no other addText or AddGfx calls
void spaceinvaders(void) {
    if (!badgeIsOpen) return;
    char game[] = "Game", over[] = "Over";

    printf("# sizeof(gfx_start_data): %li\n",sizeof(gfx_start_data));
    badgeAddGfxMessage(0, gfx_start_data, gfx_start_width);
    badgeSetEffects(0, E_left, 7, false, true);
    printf("# sizeof(gfx_invader_data): %li\n",sizeof(gfx_invader_data));
    badgeAddGfxMessage(1, gfx_invader_data, gfx_inv_width);
    badgeSetEffects(1, E_down, 6, false, false);
    badgeAddTextMessage(2, game, sizeof(game)-1);
    badgeSetEffects(2, E_volume, 5, false, false);
    badgeAddTextMessage(3, over, sizeof(over)-1);
    badgeSetEffects(3, E_laser, 6, false, false);

    badgeSetBrightness(2);
}
