/**
 * Simple control API for the Xanes (?) USB LED Badge
 * Copyright (C) 2017 HappyCodingRobot
 *
 * This code is licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <byteswap.h>
#include <hidapi/hidapi.h>
#include "badge.h"
#include "font.h"


/* Badge VID, PID, and interface */
#define BADGE_VID           0x0416
#define BADGE_PID           0x5020
#define BADGE_NAME          'LS32 Custm HID'
#define BADGE_INTERFACE     0
#define BADGE_REPORT_NUM    0


/**
 * Badge Protocol (Report #0)
 *  Report size: 64 byte (rep_num+data[64])
 *
 * Header (first report to send):
 *  id[5]:  "wang",0x00
 *  byte:   Brightness in 4 steps
 *  byte:   bit-coded: flash messages
 *  byte:   bit-coded: border messages
 *  byte:   Message 0 speed and effect
 *  byte:   Message 1 speed and effect
 *  byte:   Message 2 speed and effect
 *  byte:   Message 3 speed and effect
 *  byte:   Message 4 speed and effect
 *  byte:   Message 5 speed and effect
 *  byte:   Message 6 speed and effect
 *  byte:   Message 7 speed and effect
 *  word:   Message 0 length (big endian)
 *  word:   Message 1 length (big endian)
 *  word:   Message 2 length (big endian)
 *  word:   Message 3 length (big endian)
 *  word:   Message 4 length (big endian)
 *  word:   Message 5 length (big endian)
 *  word:   Message 6 length (big endian)
 *  word:   Message 7 length (big endian)
 *
 *  The length counts in the 8x11 characters/elements.
 *
 * Brightness coding:
 *  0x00xx0000
 *      ^^ ----- 0: 100%, 1: 75%, 2: 50%, 3: 25%
 *
 * Flash & Border effect coding:
 *  0bxxxxxxxx
 *           ^ - 1: Message 1 flash/border, 0: no effect
 *          ^ -- 1: Message 2 flash/border, 0: no effect
 *                  ...
 *
 * Speed & effect/pattern coding:
 *  0b00000000
 *        ^^^^ - Effect number [0..8], see enum effects
 *    ^^^^ ----- Scrolling/effect speed [1..8]
 *
 *
 * Data follows (second to nth report to send, all use Report #0)
 *  char[11]:   characters as bitmasks (8x11), stuffed together to fill the reports
 *
 *  Display:
 *  0 {8bit}    | 11    | 22    | ...
 *  1 {8bit}    | 12    | 23    | ...
 *  ...
 *  10 {8bit}   | 21    | 32    | ...
 *
 *
 *  Up to 6 'chars' are displayed directly on the display. Here the last 4 rows/bits are ignored.
 *  On fewer 'chars', the content is centered on the display. On more than 6 'chars', the 6th is
 *  fully used, depending on the chosen effect.
 *  This behavior is important if bitmaps should be displayed.
 *
 */


struct header_s {
    char start[5];      // magic: "wang",0x00
    uint8_t brightness; // badge brightness
    uint8_t flash;      // bit-coded: flash messages
    uint8_t border;     // bit-coded: border messages
    char lineConf[8];   // config of 8 lines; 0xAB : A-speed[1..8] , B-effect[0..8]
    uint16_t msgLen[8]; // length lines (in BIG endian!) (-> not really used, 255 char are enough for now)
};

struct msg_s {
    char *data;
char msg[MAX_STR];
    int msg_len;
bool blink;
bool frame;
    uint8_t speed;
    effects_t pattern;
    char msgType;
    uint8_t font;
};


struct header_s bHeader = {{"wang\0"},0x00,0x00,0x00,{"AFGH@DFG"},
    {0}
};

struct msg_s bMessages[N_MESSAGES];

hid_device* badge_handle = NULL;
bool badgeIsOpen = false;



