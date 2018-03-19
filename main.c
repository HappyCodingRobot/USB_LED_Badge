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
#include "readpng.h"
// TODO: add license file

/* Prototypes */
void spaceinvaders(void);
void readPNGtoMap(char *filename, char *pImg);

/* Globals */
static const char *help[3] = {
	"USB Led-Badge CLI\n"
	"Copyright (C) 2018 HappyCodingRobot\n\n"
	"Usage: %s [options...]\n",

	"\nOptions:\n"
	"\t-h  Show help.\n"
	"\t-i  Set message index [0..7].\n"
	"\t-m  Set message text.\n"
	"\t(-p  Load graphic message (as .png).       ->>> TODO! )\n"
	"\t-s  Set the message speed [1..8].\n"
	"\t-e  Set the message effect [0..8].\n"
	"\t\t0: to left\t 1: to right\n"
	"\t\t2: scroll up\t 3: scroll down\n"
	"\t\t4: Freeze\t 5: Animation\n"
	"\t\t6: Snow\t\t 7: Volume\n"
	"\t\t8: Laser\n"
	"\t-b  Blink message.\n"
	"\t-f  Set frame for message.\n"
	"\t-B  Set the badge brightness [0..3]. This is for all messages.\n"
	"\t-D  Demo mode. Use as the only argument.\n",

	"\nExamples:\n"
	"\tset message with effect and speed:  %s -i <idx> -\n"
	"\t   ->>>> TODO! \n"
	"\n"
};
// TODO: add examples to help message
// TODO: add emoji/icons to use in message (?)

//static uch *image_data;


/* all the main stuff */


// TODO: implement libpng read image
// TODO: convert to usable bitmap for badge
// use libpng to read image file
/*
int readPNGtoMap__(FILE *fp) {
    png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr,
        user_error_fn, user_warning_fn);
    if (!png_ptr)
        return (ERROR);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
        return (ERROR);
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr,
          (png_infopp)NULL);
        return (ERROR);
    }

}
*/

void readPNGtoMap(char *filename, char *pImg) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "! file '%s' not found or can't open.\n", filename);
        return;
    }

    unsigned long imgWidth, imgHeight;
    int res = readpng_init(fp, &imgWidth, &imgHeight);
    if (res) {
        switch (res) {
            case 1:
                fprintf(stderr, "! bad signature, not a PNG?\n");
                break;
            case 2:
                fprintf(stderr, "! libpng error ..\n has bad IHDR (libpng longjmp)\n");
                break;
            case 4:
                fprintf(stderr, "! out of memory, can not initialize structures.\n");
                break;
            default:
                fprintf(stderr, "! unknown error.\n");
                break;
        }
        fclose(fp);
        return;
    }
#ifdef DEBUG_VERBOSE
    printf("Picture dimensions: %lu , %lu\n", imgWidth, imgHeight);
#endif // DEBUG_VERBOSE

    unsigned char red, green, blue;
    res = readpng_get_bgcolor(&red, &green, &blue);
    if (res > 1) {
        readpng_cleanup(TRUE);
        fprintf(stderr,"! libpng error while checking for background color\n");
        fclose(fp);
        return;
    }
#ifdef DEBUG_VERBOSE
    if (res == 1)
        printf(": no bKGD chunk found.\n");
    printf("Background: r=%i g=%i b=%i\n", red, green, blue);
#endif // DEBUG_VERBOSE


    static double display_exponent = 0;
    static unsigned long image_rowbytes;
    static int image_channels;
    uch *img_data;
    //uch* readpng_get_image(double display_exponent, int* pChannels, ulg* pRowbytes);
    img_data = readpng_get_image(0, &image_channels, &image_rowbytes);
#ifdef DEBUG_VERBOSE
    printf("image channels= %i\nimage row bytes= %i\n", image_channels, image_rowbytes);
#endif // DEBUG_VERBOSE
    /* done with PNG file, cean up but do NOT nuke image_data! */
    readpng_cleanup(FALSE);
    fclose(fp);

    if (!img_data) {
        fprintf(stderr, "! unable to decode PNG image\n");
        return;
    }

    //test_out_print_byte(img_data,image_rowbytes,imgHeight);
    // TODO: conversion stuff..
    img_data = calloc(imgWidth*imgHeight, 1);

    // now clean up also image data
    readpng_cleanup(TRUE);
    //free(img_data);
}



int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    char *image = NULL;

#ifdef DEBUG_VERBOSE
    readpng_version_info();
#endif // DEBUG_VERBOSE

