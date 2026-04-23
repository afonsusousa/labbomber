#ifndef VIDEO_H
#define VIDEO_H

#include <lcom/lcf.h>

/**
 * @name VBE Functions & BIOS Services
 * 
 * VBE Functions:
 * AH=0x4F, AL=0x00: Return VBE Controller Information (ES:DI)
 * AH=0x4F, AL=0x01: Return VBE Mode Info (Mode in CX, info to ES:DI)
 * AH=0x4F, AL=0x02: Set VBE mode (Mode in BX, bit 14 set for Linear Framebuffer)
 * 
 * BIOS Set Video Mode:
 * AH=0x00, AL=<video_mode> (Set AL to 0x03 for Minix default text mode)
 */
#define BIOS_VID_INT        0x10    // BIOS Video Service Interrupt

// VBE Call (AH)
#define VBE_CALL          0x4F      // VBE function identifier

// VBE Functions (AL)
#define VBE_CTRL_INFO     0x00      // Return VBE Controller Information
#define VBE_MODE_INFO     0x01      // Return VBE Mode Info
#define VBE_SET_MODE      0x02      // Set VBE mode

// BIOS Video Modes
#define BIOS_SET_VID_MODE 0x00      // BIOS Set Video Mode
#define MINIX_TEXT_MODE   0x03      // Minix default text mode

// VBE Mode settings
#define LINEAR_FRAMEBUFR  BIT(14)   // Bit 14 must be set to use flat frame buffer model

int vg_init_mode(uint16_t mode);

int draw_pixel(uint16_t x, uint16_t y, uint32_t color);
int draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

int draw_xpm(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y);

#endif