int badgeOpen() {
    int res=0;
    wchar_t wstr[MAX_STR];
    // Enumerate and print the HID devices on the system
    struct hid_device_info *devs, *cur_dev;

#ifdef DEBUG_LIST_ALL
    //devs = hid_enumerate(0x0, 0x0);
    devs = hid_enumerate(BADGE_VID, BADGE_PID);
    cur_dev = devs;
    while (cur_dev) {
        printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
               cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
        printf("\n");
        printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
        printf("  Product:      %ls\n", cur_dev->product_string);
        printf("\n");
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
#endif // DEBUG_LIST_ALL

    // Open the device using the VID, PID,
    // and optionally the Serial number.
    badge_handle = hid_open(BADGE_VID, BADGE_PID, NULL);
    if (badge_handle == NULL) {
        fprintf(stderr,"! Could not open device.\n");
        return -1;
    }

#ifdef DEBUG_LIST_ALL
    // Read the Manufacturer String
    res = hid_get_manufacturer_string(badge_handle, wstr, MAX_STR);
    printf("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    res = hid_get_product_string(badge_handle, wstr, MAX_STR);
    printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    res = hid_get_serial_number_string(badge_handle, wstr, MAX_STR);
    printf("Serial Number String: %ls", wstr);
    printf("\n");
    //printf("Serial Hex Number String: %x", wstr); printf("\n");
#endif // DEBUG_LIST_ALL

    // initialize message memories
    for (int i=0; i<N_MESSAGES; i++) {
        bMessages[i].data = calloc(MAX_STR*BADGE_MSG_FONT_HEIGHT, (sizeof(char)));
        bMessages[i].font = BADGE_MSG_FONT;
    }
    badgeIsOpen = true;
    return res;
}


int badgeSend() {
    int res=0;
    char ReportBuf[65];                         // report number in ReportBuf[0]
    //char *pReportNum = ReportBuf;
    char *pData  = ReportBuf+1;
    char *pDisp = calloc(DISP_SIZE, sizeof(char));
    char *pDIdx = pDisp, *pSIdx = pDisp;
    int data_len;

    if (!badge_handle) {
        fprintf(stderr,"! Device not opened.\n");
        return -1;
    }
    // fill header from messages struct
    for (int i=0; i<N_MESSAGES; i++) {
        if (bMessages[i].msg_len>0) {
            bHeader.msgLen[i] = bswap_16(bMessages[i].msg_len);     // in BIG endian!
            data_len = bMessages[i].msg_len * BADGE_MSG_FONT_HEIGHT;
            // fill badge display buffer
            memcpy( pDIdx, bMessages[i].data, data_len);
            //memset( pDIdx, 0xFF, BADGE_MSG_FONT_HEIGHT);          // >>>> DEBUG !!!!
            pDIdx += data_len;
        }
    }
    // send report with header
    memset(ReportBuf, 0, sizeof(ReportBuf));
    memcpy(pData, &bHeader, sizeof(bHeader));
#ifdef DEBUG_VERBOSE
    //printf("# malloc_usable_size(pDisp): %i\n", malloc_usable_size(pDisp));
    printf("# used data: %lu\n", pDIdx-pDisp);
    printf("# Header data:");
    hex_out(pData, 32);                                             // >>>> DEBUG !!!!
#endif // DEBUG_VERBOSE
    if (hid_write(badge_handle, ReportBuf, sizeof(ReportBuf)) >= 0) {
        // send data reports
        while ((pSIdx < pDIdx) && (res >= 0)) {
            memset(ReportBuf, 0, sizeof(ReportBuf));
            memcpy(pData, pSIdx, 64);
#ifdef DEBUG_VERBOSE
            printf("\n# Report data #%i:",res);
            hex_out(pData, 64);
#endif // DEBUG_VERBOSE
            if (hid_write(badge_handle, ReportBuf, sizeof(ReportBuf)) < 0) {
                fprintf(stderr,"! Error on sending Data Report #%i\n",res);
                res = -1;
            } else {
                res++;
            }
            pSIdx += 64;
        }
    } else {
        fprintf(stderr,"! Error on sending Header Report.\n");
        res = -1;
    }

    free(pDisp);
    return res;
}


void badgeAddTextMessage(uint8_t msg_num, char* msg, int msg_len) {
    const font_info_t *pFont;
    const char *bitmap;
    char *pDIdx;
    char c;
    // return if invalid message number or length
    if (msg_num>=N_MESSAGES || msg_len==0) {
        return;
    }
    if (msg_len>MAX_STR) {
        bMessages[msg_num].msg_len = MAX_STR;
    } else {
        bMessages[msg_num].msg_len = msg_len;
    }
    pFont = fonts[bMessages[msg_num].font];
    pDIdx = bMessages[msg_num].data;

    for (int i=0; i<bMessages[msg_num].msg_len; i++) {
        //
        c = msg[i];
        if ((c < pFont->char_start) || (c > pFont->char_end))
            c= pFont->char_start;                           // -> should be 'space'
        c = c - pFont->char_start;
        bitmap = pFont->bitmap + pFont->char_descriptors[c].offset;
        //memcpy( mt+(i*11), bitmap, 11);
        //memcpy( &mt[i*11], bitmap, 11);
        memcpy( pDIdx, bitmap, BADGE_MSG_FONT_HEIGHT);
        //memset( pDIdx, 0xFF, BADGE_MSG_FONT_HEIGHT);    // >>>> DEBUG !!!!
        pDIdx += BADGE_MSG_FONT_HEIGHT;
    }

}


void badgeAddGfxMessage(uint8_t msg_num, char *gfx, int gfx_width) {
    // return if invalid message number or length
    if (msg_num>=N_MESSAGES || gfx_width==0) {
        return;
    }
    memset( bMessages[msg_num].data, 0x00, MAX_STR*BADGE_MSG_FONT_HEIGHT);
    // TODO: size check of graphic data (?)
    bMessages[msg_num].msg_len = gfx_transform_to_bits(bMessages[msg_num].data, gfx_width, gfx);
}


void badgeSetEffects(uint8_t msg_num, effects_t msg_pat, uint8_t msg_spd, bool msg_blink, bool msg_frame) {
    uint8_t spd=0, pat=0;
    if (msg_num >= N_MESSAGES) {
        return;
    }
    if (msg_spd>=MIN_SPEED && msg_spd<=MAX_SPEED) {
        spd = msg_spd;
    } else {
        spd = (uint8_t) DEF_SPEED;
    }
    if (msg_pat>=E_left && msg_pat<=E_laser) {
        pat = (uint8_t) msg_pat;
    } else {
        pat = (uint8_t) E_right;
    }
    bHeader.lineConf[msg_num] = (spd << 4) | pat;
    bHeader.flash &= !(1 << msg_num);
    bHeader.flash |= (msg_blink << msg_num);
    bHeader.border &= !(1 << msg_num);
    bHeader.border |= (msg_frame << msg_num);
}


void badgeSetBrightness(uint8_t msg_br) {
    if (msg_br <= MIN_BRIGHTNESS) {
        bHeader.brightness = (uint8_t) (msg_br << 4);
    }
}

int badgeClose() {
    // free message memories
    for (int i=0; i<N_MESSAGES; i++) {
        free(bMessages[i].data);
    }
    // close HID
    hid_close(badge_handle);
    badgeIsOpen = false;
    // Finalize the hidapi library
    return hid_exit();
}




/**< Helper functions --------------------------------------------------- */

void badgeSetFont(uint8_t msg_num, uint8_t font_num) {
    if ((msg_num >= N_MESSAGES) ||(font_num >= NUM_FONTS)) {
        return;
    }
    bMessages[msg_num].font = font_num;
}


int gfx_transform_to_bits(char* data_out, int len, char* data_in) {
    int cnt=0;
    int len_char = len / 8;
    int len_rem = len % 8;
    int line_addr = 0;
    for (int l=0; l<len_char; l++) {
        for (int j=0; j<BADGE_MSG_FONT_HEIGHT; j++) {
            line_addr = (l*8+j*len);
            for (int i=0; i<8; i++) {
                data_out[cnt] |= (data_in[line_addr+i] << (7-i));
            }
            cnt++;
        }
    }
    if (len_rem > 0) {
        for (int j=0; j<BADGE_MSG_FONT_HEIGHT; j++) {
            line_addr = len_char*8+j*len;
            for (int i=0; i<len_rem; i++) {
                data_out[cnt] |= (data_in[line_addr+i] << (7-i));
            }
            cnt++;
        }
        len_char++;
    }
    #ifdef DEBUG_VERBOSE
    printf("# return length: %i\n",len_char);
    #endif // DEBUG_VERBOSE
    return len_char;
}



/**< Debug functions --------------------------------------------------- */
#ifdef DEBUG_VERBOSE

#define HEX_BR_LEN 8
void hex_out(unsigned char *data, int len) {
    for (int i=0; i<len; i++) {
        if (!(i % HEX_BR_LEN))
            printf("\n");
        printf(" 0x%02X", data[i]);
    }
    printf("\n");
}

// print out as stored for badge : 1 byte/pix, top to down then left, to right
void test_out_print(char *data, int len, int h) {
    for (int i=0; i<len*h; i++) {
        if (!(i % h))
            printf("|\n");
        printf("%c", data[i]? '*':' ');
    }
    printf("\n");
}

// print out image vertically : 1 byte/pix, left to right, then top to down
void test_out_print_byte(char *data, int len, int h) {
    for (int i=0; i<len; i++) {
        for (int j=h-1; j>=0; j--) {
            printf("%c", data[i+j*len]? '*':' ');
        }
        printf(" |\n");
    }
    printf("\n");
}

// print out display image image vertically : 1 bit/pix, stored as bytes /w height 11, left, to right then top to down
// length in byte!
void test_out_print_bit(unsigned char *data, int len_byte, int h) {
    for (int i=0; i<len_byte*h; i+=h) {
        for (int b=7; b>=0; b--) {
            for (int j=h; j>=0; j--) {
                printf("%c", (data[i+j] & (1<<b))? '*':' ');
            }
            printf(" | char: %i\n",i);
        }
    }
    printf("\n");
}


#endif // DEBUG_VERBOSE
