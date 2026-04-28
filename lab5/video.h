#ifndef VIDEO_H
#define VIDEO_H

#include <lcom/lcf.h>

/**
 * @name VBE Functions & BIOS Services
 * 
 * VBE Functions:
 * 
 * AH => MSB of the AX register => 0x4F means VBE and not VBA command
 * AL => LSB of the AX register => The actual VBE function
 * 
 *      31               16 15                0  (Bit positions)
 *      |------------------|------------------|
 *      |                  |        AX        |  (16 bits)
 *      |    (Upper 16)    |--------|---------|
 *      |                  |   AH   |   AL    |  (8 bits each)
 *      |------------------|--------|---------|
 *      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *          ^^ This is EAX (32 bits total)
 *                    
 * AH=0x4F, AL=0x00: Return VBE Controller Information (ES:DI)
 * AH=0x4F, AL=0x01: Return VBE Mode Info (Mode in CX, info to ES:DI)
 * AH=0x4F, AL=0x02: Set VBE mode (Mode in BX, bit 14 set for Linear Framebuffer)
 * 
 * BIOS Set Video Mode:
 * AH=0x00, AL=<video_mode> (Set AL to 0x03 for Minix default text mode)
 * 
 * 
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

// Graphic Modes
#define VBE_MODE_105      0x105     // 1024x768, Indexed, 8 bpp
#define VBE_MODE_110      0x110     // 640x480, Direct color, 15 bpp (1:5:5:5)
#define VBE_MODE_115      0x115     // 800x600, Direct color, 24 bpp (8:8:8)
#define VBE_MODE_11A      0x11A     // 1280x1024, Direct color, 16 bpp (5:6:5)
#define VBE_MODE_14C      0x14C     // 1152x864, Direct color, 32 bpp (8:8:8:8)

int     is_valid_mode(uint16_t mode);
reg86_t vbe_reg();
int     vg_init_mode(uint16_t mode);

int     init_framebuffer(uint16_t mode);
char*   get_video_mem();
vbe_mode_info_t get_vmi();
unsigned get_bytes_per_pixel();
uint16_t get_hres();
uint16_t get_vres();

int     draw_pixel(uint16_t x, uint16_t y, uint32_t color);
int     draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int     draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

int     draw_xpm(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y);

#endif