/// testing ..
    //readPNGtoMap("gfx/test_11x44.png", image);
    //readPNGtoMap("gfx/test.png", image);
    //readPNGtoMap("gfx/index_unicorn.png", image);
    // stuff..
    //free(image);
//    return EXIT_FAILURE;

// https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
    char c;
    char *cval = NULL;
    int tmp, ret = 0;
    uint8_t idx = 0;
#ifdef DEBUG
    printf("\nArguments: %i\n", argc);
    for (int c=0; c<argc; c++) {
        printf(" %i: <%s>\n", c, argv[c]);
    }
#endif // DEBUG_VERBOSE
    if (argc == 1) {
        printf(help[0], argv[0]);
        return EXIT_FAILURE;
    }

    ret = badgeOpen();
    printf("open: %i\n",ret);
    if (ret < 0) {
        printf("! Failed to open badge. Existing ..\n");
        return EXIT_FAILURE;
    }
    while ((c = getopt(argc, argv, "hi:m:p:s:e:bfB:D")) != -1) {
        switch (c) {
            case 'h':
                //show_help();
                printf(help[0], argv[0]);
                printf(help[1]);
                printf(help[2]);
                badgeClose();
                return EXIT_SUCCESS;
                //break;
            case 'i':
                if (optarg) {
                    tmp = atoi(optarg);
                    if (tmp>=0 && tmp<=7) {
                        idx = tmp;
                    } else {
                        fprintf(stderr, "! Message index out of range: %i\n", tmp);
                        //return EXIT_FAILURE;
                        ret = EXIT_FAILURE;
                    }
                }
                break;
            case 'm':
                if (optarg) {
#ifdef DEBUG
                    printf("--Message %i: \"%s\", l=%i \n", idx, optarg, (int)strlen(optarg));
#endif // DEBUG
                    if (badgeAddTextMessage(idx, optarg, (int)strlen(optarg)) < 0) {
                        fprintf(stderr, "! Error. Badge not opened?\n");
// FIXME                        return EXIT_FAILURE;
// remove? check not necessary here..
                    }
                } else {
                    fprintf(stderr, "! Empty message.\n");
                    //return EXIT_FAILURE;
                    ret = EXIT_FAILURE;
                }
                break;
            case 'p':
                // TODO: use readPNGtoMap() fkt here when ready!!!
                fprintf(stderr, "! Not implemented yet.");
                //return EXIT_FAILURE;
                ret = EXIT_FAILURE;
                //
                if (optarg) {
                    //
                } else {
                    fprintf(stderr, "! No .png file.\n");
                    //return EXIT_FAILURE;
                    ret = EXIT_FAILURE;
                }
                break;
            case 's':
                if (optarg) {
                    tmp = atoi(optarg);
                    if (tmp>=1 && tmp<=8) {
                        badgeSetEffectsSpd(idx, tmp);
                    } else {
                        fprintf(stderr, "! Message speed out of range: %i\n", tmp);
                        //return EXIT_FAILURE;
                        ret = EXIT_FAILURE;
                    }
                }
                break;
            case 'e':
                if (optarg) {
                    tmp = atoi(optarg);
                    if (tmp>=0 && tmp<=8) {
                        badgeSetEffectsPat(idx, tmp);
                    } else {
                        fprintf(stderr, "! Message effect out of range: %i\n", tmp);
                        //return EXIT_FAILURE;
                        ret = EXIT_FAILURE;
                    }
                }
                break;
            case 'b':
                badgeSetEffectsBlink(idx, true);
                break;
            case 'f':
                badgeSetEffectsFrame(idx, true);
                break;
            case 'B':
                if (optarg) {
                    tmp = atoi(optarg);
                    if (tmp>=0 && tmp<=3) {
                        badgeSetBrightness(3-tmp);
                    } else {
                        fprintf(stderr, "! Brightness out of range: %i\n", tmp);
                        //return EXIT_FAILURE;
                        ret = EXIT_FAILURE;
                    }
                }
                break;
            case 'D':
                printf("Setting up demo ..\ntrying to load Space Invaders ..\n");
                spaceinvaders();
                break;
            case ':':
            case '?':
            default:
                //fprintf(stderr, "! Wrong or missing argument.\n");
                //show_help();
                //return EXIT_FAILURE;
                ret = EXIT_FAILURE;
                break;
        }
#ifdef DEBUG
        printf("command=%c , par=%s\n", c, optarg);
        //printf("command=%c , par=%s\n", c, cval);
        //cval = NULL;
#endif // DEBUG
        if (ret == EXIT_FAILURE)
            break;
    }

    if (ret != EXIT_FAILURE)
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
