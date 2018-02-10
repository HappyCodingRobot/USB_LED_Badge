/**
 * Simple control API for the Xanes (?) USB LED Badge
 * Copyright (C) 2017 HappyCodingRobot
 *
 * This code is licensed under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#ifndef BADGE_H
#define BADGE_H

/**
 * Message types
 */
#define BADGE_MSG_TYPE_TEXT   0
#define BADGE_MSG_TYPE_BITMAP 1

/**
 * Bounds for the badge
 */
#define MIN_SPEED 1
#define DEF_SPEED 4
#define MAX_SPEED 8

#define MIN_BRIGHTNESS 3
#define MAX_BRIGHTNESS 0

#define N_MESSAGES 8

#define MAX_STR 255

#define BADGE_MSG_FONT          1
#define BADGE_MSG_FONT_HEIGHT   11
//#define DISP_SIZE (MAX_STR*11*N_MESSAGES)+64
#define DISP_SIZE 32767

typedef enum e_effects {
    E_left      = 0,
    E_right,
    E_up,
    E_down,
    E_freeze,
    E_animation,
    E_snow,
    E_volume,
    E_laser
} effects_t;

typedef enum e_brightness {
    BR_100      = 0,
    BR_75,
    BR_50,
    BR_25
} brightness_t;

// ???
extern bool badgeIsOpen;


/** \brief Open HID and prepares structures
 *
 * \return 0 or debug result on success, -1 on error.
 */
int badgeOpen(void);


/** \brief Add text messages to the message struct
 *
 * \param msg_num   - Message number [0..7]
 * \param msg       - A pointer to the message text (characters)
 * \param msg_len   - Length of the message
 */
void badgeAddTextMessage(uint8_t msg_num, char *msg, int msg_len);


/** \brief Add graphic to the message struct
 *
 * \param msg_num   - Message number [0..7]
 * \param gfx       - A pointer to the graphics
 * \param gfx_width - Length of the graphic (in pixel)
 *
 * Converts and adds an image to a message struct. Excepts data as c array with
 * 1 byte/px, stored from left to right and top to bottom, a height of 11 pixels
 * (more than 11 are ignored).
 */
void badgeAddGfxMessage(uint8_t msg_num, char *gfx, int gfx_width);


/** \brief Sets the message effects and behavior to the message struct
 *
 * \param msg_num   - Message number [0..7]
 * \param msg_pat   - Message effect (see: enum e_effects)
 * \param msg_spd   - Scrolling/effect speed [1..8]
 * \param msg_blink - Blink message [true,false]
 * \param msg_frame - Frame around message [true, false]
 */
void badgeSetEffects(uint8_t msg_num, effects_t msg_pat, uint8_t msg_spd, bool msg_blink, bool msg_frame);


/** \brief Sets the message brightness in 4 steps
 *
 * \param msg_br    - Brightness value Message number [0..3: 100%..25%]
 */
void badgeSetBrightness(uint8_t msg_br);


/** \brief build the report buffer from the complete message struct and send it to the device
 *
 * \return number of reports send on success, -1 on error.
 */
int badgeSend(void);


/** \brief Close HID
 *
 * \return 0 on success, -1 on error.
 */
int badgeClose(void);




/**< helper functions */
//void badgeClear(void); // needed ??
//void badgeDrawChar(uint8_t id, char c, char *target);
void badgeSetFont(uint8_t msg_num, uint8_t font_num);   // testing, use before badgeAddTextMessage()
int gfx_transform_to_bits(char* data_out, int len, char* data_in);


/**< debug functions */
#ifdef DEBUG_VERBOSE
void hex_out(unsigned char *data, int len);
void test_out_print_byte(char *data, int len, int h);
void test_out_print_bit(unsigned char *data, int len_byte, int h);
#endif // DEBUG_VERBOSE


#endif	/* BADGE_H */
