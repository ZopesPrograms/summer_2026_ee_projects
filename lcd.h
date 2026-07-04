#include <stdint.h>
#include "gpio.h"

#define ILI9486
#define RST_PIN GPIO_PD10
#define CS_PIN GPIO_PD11
#define F_CS_PIN GPIO_PC1
#define RS_PIN GPIO_PE17
#define WR_PIN GPIO_PE16
#define DB0_PIN GPIO_PB0
#define DB13_PIN GPIO_PD13

#ifdef ILI9486

#define LCD_RESET_SOFT   0x01
#define LCD_DISPLAY_OFF  0x28
#define LCD_POWER_CTRL_1 0xC0
#define LCD_POWER_CTRL_2 0xC1
#define LCD_POWER_CTRL_3 0xC2
#define VCOM_CTRL        0xC5 // common voltage control for capacitance in LCD display so crystals align properly
#define DISP_FUNC_CTRL   0xB6
#define POS_GAMMA_CTRL   0xE0 // controls nonlinear adjustment of brightness in accordance w/human eye sensitivity
#define NEG_GAMMA_CTRL   0xE1 // similar to positive gamma control, but negative I guess...
#define DISP_INV_OFF     0x20 // turns of display inversion (we don't produce a photographic negative display).
#define MEM_ACC_CTRL     0x36 // controls the memory access permissions of lcd display (I think).
#define INTF_PIX_FORM    0x3A // controls the LCD interface pixel format (how many bits/pixel, 8bit, 16bit, etc!)
#define LCD_SLEEP_OUT    0x11
#define LCD_DISPLAY_ON   0x29

#define COL_ADDR_SET     0x2A // Sets column (x) address for pixel to be written or read
#define PAGE_ADDR_SET    0x2B // Sets page (y) address for pixel to be written or read

#define RAM_WR           0x2C // RAM WRITE to LCD display memory
#define RAM_RD           0x2E // RAM READ from LCD display memory

#endif

void lcd_init(int16_t width, int16_t height);
void lcd_writecommand(uint8_t c);
void lcd_writedata(uint8_t c);
void lcd_drawpixel(uint16_t x, uint16_t y, uint16_t color);
